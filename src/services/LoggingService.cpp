#include "services/LoggingService.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>

namespace {
QMutex& logMutex()
{
    static QMutex mutex;
    return mutex;
}

QString logPath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/slatemark.log");
}
}

void LoggingService::install()
{
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& message) {
        LoggingService::log(message);
    });
}

void LoggingService::log(const QString& message)
{
    QMutexLocker locker(&logMutex());
    QFile file(logPath());
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << message << '\n';
}

