# Packaging Notes

Use `windeployqt --webengine` before wrapping the app in an installer. Inno Setup, WiX, and MSIX can all package the deployed folder as long as `QtWebEngineProcess.exe` and the WebEngine resource directories are included.

