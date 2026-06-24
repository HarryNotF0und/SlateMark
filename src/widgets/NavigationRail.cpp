#include "widgets/NavigationRail.h"

#include "widgets/SvgIconButton.h"

#include <QVBoxLayout>

NavigationRail::NavigationRail(ThemeManager* theme, QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("navigationRail"));
    setFixedWidth(66);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto* tools = new SvgIconButton(QStringLiteral("sidebar"), theme, this);
    tools->setToolTip(QStringLiteral("Toggle tools"));
    tools->setSelected(true);
    layout->addWidget(tools, 0, Qt::AlignHCenter);
    m_buttons.push_back(tools);
    connect(tools, &QToolButton::clicked, this, &NavigationRail::toolPanelToggled);

    layout->addStretch();
    auto* settings = new SvgIconButton(QStringLiteral("settings"), theme, this);
    settings->setToolTip(QStringLiteral("Settings"));
    connect(settings, &QToolButton::clicked, this, &NavigationRail::settingsRequested);
    layout->addWidget(settings, 0, Qt::AlignHCenter);
}
