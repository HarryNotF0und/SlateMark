#pragma once

#include "services/MarkdownService.h"
#include "theme/ThemeManager.h"

#include <QLabel>
#include <QPair>
#include <QVector>
#include <QWidget>

class QVBoxLayout;

class ToolPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ToolPanel(ThemeManager* theme, QWidget* parent = nullptr);
    void setStatistics(const DocumentStatistics& stats);
    void setOutline(const QVector<MarkdownHeading>& headings);

signals:
    void formatRequested(const QString& before, const QString& after, const QString& placeholder);
    void headingRequested(int level);
    void blockRequested(const QString& markdown);
    void tableRequested();
    void outlineLineRequested(int line);

private:
    struct ActionSpec
    {
        QString id;
        QString icon;
        QString tooltip;
    };

    QWidget* createSection(const QString& title, const QVector<ActionSpec>& actions);
    QLabel* m_stats = nullptr;
    QVBoxLayout* m_outlineLayout = nullptr;
    ThemeManager* m_theme = nullptr;
};
