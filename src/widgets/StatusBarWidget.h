#pragma once

#include "services/MarkdownService.h"

#include <QLabel>
#include <QWidget>

class StatusBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBarWidget(QWidget* parent = nullptr);
    void setCursorInfo(int line, int column);
    void setStatistics(const DocumentStatistics& stats);
    void setSaved(bool saved);
    void setZoom(int percent);

private:
    QLabel* m_left = nullptr;
    QLabel* m_center = nullptr;
    QLabel* m_right = nullptr;
    int m_line = 1;
    int m_column = 1;
    DocumentStatistics m_stats;
};

