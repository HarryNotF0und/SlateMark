#pragma once

#include <QObject>
#include <QPalette>

enum class ThemeMode
{
    Dark,
    Light,
    System
};

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    explicit ThemeManager(QObject* parent = nullptr);

    ThemeMode mode() const;
    void setMode(ThemeMode mode);
    QString styleSheet() const;
    QColor textColor() const;
    QColor mutedTextColor() const;
    QColor accentColor() const;
    QColor panelColor() const;
    bool isDark() const;

signals:
    void themeChanged();

private:
    ThemeMode m_mode = ThemeMode::Dark;
};

