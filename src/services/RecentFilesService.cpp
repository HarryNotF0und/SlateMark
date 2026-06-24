#include "services/RecentFilesService.h"

#include <QSettings>

RecentFilesService::RecentFilesService(QObject* parent)
    : QObject(parent)
{
}

QStringList RecentFilesService::files() const
{
    return QSettings(QStringLiteral("SlateMark"), QStringLiteral("SlateMark")).value(QStringLiteral("recentFiles")).toStringList();
}

void RecentFilesService::addFile(const QString& path)
{
    QStringList list = files();
    list.removeAll(path);
    list.prepend(path);
    while (list.size() > 10) {
        list.removeLast();
    }
    QSettings(QStringLiteral("SlateMark"), QStringLiteral("SlateMark")).setValue(QStringLiteral("recentFiles"), list);
    emit changed();
}

void RecentFilesService::clear()
{
    QSettings(QStringLiteral("SlateMark"), QStringLiteral("SlateMark")).remove(QStringLiteral("recentFiles"));
    emit changed();
}

