#include "services/LoggingService.h"
#include "window/MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SlateMark"));
    app.setOrganizationName(QStringLiteral("SlateMark"));
    LoggingService::install();

    MainWindow window;
    window.show();
    return app.exec();
}

