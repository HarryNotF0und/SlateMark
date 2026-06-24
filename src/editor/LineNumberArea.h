#pragma once

#include <QWidget>

class MarkdownEditor;
class QPaintEvent;

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(MarkdownEditor* editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    MarkdownEditor* m_editor = nullptr;
};
