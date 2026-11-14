# EffeXis

Open-source GPU video editor for Windows and Linux. Multitrack editing, Vulkan
compositing, Effect-style look presets, and a local MCP server so Claude Code
(and other agents) can edit the same project you edit by hand.

EffeXis is a fork of [QuickEdit](https://github.com/Zovelik/QuickEdit) (BSD
3-Clause). See [FORK.md](FORK.md).

## Features (v0.1)

- Timeline: video / sound / audio / subtitle tracks, split, move, ripple, nested sequences
- GPU decode / composite / encode (Vulkan Video, NVENC, AMF when available)
- Video effects: color, blur, sharpen, vignette, chroma key, pixelate, **grain, RGB shift, CRT, glitch, film look**
- Built-in look presets + `.effexis-preset.json` save/load
- MCP on `http://127.0.0.1:8765/mcp` for AI-assisted editing
- Manual UI (Dear ImGui) — no agent required

## MCP (Claude Code)

1. Run EffeXis and enable **Edit → Preferences → AI Control**.
2. Register the server:

```bash
claude mcp add --transport http effexis http://127.0.0.1:8765/mcp
```

Or copy [templates/claude-video-project](templates/claude-video-project) into a
video project (`.mcp.json` + `AGENTS.md`).

Tool reference: [docs/mcp-tools.md](docs/mcp-tools.md).

## Building

### Requirements

- CMake 3.15+ and Ninja
- C++20: Visual Studio 2022 (Windows) or GCC 12+ / Clang 15+ (Linux)
- Vulkan SDK 1.4 (`glslc`, `shaderc`)
- GPU with Vulkan 1.3+

### Dependencies

```bash
git submodule update --init --recursive
```

Downloaded (gitignored):

- **FFmpeg** LGPL shared → `3rdparty/ffmpeg/win64` (Windows) from
  [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds)
- Optional: VST3 SDK → `3rdparty/vst3sdk`, whisper.cpp → `3rdparty/whisper.cpp`

### Build (Windows)

From a VS x64 developer environment, with `VULKAN_SDK` set:

```bat
cmake -G Ninja -S . -B out/build/x64-Release -DCMAKE_BUILD_TYPE=Release ^
  -DVulkan_INCLUDE_DIR=%VULKAN_SDK%/Include ^
  -DVulkan_LIBRARY=%VULKAN_SDK%/Lib/vulkan-1.lib
cmake --build out/build/x64-Release
ctest --test-dir out/build/x64-Release
```

Binary: `out/build/x64-Release/bin/EffeXis.exe`

### Build (Linux)

```bash
cmake -G Ninja -S . -B out/build/release -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/release
```

## Acceptance checklist (v0.1)

- [ ] Build and launch on Windows
- [ ] Import media, split/move clips, preview, export MP4
- [ ] Apply a look preset (UI or `effects_apply_preset`) with live preview
- [ ] Claude Code via MCP: `get_project` → cut → preset → export; undo works

## Clean-room / REA

Research of DaVinci Resolve or Effect.app with REA is allowed only under your
legal authority. EffeXis implementations are independent (clean-room). REA is
not required at runtime (`npx rea-agents` for development research only).

## License

BSD 3-Clause — see [LICENSE](LICENSE) and [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).
