# Packaging Notes

## Linux

Use the CMake install target after a release build:

```bash
cmake --preset linux-release
cmake --build --preset linux-release
cmake --install build/linux-release --prefix /usr
```

The install target places:

- `SlateMark` under the configured binary directory.
- `io.github.HarryNotF0und.SlateMark.desktop` under `share/applications`.
- `io.github.HarryNotF0und.SlateMark.metainfo.xml` under `share/metainfo`.
- `io.github.HarryNotF0und.SlateMark.png` under `share/icons/hicolor/512x512/apps`.

Package maintainers should run `update-desktop-database` and `gtk-update-icon-cache` in their package scripts when their distribution expects it. Qt WebEngine runtime files should come from the distribution Qt packages.

## Windows

Use `windeployqt --webengine` before wrapping the app in an installer. Inno Setup, WiX, and MSIX can all package the deployed folder as long as `QtWebEngineProcess.exe` and the WebEngine resource directories are included.
