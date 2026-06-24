#include "widgets/ViewModeSwitcher.h"

#include "widgets/SvgIconButton.h"

#include <QHBoxLayout>

ViewModeSwitcher::ViewModeSwitcher(ThemeManager* theme, QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("viewModeSwitcher"));
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(4);

    const QVector<QPair<QString, ViewMode>> specs = {
        {QStringLiteral("editor-only"), ViewMode::EditorOnly},
        {QStringLiteral("split-view"), ViewMode::Split},
        {QStringLiteral("preview"), ViewMode::PreviewOnly}
    };
    for (const auto& spec : specs) {
        auto* button = new SvgIconButton(spec.first, theme, this);
        button->setFixedSize(38, 34);
        button->setToolTip(spec.first);
        layout->addWidget(button);
        m_buttons.push_back(button);
        connect(button, &QToolButton::clicked, this, [this, spec] {
            setMode(spec.second);
            emit modeChanged(spec.second);
        });
    }
    setMode(ViewMode::Split);
}

void ViewModeSwitcher::setMode(ViewMode mode)
{
    m_mode = mode;
    for (int i = 0; i < m_buttons.size(); ++i) {
        m_buttons.at(i)->setSelected((i == 0 && mode == ViewMode::EditorOnly)
            || (i == 1 && mode == ViewMode::Split)
            || (i == 2 && mode == ViewMode::PreviewOnly));
    }
}

