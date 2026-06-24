cmake_minimum_required(VERSION 3.24)

project(SlateMark VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Svg
    Concurrent
    Test
)
find_package(Qt6 6.5 QUIET COMPONENTS
    PrintSupport
    Quick
    QuickWidgets
    WebEngineCore
    WebEngineWidgets
    WebChannel
)

if(TARGET Qt6::WebEngineWidgets)
    set(SLATEMARK_HAS_WEBENGINE ON)
else()
    set(SLATEMARK_HAS_WEBENGINE OFF)
    message(WARNING "Qt WebEngineWidgets was not found. SlateMark will build with the QTextBrowser preview fallback. Install Qt WebEngine for full preview support.")
endif()

if(TARGET Qt6::WebChannel)
    set(SLATEMARK_HAS_WEBCHANNEL ON)
else()
    set(SLATEMARK_HAS_WEBCHANNEL OFF)
    message(WARNING "Qt WebChannel was not found. QWebEngineView preview will build without preview-to-editor bridge callbacks.")
endif()

set(SLATEMARK_COMMON_SOURCES
    src/documents/Document.h
    src/services/MarkdownService.h
    src/services/MarkdownService.cpp
    src/services/FileService.h
    src/services/FileService.cpp
    src/services/SettingsService.h
    src/services/SettingsService.cpp
    src/services/RecentFilesService.h
    src/services/RecentFilesService.cpp
    src/services/RecoveryService.h
    src/services/RecoveryService.cpp
    src/services/ExportService.h
    src/services/ExportService.cpp
    src/services/StatisticsService.h
    src/services/StatisticsService.cpp
    src/services/OutlineService.h
    src/services/OutlineService.cpp
    src/services/LoggingService.h
    src/services/LoggingService.cpp
    src/services/ImageService.h
    src/services/ImageService.cpp
    src/services/ScrollSyncService.h
    src/services/ScrollSyncService.cpp
    src/commands/CommandRegistry.h
    src/commands/CommandRegistry.cpp
    src/theme/ThemeManager.h
    src/theme/ThemeManager.cpp
    src/theme/IconManager.h
    src/theme/IconManager.cpp
)

set(SLATEMARK_APP_SOURCES
    src/main.cpp
    src/window/MainWindow.h
    src/window/MainWindow.cpp
    src/editor/MarkdownEditor.h
    src/editor/MarkdownEditor.cpp
    src/editor/LineNumberArea.h
    src/editor/MarkdownHighlighter.h
    src/editor/MarkdownHighlighter.cpp
    src/preview/PreviewWidget.h
    src/preview/PreviewWidget.cpp
    src/preview/PreviewBridge.h
    src/preview/PreviewBridge.cpp
    src/widgets/SvgIconButton.h
    src/widgets/SvgIconButton.cpp
    src/widgets/NavigationRail.h
    src/widgets/NavigationRail.cpp
    src/widgets/ToolPanel.h
    src/widgets/ToolPanel.cpp
    src/widgets/DocumentTabBar.h
    src/widgets/DocumentTabBar.cpp
    src/widgets/ViewModeSwitcher.h
    src/widgets/ViewModeSwitcher.cpp
    src/widgets/StatusBarWidget.h
    src/widgets/StatusBarWidget.cpp
    src/widgets/CommandPalette.h
    src/widgets/CommandPalette.cpp
    src/dialogs/ExportDialog.h
    src/dialogs/ExportDialog.cpp
    src/dialogs/TableDialog.h
    src/dialogs/TableDialog.cpp
    src/dialogs/SettingsDialog.h
    src/dialogs/SettingsDialog.cpp
)

if(WIN32)
    list(APPEND SLATEMARK_APP_SOURCES src/app/slatemark.rc)
endif()

qt_add_executable(SlateMark WIN32
    ${SLATEMARK_COMMON_SOURCES}
    ${SLATEMARK_APP_SOURCES}
    resources/resources.qrc
)

target_include_directories(SlateMark PRIVATE src)
target_link_libraries(SlateMark PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    Qt6::Concurrent
)
if(SLATEMARK_HAS_WEBENGINE)
    target_compile_definitions(SlateMark PRIVATE SLATEMARK_HAS_WEBENGINE=1)
    target_link_libraries(SlateMark PRIVATE Qt6::WebEngineWidgets)
endif()
if(SLATEMARK_HAS_WEBCHANNEL)
    target_compile_definitions(SlateMark PRIVATE SLATEMARK_HAS_WEBCHANNEL=1)
    target_link_libraries(SlateMark PRIVATE Qt6::WebChannel)
endif()

enable_testing()

qt_add_executable(SlateMarkTests
    tests/test_core.cpp
    ${SLATEMARK_COMMON_SOURCES}
    resources/resources.qrc
)
target_include_directories(SlateMarkTests PRIVATE src)
target_link_libraries(SlateMarkTests PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::Test)
add_test(NAME SlateMarkTests COMMAND SlateMarkTests)
