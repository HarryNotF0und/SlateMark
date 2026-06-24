#include "services/LoggingService.h"
#include "window/MainWindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SlateMark"));
    app.setOrganizationName(QStringLiteral("SlateMark"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app-icon.png")));
    LoggingService::install();

    MainWindow window;
    window.show();
    return app.exec();
}
