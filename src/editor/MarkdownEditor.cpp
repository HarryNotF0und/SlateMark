#include "editor/MarkdownEditor.h"

#include "editor/LineNumberArea.h"
#include "editor/MarkdownHighlighter.h"
#include "services/FileService.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextBlock>
#include <QUrl>

LineNumberArea::LineNumberArea(MarkdownEditor* editor)
    : QWidget(editor)
    , m_editor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}

MarkdownEditor::MarkdownEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setObjectName(QStringLiteral("markdownEditor"));
    setAcceptDrops(true);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setUndoRedoEnabled(true);
    setTabChangesFocus(false);
    m_lineNumberArea = new LineNumberArea(this);
    m_highlighter = new MarkdownHighlighter(document());

    connect(this, &QPlainTextEdit::blockCountChanged, this, &MarkdownEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &MarkdownEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &MarkdownEditor::highlightCurrentLine);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &MarkdownEditor::publishCursorInfo);

    setEditorFont(QFont(QStringLiteral("Cascadia Mono"), 12));
    setTabWidthSpaces(4);
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    publishCursorInfo();
}

int MarkdownEditor::lineNumberAreaWidth() const
{
    if (!m_showLineNumbers) {
        return 0;
    }
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    return 18 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void MarkdownEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(m_darkTheme ? QStringLiteral("#121820") : QStringLiteral("#fbfcfe")));
    painter.setPen(QColor(m_darkTheme ? QStringLiteral("#8a94a3") : QStringLiteral("#8a96a8")));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.drawText(0, top, m_lineNumberArea->width() - 8, fontMetrics().height(),
                Qt::AlignRight, QString::number(blockNumber + 1));
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void MarkdownEditor::setEditorFont(const QFont& font)
{
    setFont(font);
    updateLineNumberAreaWidth(0);
}

void MarkdownEditor::setDarkTheme(bool dark)
{
    m_darkTheme = dark;
    m_highlighter->setDarkTheme(dark);
    highlightCurrentLine();
    m_lineNumberArea->update();
}

void MarkdownEditor::setTabWidthSpaces(int spaces)
{
    m_tabSpaces = qBound(2, spaces, 12);
    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * m_tabSpaces);
}

void MarkdownEditor::setShowLineNumbers(bool show)
{
    m_showLineNumbers = show;
    updateLineNumberAreaWidth(0);
}

void MarkdownEditor::gotoLine(int line)
{
    QTextBlock block = document()->findBlockByNumber(qMax(0, line - 1));
    if (block.isValid()) {
        QTextCursor cursor(block);
        setTextCursor(cursor);
        centerCursor();
        setFocus();
    }
}

void MarkdownEditor::insertWrapped(const QString& before, const QString& after, const QString& placeholder)
{
    QTextCursor cursor = textCursor();
    const QString selected = cursor.selectedText();
    const int start = cursor.selectionStart();
    cursor.insertText(before + (selected.isEmpty() ? placeholder : selected) + after);
    if (selected.isEmpty()) {
        cursor.setPosition(start + before.size());
        cursor.setPosition(start + before.size() + placeholder.size(), QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }
}

void MarkdownEditor::insertBlockText(const QString& markdown)
{
    QTextCursor cursor = textCursor();
    if (!cursor.atBlockStart()) {
        cursor.insertText(QStringLiteral("\n"));
    }
    cursor.insertText(markdown + QStringLiteral("\n"));
}

void MarkdownEditor::setHeadingLevel(int level)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    QTextBlock start = document()->findBlock(cursor.selectionStart());
    QTextBlock end = document()->findBlock(cursor.selectionEnd());
    for (QTextBlock block = start; block.isValid(); block = block.next()) {
        QTextCursor lineCursor(block);
        lineCursor.select(QTextCursor::LineUnderCursor);
        QString text = lineCursor.selectedText();
        text.remove(QRegularExpression(QStringLiteral("^#{1,6}\\s+")));
        if (level > 0) {
            text.prepend(QString(level, '#') + ' ');
        }
        lineCursor.insertText(text);
        if (block == end) break;
    }
    cursor.endEditBlock();
}

void MarkdownEditor::indentSelection(int delta)
{
    reindentSelection(QString(m_tabSpaces, ' '), delta < 0);
}

void MarkdownEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);
    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void MarkdownEditor::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Tab) {
        indentSelection(1);
        return;
    }
    if (event->key() == Qt::Key_Backtab) {
        indentSelection(-1);
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTextCursor cursor = textCursor();
        const QString current = cursor.block().text();
        QPlainTextEdit::keyPressEvent(event);
        QRegularExpressionMatch match = QRegularExpression(QStringLiteral("^(\\s*)([-*+]|\\d+[.)]|>)\\s+")).match(current);
        if (match.hasMatch()) {
            insertPlainText(match.captured(1) + match.captured(2) + ' ');
        }
        return;
    }
    if (handlePairCompletion(event)) {
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
}

void MarkdownEditor::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }
    QPlainTextEdit::dragEnterEvent(event);
}

void MarkdownEditor::dropEvent(QDropEvent* event)
{
    for (const QUrl& url : event->mimeData()->urls()) {
        const QString path = url.toLocalFile();
        const QString suffix = QFileInfo(path).suffix().toLower();
        if (FileService::isSupportedMarkdownPath(path)) {
            emit fileDropped(path);
            return;
        }
        if (QStringList{QStringLiteral("png"), QStringLiteral("jpg"), QStringLiteral("jpeg"), QStringLiteral("webp"), QStringLiteral("gif"), QStringLiteral("svg")}.contains(suffix)) {
            emit imageDropped(path);
            return;
        }
    }
    QPlainTextEdit::dropEvent(event);
}

void MarkdownEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void MarkdownEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(m_darkTheme ? QStringLiteral("#1b2430") : QStringLiteral("#eaf4ff")));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }
    setExtraSelections(selections);
}

void MarkdownEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void MarkdownEditor::publishCursorInfo()
{
    const QTextCursor cursor = textCursor();
    emit cursorLineColumnChanged(cursor.blockNumber() + 1, cursor.positionInBlock() + 1);
}

bool MarkdownEditor::handlePairCompletion(QKeyEvent* event)
{
    const QString text = event->text();
    const QHash<QString, QString> pairs = {
        {QStringLiteral("("), QStringLiteral(")")},
        {QStringLiteral("["), QStringLiteral("]")},
        {QStringLiteral("{"), QStringLiteral("}")},
        {QStringLiteral("\""), QStringLiteral("\"")},
        {QStringLiteral("'"), QStringLiteral("'")},
        {QStringLiteral("`"), QStringLiteral("`")}
    };
    if (!pairs.contains(text)) {
        return false;
    }
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        const QString selected = cursor.selectedText();
        cursor.insertText(text + selected + pairs.value(text));
    } else {
        cursor.insertText(text + pairs.value(text));
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }
    return true;
}

void MarkdownEditor::reindentSelection(const QString& prefix, bool remove)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    QTextBlock start = document()->findBlock(cursor.selectionStart());
    QTextBlock end = document()->findBlock(cursor.selectionEnd());
    for (QTextBlock block = start; block.isValid(); block = block.next()) {
        QTextCursor lineCursor(block);
        lineCursor.movePosition(QTextCursor::StartOfBlock);
        if (remove) {
            QString text = block.text();
            int count = 0;
            while (count < prefix.size() && count < text.size() && text.at(count) == ' ') {
                ++count;
            }
            for (int i = 0; i < count; ++i) {
                lineCursor.deleteChar();
            }
        } else {
            lineCursor.insertText(prefix);
        }
        if (block == end) break;
    }
    cursor.endEditBlock();
}
