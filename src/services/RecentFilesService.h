#pragma once

#include <QObject>
#include <QStringList>

class RecentFilesService : public QObject
{
    Q_OBJECT

public:
    explicit RecentFilesService(QObject* parent = nullptr);
    QStringList files() const;
    void addFile(const QString& path);
    void clear();

signals:
    void changed();
};

