#pragma once

#include <QTabBar>

class QMouseEvent;

class DocumentTabBar : public QTabBar
{
    Q_OBJECT

public:
    explicit DocumentTabBar(QWidget* parent = nullptr);

signals:
    void middleCloseRequested(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QRect closeButtonRect(int index) const;
};
