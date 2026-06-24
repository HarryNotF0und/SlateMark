# SlateMark

SlateMark is a Qt 6 / C++20 Windows Markdown editor built with native Qt Widgets, a `QPlainTextEdit` editor, and `QWebEngineView` only for live preview.

## Implemented

- Native Qt Widgets shell with menu bar, document tabs, navigation rail, collapsible tools panel, splitter editor/preview layout, view modes, and status bar.
- Markdown editor with line numbers, current line highlight, Markdown syntax highlighting, auto indent, Tab and Shift+Tab indentation, pair completion, drag-open Markdown files, and drag-in image Markdown insertion.
- Live preview with 180 ms debounce, safe escaped Markdown conversion, task lists, tables, code blocks with copy buttons, Mermaid fenced blocks as inspectable source, and math spans.
- File operations: new, open, save, save as, close tab with unsaved prompts, recent files, and safe writes via `QSaveFile`.
- Multiple tabs with close buttons, middle-click close, and movable tabs.
- HTML export dialog and complete HTML5 output.
- Settings dialog with persisted theme, editor font size, tab width, wrap, scroll sync, auto save, and auto-save interval.
- Auto recovery copies under `QStandardPaths::AppDataLocation`.
- Core Qt Test coverage for Markdown conversion, file I/O, HTML export, recent files, settings, recovery, formatting helpers, outline parsing, and statistics.
- SVG icon pipeline through Qt resources and a dynamic `IconManager` that recolors cached SVGs per theme/state.

## Current limitations

- The Markdown converter is an internal safe subset implementation. The architecture keeps it isolated in `MarkdownService`; replace it with md4c or cmark-gfm for full GitHub Flavored Markdown parity.
- KaTeX, Mermaid, and syntax highlighter JavaScript libraries are not bundled in this scaffold, so math and Mermaid are presented safely/styled but not fully rendered.
- The app uses the native Windows title bar to keep Snap Layout, resize behavior, shadows, and system controls reliable.
- Session restore is represented in settings and recovery storage, but automatic reopening of all prior tabs is not yet implemented.

## Dependencies

- Qt 6.5 or newer
- Qt modules: Core, Gui, Widgets, Svg, WebEngineWidgets, WebChannel, Concurrent, Test
- CMake 3.24 or newer
- Ninja
- MSVC 2022 recommended on Windows 10/11; MinGW Qt builds should also work if Qt WebEngine is available

No Electron or web-app shell is used. The preview is the only WebEngine surface.

## Configure and Build

Open a Qt-enabled Developer PowerShell where `cmake`, `ninja`, and the Qt 6 CMake package are on `PATH`.

```powershell
cd C:\Users\SiO2-\Documents\Markdown\SlateMark
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
```

For MinGW:

```powershell
cmake --preset windows-mingw
cmake --build --preset windows-mingw-release
ctest --preset windows-mingw-release
```

If CMake cannot locate Qt, pass `CMAKE_PREFIX_PATH`:

```powershell
cmake --preset windows-msvc -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
```

## Run

```powershell
.\build\windows-msvc\SlateMark.exe
```

## Deploy with windeployqt

From a Qt command prompt:

```powershell
mkdir dist
copy .\build\windows-msvc\SlateMark.exe .\dist\
windeployqt --release --compiler-runtime .\dist\SlateMark.exe
```

Qt WebEngine deployment must include `QtWebEngineProcess.exe`, resources, translations, and ICU files. Qt 6.11's `windeployqt` detects `QWebEngineView` automatically when run from the matching Qt kit.

## Optional Packaging

After `windeployqt`, package the `dist` directory with Inno Setup, WiX, or MSIX. Ensure the installer preserves the deployed Qt WebEngine resource folders beside the executable.

## Third-party Licenses

- Qt: LGPL/commercial depending on your Qt distribution and license.
- This project scaffold: MIT.
- If md4c, cmark-gfm, KaTeX, Mermaid, or highlight.js are added later, document their exact versions and licenses here before redistribution.
