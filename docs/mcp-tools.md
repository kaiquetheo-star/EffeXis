# EffeXis MCP tools

The desktop app hosts a Model Context Protocol server (Streamable HTTP) on
`http://127.0.0.1:8765/mcp` when **Edit → Preferences → AI Control** is enabled.
Loopback only. Every mutation is one undo step.

Connect Claude Code:

```bash
claude mcp add --transport http effexis http://127.0.0.1:8765/mcp
```

## Inspection

| Tool | Purpose |
|------|---------|
| `get_project` | Current sequence, tracks, clips, media, playhead |
| `get_clip` | Full clip JSON (ticks: 705600000 per second) |
| `render_frame` | Composite PNG at a time |
| `transcribe_media` | Whisper transcription (model required) |
| `list_shaders` | Built-in and user shaders |
| `effects_list` | Video effect types + EffeXis look presets |

## Media and structure

| Tool | Purpose |
|------|---------|
| `import_media` | Import files, wait for probe |
| `create_sequence` | New empty timeline |
| `switch_sequence` | Show another sequence |
| `update_sequence` | Name / size / fps |
| `add_track` | video / audio / subtitle |

## Clips and editing

| Tool | Purpose |
|------|---------|
| `add_clip` | Place media on a track |
| `add_text` / `add_title` / `add_subtitle` / `add_generator` | Creative clips |
| `update_clip` | Patch clip JSON |
| `move_clip` / `split_clips` / `delete_clips` | Timeline edits |
| `set_transition` | Transition into a clip |
| `add_effect` | Single video effect on clip or track |
| `effects_apply_preset` | Apply look preset (replace or append) |
| `effects_set_param` | Change one effect parameter |
| `effects_clear` | Remove all video effects |
| `effects_save_preset` | Write `.effexis-preset.json` |

## Shaders, playback, export

| Tool | Purpose |
|------|---------|
| `write_shader` | Create/compile user GLSL |
| `set_playhead` / `play` / `pause` | Viewer control |
| `select_clips` | Selection |
| `export` (and related tools from upstream) | Render timeline |

Use `tools/list` on the live server for the exact schema of every tool; this
document mirrors the EffeXis v0.1 surface for agents.

## Built-in look presets

`warm_film`, `cold_grade`, `vhs_tape`, `crt_monitor`, `soft_glow`, `noir`,
`neon_glitch`, `clean_punch`, `dream_haze`, `bleach_bypass`, `sunset_grade`,
`broadcast_safe`.

Example:

```json
{
  "preset_id": "warm_film",
  "clip_id": 12
}
```

Call `effects_apply_preset` with those arguments.
