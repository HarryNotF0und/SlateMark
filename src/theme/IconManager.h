#pragma once

#include <QColor>
#include <QHash>
#include <QIcon>
#include <QObject>
#include <QSize>

class IconManager : public QObject
{
    Q_OBJECT

public:
    static IconManager& instance();

    QIcon icon(const QString& name, const QColor& color, const QSize& size = QSize(24, 24));
    void clearCache();

private:
    explicit IconManager(QObject* parent = nullptr);
    QHash<QString, QIcon> m_cache;
};

