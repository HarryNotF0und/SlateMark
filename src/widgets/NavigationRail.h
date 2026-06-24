#pragma once

#include "theme/ThemeManager.h"

#include <QVector>
#include <QWidget>

class SvgIconButton;

class NavigationRail : public QWidget
{
    Q_OBJECT

public:
    explicit NavigationRail(ThemeManager* theme, QWidget* parent = nullptr);

signals:
    void toolPanelToggled();
    void settingsRequested();

private:
    QVector<SvgIconButton*> m_buttons;
};
