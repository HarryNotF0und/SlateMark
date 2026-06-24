#include "services/FileService.h"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QTextStream>

FileResult FileService::readTextFile(const QString& path)
{
    QFile file(path);
    if (!file.exists()) {
        return {false, {}, QStringLiteral("File does not exist: %1").arg(path)};
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {false, {}, file.errorString()};
    }
    return {true, QString::fromUtf8(file.readAll()), {}};
}

FileResult FileService::writeTextFile(const QString& path, const QString& text)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return {false, {}, file.errorString()};
    }
    file.write(text.toUtf8());
    if (!file.commit()) {
        return {false, {}, file.errorString()};
    }
    return {true, {}, {}};
}

bool FileService::isSupportedMarkdownPath(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QStringLiteral("md") || suffix == QStringLiteral("markdown") || suffix == QStringLiteral("txt");
}
