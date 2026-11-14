# EffeXis is a fork of QuickEdit

EffeXis is based on [Zovelik/QuickEdit](https://github.com/Zovelik/QuickEdit)
(BSD 3-Clause). Upstream remains available as the git remote `upstream`.

## What EffeXis adds

- Product branding and paths (`EffeXis`, `%APPDATA%\EffeXis`, project format
  `EffeXis project` with read compatibility for QuickEdit projects)
- Extra GPU video effects: grain, RGB shift, CRT, glitch, film look
- Built-in look presets and `.effexis-preset.json` I/O
- MCP tools for listing, applying, clearing and saving effect presets

## Clean-room policy

DaVinci Resolve and Effect.app may be studied with REA when you have legal
authority. EffeXis does **not** ship proprietary code, assets, shaders or
binaries from those products. REA is a development research tool only; it is
not a runtime dependency of the editor.

## License

EffeXis source changes are under the same BSD 3-Clause License as QuickEdit.
Third-party components keep their own licenses; see `THIRD-PARTY-NOTICES.md`
and `LICENSE`.
