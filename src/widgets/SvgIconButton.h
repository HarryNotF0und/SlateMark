#pragma once

#include "theme/ThemeManager.h"

#include <QToolButton>

class QEnterEvent;
class QEvent;
class QMouseEvent;

class SvgIconButton : public QToolButton
{
    Q_OBJECT

public:
    explicit SvgIconButton(const QString& iconName, ThemeManager* theme, QWidget* parent = nullptr);
    void setIconName(const QString& iconName);
    QString iconName() const;
    void setSelected(bool selected);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
    void refreshIcon();

private:
    QColor currentColor() const;

    QString m_iconName;
    ThemeManager* m_theme = nullptr;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_selected = false;
};
