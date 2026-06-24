#include "widgets/SvgIconButton.h"

#include "theme/IconManager.h"

#include <QMouseEvent>
#include <QStyle>

SvgIconButton::SvgIconButton(const QString& iconName, ThemeManager* theme, QWidget* parent)
    : QToolButton(parent)
    , m_iconName(iconName)
    , m_theme(theme)
{
    setObjectName(QStringLiteral("svgIconButton"));
    setAutoRaise(true);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(42, 42);
    setIconSize(QSize(19, 19));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    if (m_theme) {
        connect(m_theme, &ThemeManager::themeChanged, this, &SvgIconButton::refreshIcon);
    }
    refreshIcon();
}

void SvgIconButton::setIconName(const QString& iconName)
{
    m_iconName = iconName;
    refreshIcon();
}

QString SvgIconButton::iconName() const { return m_iconName; }

void SvgIconButton::setSelected(bool selected)
{
    m_selected = selected;
    setProperty("selected", selected);
    style()->unpolish(this);
    style()->polish(this);
    refreshIcon();
}

void SvgIconButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    refreshIcon();
    QToolButton::enterEvent(event);
}

void SvgIconButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    refreshIcon();
    QToolButton::leaveEvent(event);
}

void SvgIconButton::mousePressEvent(QMouseEvent* event)
{
    m_pressed = true;
    refreshIcon();
    QToolButton::mousePressEvent(event);
}

void SvgIconButton::mouseReleaseEvent(QMouseEvent* event)
{
    m_pressed = false;
    refreshIcon();
    QToolButton::mouseReleaseEvent(event);
}

QColor SvgIconButton::currentColor() const
{
    if (!isEnabled()) {
        return QColor(QStringLiteral("#6b7280"));
    }
    if (m_selected || m_pressed) {
        return m_theme ? m_theme->accentColor() : QColor(QStringLiteral("#5aa7ff"));
    }
    if (m_hovered) {
        return m_theme ? m_theme->textColor() : QColor(QStringLiteral("#e8edf4"));
    }
    return m_theme ? m_theme->mutedTextColor() : QColor(QStringLiteral("#9aa4b2"));
}

void SvgIconButton::refreshIcon()
{
    setIcon(IconManager::instance().icon(m_iconName, currentColor(), iconSize()));
}
