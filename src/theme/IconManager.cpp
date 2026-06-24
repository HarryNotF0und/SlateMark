#include "theme/IconManager.h"

#include <QFile>
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QtMath>

IconManager& IconManager::instance()
{
    static IconManager manager;
    return manager;
}

IconManager::IconManager(QObject* parent)
    : QObject(parent)
{
}

QIcon IconManager::icon(const QString& name, const QColor& color, const QSize& size)
{
    const QString key = name + color.name(QColor::HexArgb) + QStringLiteral("%1x%2").arg(size.width()).arg(size.height());
    if (m_cache.contains(key)) {
        return m_cache.value(key);
    }

    QFile file(QStringLiteral(":/icons/%1.svg").arg(name));
    if (!file.open(QIODevice::ReadOnly)) {
        file.setFileName(QStringLiteral(":/icons/generic.svg"));
        file.open(QIODevice::ReadOnly);
    }
    QByteArray svg = file.readAll();
    svg.replace("currentColor", color.name().toUtf8());

    QSvgRenderer renderer(svg);
    const qreal ratio = qApp->devicePixelRatio();
    QPixmap pixmap(QSize(qCeil(size.width() * ratio), qCeil(size.height() * ratio)));
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    const qreal inset = qMax<qreal>(1.5, qMin(size.width(), size.height()) * 0.08);
    renderer.render(&painter, QRectF(inset, inset, size.width() - inset * 2, size.height() - inset * 2));
    QIcon out(pixmap);
    m_cache.insert(key, out);
    return out;
}

void IconManager::clearCache()
{
    m_cache.clear();
}
