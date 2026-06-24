#include "widgets/StatusBarWidget.h"

#include <QHBoxLayout>

StatusBarWidget::StatusBarWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("statusBarWidget"));
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(16);
    m_left = new QLabel(this);
    m_center = new QLabel(this);
    m_right = new QLabel(this);
    layout->addWidget(m_left);
    layout->addWidget(m_center);
    layout->addStretch();
    layout->addWidget(m_right);
    setFixedHeight(32);
    setCursorInfo(1, 1);
    setSaved(true);
}

void StatusBarWidget::setCursorInfo(int line, int column)
{
    m_line = line;
    m_column = column;
    m_left->setText(QStringLiteral("Ln %1, Col %2").arg(line).arg(column));
}

void StatusBarWidget::setStatistics(const DocumentStatistics& stats)
{
    m_stats = stats;
    m_center->setText(QStringLiteral("%1 lines   %2 words   %3 chars   UTF-8   Markdown")
        .arg(stats.lines)
        .arg(stats.words)
        .arg(stats.characters));
}

void StatusBarWidget::setSaved(bool saved)
{
    const QString existing = m_right->text();
    const QString zoom = existing.contains('%') ? existing.section(' ', -1) : QStringLiteral("100%");
    m_right->setText(QStringLiteral("%1   %2").arg(saved ? QStringLiteral("Saved") : QStringLiteral("Unsaved"), zoom));
}

void StatusBarWidget::setZoom(int percent)
{
    const bool saved = !m_right->text().startsWith(QStringLiteral("Unsaved"));
    m_right->setText(QStringLiteral("%1   %2%").arg(saved ? QStringLiteral("Saved") : QStringLiteral("Unsaved")).arg(percent));
}

