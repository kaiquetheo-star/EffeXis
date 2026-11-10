#include <media/VideoReader.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>


using namespace core;


namespace media
{

namespace {

// Requests up to this far ahead of the current position decode forward;
// further ones seek.
constexpr Time ForwardDecodeLimit = 2 * TicksPerSecond;

// Still images cover the whole timeline.
constexpr Time StillImageDuration = std::numeric_limits<Time>::max() / 4;


std::string errorString(int error)
{
    char buffer[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(error, buffer, sizeof(buffer));
    return buffer;
}


// Picks the Vulkan format when the decoder offers it, the software one
// otherwise. FFmpeg calls it again without Vulkan if the hardware decoder
// cannot be set up for the stream.
AVPixelFormat getFormat(AVCodecContext* codec, const AVPixelFormat* formats)
{
    for (const AVPixelFormat* format = formats; *format != AV_PIX_FMT_NONE; format++) {
        if (*format == AV_PIX_FMT_VULKAN) {
            return *format;
        }
    }

    // The first one is the native software format.
    for (const AVPixelFormat* format = formats; *format != AV_PIX_FMT_NONE; format++) {
        const AVPixFmtDescriptor* descriptor = av_pix_fmt_desc_get(*format);

        if (descriptor && !(descriptor->flags & AV_PIX_FMT_FLAG_HWACCEL)) {
            return *format;
        }
    }

    return AV_PIX_FMT_NONE;
}


// Up to this alpha, a texel is not seen: lossy codecs (VP9) leave a little
// alpha in the transparent parts.
constexpr uint8_t HiddenAlpha = 2;


// Gives the transparent texels the color of their visible neighbors,
// ring by ring, up to the given number of rings. The pictures keep straight
// alpha, and their filtering (bilinear, bicubic, Lanczos, blur) mixes the
// colors of neighboring texels whatever their alpha: the black usually stored
// under transparent pixels would otherwise darken the edges.
void bleedTransparentColors(uint8_t* rgba, uint32_t width, uint32_t height, int rings)
{
    const size_t count = (size_t)width * height;

    // 0: transparent, 1: has a color, 2: transparent, in the next ring.
    std::vector<uint8_t> state(count);
    size_t transparent = 0;

    for (size_t i = 0; i < count; i++) {
        state[i] = (rgba[i * 4 + 3] > HiddenAlpha) ? 1 : 0;
        transparent += (state[i] == 0);
    }

    if (transparent == 0 || transparent == count) {
        return;
    }

    auto forNeighbors = [width, height](size_t index, auto&& visit) {
        const int64_t x = (int64_t)(index % width);
        const int64_t y = (int64_t)(index / width);

        for (int64_t ny = std::max<int64_t>(y - 1, 0); ny <= std::min<int64_t>(y + 1, height - 1); ny++) {
            for (int64_t nx = std::max<int64_t>(x - 1, 0); nx <= std::min<int64_t>(x + 1, width - 1); nx++) {
                if (nx != x || ny != y) {
                    visit((size_t)(ny * width + nx));
                }
            }
        }
    };

    // The first ring: the transparent texels touching a visible one.
    std::vector<size_t> ring;
    std::vector<size_t> next;

    for (size_t i = 0; i < count; i++) {
        if (state[i] == 0) {
            bool edge = false;
            forNeighbors(i, [&](size_t n) { edge |= (state[n] == 1); });

            if (edge) {
                state[i] = 2;
                ring.push_back(i);
            }
        }
    }

    std::vector<uint8_t> colors;

    for (int r = 0; r < rings && !ring.empty(); r++) {
        // The average of the colored neighbors, written once the whole ring
        // is done so that it does not lean on its own texels.
        colors.resize(ring.size() * 3);

        for (size_t k = 0; k < ring.size(); k++) {
            uint32_t sum[3] = {};
            uint32_t n = 0;

            forNeighbors(ring[k], [&](size_t neighbor) {
                if (state[neighbor] == 1) {
                    for (int c = 0; c < 3; c++) {
                        sum[c] += rgba[neighbor * 4 + c];
                    }

                    n++;
                }
            });

            for (int c = 0; c < 3; c++) {
                colors[k * 3 + c] = (uint8_t)((sum[c] + n / 2) / std::max<uint32_t>(n, 1));
            }
        }

        for (size_t k = 0; k < ring.size(); k++) {
            std::memcpy(rgba + ring[k] * 4, colors.data() + k * 3, 3);
            state[ring[k]] = 1;
        }

        next.clear();

        for (size_t index : ring) {
            forNeighbors(index, [&](size_t n) {
                if (state[n] == 0) {
                    state[n] = 2;
                    next.push_back(n);
                }
            });
        }

        ring.swap(next);
    }
}

} // namespace


VideoFrame::~VideoFrame()
{
    av_frame_free(&hwFrame);
}


VideoReader::VideoReader(const std::filesystem::path& path, AVBufferRef* hwDevice)
{
    const std::u8string u8 = path.u8string();
    const std::string utf8(u8.begin(), u8.end());

    int result = avformat_open_input(&_format, utf8.c_str(), nullptr, nullptr);

    if (result < 0) {
        throw std::runtime_error("Could not open the file: " + errorString(result));
    }

    try {
        result = avformat_find_stream_info(_format, nullptr);

        if (result < 0) {
            throw std::runtime_error("Could not read the streams: " + errorString(result));
        }

        openDecoder(hwDevice);
    }
    catch (...) {
        avcodec_free_context(&_codec);
        avformat_close_input(&_format);
        throw;
    }

    _packet = av_packet_alloc();
    _frame = av_frame_alloc();
}


VideoReader::~VideoReader()
{
    _last.reset();

    sws_freeContext(_sws);
    av_frame_free(&_frame);
    av_packet_free(&_packet);
    avcodec_free_context(&_codec);
    avformat_close_input(&_format);
}


void VideoReader::openDecoder(AVBufferRef* hwDevice)
{
    const AVCodec* decoder = nullptr;
    _streamIndex = av_find_best_stream(_format, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);

    if (_streamIndex < 0 || !decoder) {
        throw std::runtime_error("The file has no readable video stream");
    }

    // Only the video packets are needed.
    for (unsigned int i = 0; i < _format->nb_streams; i++) {
        _format->streams[i]->discard = ((int)i == _streamIndex) ? AVDISCARD_DEFAULT : AVDISCARD_ALL;
    }

    const AVStream* stream = _format->streams[_streamIndex];

    // VP8 and VP9 with transparency (WebM): the alpha is a second stream in
    // side data, that only the libvpx decoders read.
    const AVDictionaryEntry* alphaMode = av_dict_get(stream->metadata, "alpha_mode", nullptr, 0);

    if (alphaMode && std::strcmp(alphaMode->value, "1") == 0) {
        const char* name = (stream->codecpar->codec_id == AV_CODEC_ID_VP9) ? "libvpx-vp9"
                         : (stream->codecpar->codec_id == AV_CODEC_ID_VP8) ? "libvpx" : nullptr;

        if (const AVCodec* withAlpha = name ? avcodec_find_decoder_by_name(name) : nullptr) {
            decoder = withAlpha;
        }
    }

    _timeBase = { stream->time_base.num, stream->time_base.den };
    _startTime = (stream->start_time != AV_NOPTS_VALUE)
        ? av_rescale_q(stream->start_time, stream->time_base, AVRational{ 1, (int)TicksPerSecond })
        : 0;

    AVRational rate = stream->avg_frame_rate;

    if (rate.num <= 0 || rate.den <= 0) {
        rate = stream->r_frame_rate;
    }

    _frameDuration = (rate.num > 0 && rate.den > 0)
        ? av_rescale_q(1, AVRational{ rate.den, rate.num }, AVRational{ 1, (int)TicksPerSecond })
        : TicksPerSecond / 30;

    const char* formatName = _format->iformat ? _format->iformat->name : "";
    _stillImage = std::strstr(formatName, "image2") != nullptr ||
                  std::strstr(formatName, "_pipe") != nullptr ||
                  stream->nb_frames == 1;

    _codec = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(_codec, stream->codecpar);
    _codec->pkt_timebase = stream->time_base;

    // Software decoding uses as many threads as useful.
    _codec->thread_count = 0;

    // Vulkan decoding when the codec supports it.
    if (hwDevice && !_stillImage) {
        for (int i = 0;; i++) {
            const AVCodecHWConfig* config = avcodec_get_hw_config(decoder, i);

            if (!config) {
                break;
            }

            if (config->device_type == AV_HWDEVICE_TYPE_VULKAN &&
                (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)) {
                _codec->hw_device_ctx = av_buffer_ref(hwDevice);
                _codec->get_format = getFormat;
                break;
            }
        }
    }

    const int result = avcodec_open2(_codec, decoder, nullptr);

    if (result < 0) {
        throw std::runtime_error("Could not open the video decoder: " + errorString(result));
    }
}


void VideoReader::seek(Time t)
{
    const int64_t timestamp = av_rescale_q(
        t + _startTime,
        AVRational{ 1, (int)TicksPerSecond },
        AVRational{ (int)_timeBase.num, (int)_timeBase.den });

    // Backward: the keyframe at or before t, from which frames are decoded
    // up to t.
    av_seek_frame(_format, _streamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(_codec);

    _endOfStream = false;
    _last.reset();
}


bool VideoReader::decodeNext()
{
    while (true) {
        const int received = avcodec_receive_frame(_codec, _frame);

        if (received == 0) {
            return true;
        }

        if (received == AVERROR_EOF) {
            return false;
        }

        // Otherwise EAGAIN, or a decoding error: the frame is skipped, as
        // players do, and decoding goes on with the next packets.

        // Drained: the decoder has nothing more to give.
        if (_endOfStream) {
            return false;
        }

        // The decoder needs more data.
        bool sent = false;

        while (!sent) {
            const int read = av_read_frame(_format, _packet);

            if (read < 0) {
                // Drain the frames still in the decoder.
                avcodec_send_packet(_codec, nullptr);
                _endOfStream = true;
                sent = true;
            }
            else {
                if (_packet->stream_index == _streamIndex) {
                    avcodec_send_packet(_codec, _packet);
                    sent = true;
                }

                av_packet_unref(_packet);
            }
        }

    }
}


std::shared_ptr<VideoFrame> VideoReader::makeFrame()
{
    auto frame = std::make_shared<VideoFrame>();

    const int64_t pts = (_frame->best_effort_timestamp != AV_NOPTS_VALUE) ? _frame->best_effort_timestamp : _frame->pts;

    frame->time = (pts != AV_NOPTS_VALUE)
        ? av_rescale_q(pts, AVRational{ (int)_timeBase.num, (int)_timeBase.den }, AVRational{ 1, (int)TicksPerSecond }) - _startTime
        : (_last ? _last->end() : 0);

    frame->duration = (_frame->duration > 0)
        ? av_rescale_q(_frame->duration, AVRational{ (int)_timeBase.num, (int)_timeBase.den }, AVRational{ 1, (int)TicksPerSecond })
        : _frameDuration;

    if (_stillImage) {
        frame->time = 0;
        frame->duration = StillImageDuration;
    }

    frame->width = (uint32_t)_frame->width;
    frame->height = (uint32_t)_frame->height;

    if (_frame->format == AV_PIX_FMT_VULKAN) {
        _hardware = true;
        frame->hwFrame = av_frame_clone(_frame);
    }
    else {
        // To RGBA on the CPU, with the swscale threads.
        if (!_sws) {
            _sws = sws_alloc_context();
            _sws->threads = 0;
            _sws->flags = SWS_BILINEAR;
        }

        AVFrame* rgba = av_frame_alloc();
        rgba->format = AV_PIX_FMT_RGBA;
        rgba->width = _frame->width;
        rgba->height = _frame->height;

        if (av_frame_get_buffer(rgba, 0) >= 0 && sws_scale_frame(_sws, rgba, _frame) >= 0) {
            const size_t rowSize = (size_t)frame->width * 4;
            frame->rgba.resize(rowSize * frame->height);

            for (uint32_t y = 0; y < frame->height; y++) {
                std::memcpy(frame->rgba.data() + y * rowSize, rgba->data[0] + (size_t)y * rgba->linesize[0], rowSize);
            }

            // Pictures with transparency (alpha channel, or a palette that
            // may have one): colors under the transparent pixels, all of them
            // for a still image (decoded once), a few rings for a video.
            const AVPixFmtDescriptor* descriptor = av_pix_fmt_desc_get((AVPixelFormat)_frame->format);

            if (descriptor && (descriptor->flags & (AV_PIX_FMT_FLAG_ALPHA | AV_PIX_FMT_FLAG_PAL))) {
                bleedTransparentColors(frame->rgba.data(), frame->width, frame->height,
                    _stillImage ? std::numeric_limits<int>::max() : 4);
            }
        }

        av_frame_free(&rgba);
    }

    av_frame_unref(_frame);

    return frame;
}


std::shared_ptr<VideoFrame> VideoReader::nextFrame()
{
    if (_stillImage && _last) {
        return _last;
    }

    if (!decodeNext()) {
        return nullptr;
    }

    _last = makeFrame();
    return _last;
}


std::shared_ptr<VideoFrame> VideoReader::frameAt(Time t)
{
    if (_last && (_last->covers(t) || _stillImage)) {
        return _last;
    }

    const bool forward = _last && t >= _last->time && t < _last->end() + ForwardDecodeLimit;

    // A still image is its first frame, read without a seek: the image2
    // demuxer (TGA, and the other files without a signature) loses its only
    // packet when it seeks.
    if (!forward && !_stillImage) {
        seek(std::max<Time>(0, t));
    }

    while (decodeNext()) {
        // Frames before t are dropped before being converted.
        const int64_t pts = (_frame->best_effort_timestamp != AV_NOPTS_VALUE) ? _frame->best_effort_timestamp : _frame->pts;
        const Time time = (pts != AV_NOPTS_VALUE)
            ? av_rescale_q(pts, AVRational{ (int)_timeBase.num, (int)_timeBase.den }, AVRational{ 1, (int)TicksPerSecond }) - _startTime
            : t;
        const Time duration = (_frame->duration > 0)
            ? av_rescale_q(_frame->duration, AVRational{ (int)_timeBase.num, (int)_timeBase.den }, AVRational{ 1, (int)TicksPerSecond })
            : _frameDuration;

        if (time + duration <= t && !_stillImage) {
            av_frame_unref(_frame);
            continue;
        }

        _last = makeFrame();
        return _last;
    }

    // Past the end: the last frame stays shown, as the clip may be a frame
    // longer than the stream.
    return _last;
}

}
