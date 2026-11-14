# EffeXis video project

This project is edited with **EffeXis**, a local GPU video editor with MCP.

## Before editing

1. Launch `EffeXis.exe` (or the Linux binary).
2. Open or create the `.qep` project for this folder.
3. Enable **Edit → Preferences → AI Control** (MCP on `127.0.0.1:8765`).
4. Ensure this Claude Code session has the `effexis` MCP server registered.

## How to edit

- Start with `get_project`.
- Import with `import_media`, cut with `split_clips` / `move_clip` / `delete_clips`.
- Looks: `effects_list` then `effects_apply_preset` (e.g. `warm_film`).
- Verify with `render_frame`, then export.
- Undo is available in the app for every MCP mutation.

## Rules

- MCP is loopback-only; do not assume a cloud editor.
- Do not invent media paths — use files in this project.
- Prefer presets and built-in effects before writing custom shaders.
