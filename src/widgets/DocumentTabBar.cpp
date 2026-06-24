#include "widgets/DocumentTabBar.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionTab>

DocumentTabBar::DocumentTabBar(QWidget* parent)
    : QTabBar(parent)
{
    setObjectName(QStringLiteral("documentTabBar"));
    setTabsClosable(false);
    setMovable(true);
    setExpanding(false);
    setDrawBase(false);
    setElideMode(Qt::ElideRight);
    setUsesScrollButtons(true);
    setDocumentMode(true);
    setFixedHeight(44);
}

QRect DocumentTabBar::closeButtonRect(int index) const
{
    const QRect rect = tabRect(index);
    return QRect(rect.right() - 28, rect.center().y() - 8, 16, 16);
}

void DocumentTabBar::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < count(); ++i) {
        const QRect rect = tabRect(i);
        if (!event->rect().intersects(rect)) {
            continue;
        }

        QStyleOptionTab option;
        initStyleOption(&option, i);
        option.text.clear();
        style()->drawControl(QStyle::CE_TabBarTabShape, &option, &painter, this);

        const int centerY = rect.center().y();
        QRect textRect = rect.adjusted(14, 0, -40, 0);
        textRect.setTop(centerY - fontMetrics().height() / 2);
        textRect.setHeight(fontMetrics().height());
        painter.setPen(palette().color(QPalette::Text));
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
            fontMetrics().elidedText(tabText(i), Qt::ElideRight, textRect.width()));

        QPen pen(palette().color(QPalette::Text));
        pen.setWidthF(1.8);
        painter.setPen(pen);
        const QRect closeRect = closeButtonRect(i).adjusted(3, 3, -3, -3);
        painter.drawLine(closeRect.topLeft(), closeRect.bottomRight());
        painter.drawLine(closeRect.topRight(), closeRect.bottomLeft());
    }
}

void DocumentTabBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        const int index = tabAt(event->pos());
        if (index >= 0 && closeButtonRect(index).contains(event->pos())) {
            emit tabCloseRequested(index);
            return;
        }
    }
    if (event->button() == Qt::MiddleButton) {
        const int index = tabAt(event->pos());
        if (index >= 0) {
            emit middleCloseRequested(index);
            return;
        }
    }
    QTabBar::mouseReleaseEvent(event);
}
