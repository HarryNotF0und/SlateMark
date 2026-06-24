#pragma once

#include "theme/ThemeManager.h"

#include <QVector>
#include <QWidget>

enum class ViewMode
{
    EditorOnly,
    Split,
    PreviewOnly
};

class ViewModeSwitcher : public QWidget
{
    Q_OBJECT

public:
    explicit ViewModeSwitcher(ThemeManager* theme, QWidget* parent = nullptr);
    void setMode(ViewMode mode);

signals:
    void modeChanged(ViewMode mode);

private:
    QVector<class SvgIconButton*> m_buttons;
    ViewMode m_mode = ViewMode::Split;
};
