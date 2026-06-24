#pragma once

#include <QFont>
#include <QPlainTextEdit>

class QDragEnterEvent;
class QDropEvent;
class QKeyEvent;
class QPaintEvent;
class QResizeEvent;
class LineNumberArea;
class MarkdownHighlighter;

class MarkdownEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit MarkdownEditor(QWidget* parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void setEditorFont(const QFont& font);
    void setDarkTheme(bool dark);
    void setTabWidthSpaces(int spaces);
    void setShowLineNumbers(bool show);
    void gotoLine(int line);
    void insertWrapped(const QString& before, const QString& after, const QString& placeholder);
    void insertBlockText(const QString& markdown);
    void setHeadingLevel(int level);
    void indentSelection(int delta);

signals:
    void fileDropped(const QString& path);
    void imageDropped(const QString& path);
    void cursorLineColumnChanged(int line, int column);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);
    void publishCursorInfo();

private:
    bool handlePairCompletion(QKeyEvent* event);
    void reindentSelection(const QString& prefix, bool remove);

    LineNumberArea* m_lineNumberArea = nullptr;
    MarkdownHighlighter* m_highlighter = nullptr;
    bool m_darkTheme = true;
    bool m_showLineNumbers = true;
    int m_tabSpaces = 4;
};
