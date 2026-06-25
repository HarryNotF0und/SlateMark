#include "services/LoggingService.h"
#include "window/MainWindow.h"

#include <QApplication>
#include <QByteArray>
#include <QIcon>

namespace {
void configureWebEngineForPreview()
{
    const QByteArray previewFlags =
        "--disable-background-networking "
        "--disable-component-update "
        "--disable-domain-reliability "
        "--disable-extensions "
        "--disable-features=BackForwardCache,CalculateNativeWinOcclusion,MediaRouter,OptimizationHints,PaintHolding,Translate "
        "--disable-gpu "
        "--disable-plugins "
        "--disable-print-preview "
        "--disable-speech-api "
        "--disable-sync "
        "--metrics-recording-only "
        "--mute-audio "
        "--no-default-browser-check "
        "--no-first-run";

    const QByteArray existing = qgetenv("QTWEBENGINE_CHROMIUM_FLAGS");
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", existing.isEmpty() ? previewFlags : existing + ' ' + previewFlags);
}
}

int main(int argc, char* argv[])
{
    configureWebEngineForPreview();

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SlateMark"));
    app.setOrganizationName(QStringLiteral("SlateMark"));
#if defined(SLATEMARK_DESKTOP_FILE_NAME)
    app.setDesktopFileName(QStringLiteral(SLATEMARK_DESKTOP_FILE_NAME));
#endif
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app-icon.png")));
    LoggingService::install();

    MainWindow window;
    window.show();
    return app.exec();
}
