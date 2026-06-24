#include "theme/ThemeManager.h"

#include <QApplication>
#include <QFile>

namespace {
QString readResource(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
}

ThemeMode ThemeManager::mode() const { return m_mode; }

void ThemeManager::setMode(ThemeMode mode)
{
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;
    emit themeChanged();
}

bool ThemeManager::isDark() const
{
    if (m_mode == ThemeMode::System) {
        return qApp->palette().color(QPalette::Window).lightness() < 128;
    }
    return m_mode == ThemeMode::Dark;
}

QString ThemeManager::styleSheet() const
{
    return readResource(isDark() ? QStringLiteral(":/themes/dark.qss") : QStringLiteral(":/themes/light.qss"));
}

QColor ThemeManager::textColor() const { return isDark() ? QColor(QStringLiteral("#e8edf4")) : QColor(QStringLiteral("#1f2937")); }
QColor ThemeManager::mutedTextColor() const { return isDark() ? QColor(QStringLiteral("#96a0ad")) : QColor(QStringLiteral("#5b6472")); }
QColor ThemeManager::accentColor() const { return QColor(QStringLiteral("#5aa7ff")); }
QColor ThemeManager::panelColor() const { return isDark() ? QColor(QStringLiteral("#151b23")) : QColor(QStringLiteral("#f4f6f8")); }
