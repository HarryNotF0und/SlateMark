#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct MarkdownHeading
{
    int level = 1;
    int line = 0;
    QString text;
};

struct DocumentStatistics
{
    int characters = 0;
    int charactersNoSpaces = 0;
    int words = 0;
    int lines = 0;
    int paragraphs = 0;
    int headings = 0;
    int images = 0;
    int links = 0;
    int readingMinutes = 1;
};

struct HtmlExportOptions
{
    QString title;
    QString author;
    bool inlineCss = true;
    bool toc = false;
    bool codeCopyButtons = true;
    bool darkTheme = true;
    QString customCss;
};

enum class MathRenderMode
{
    PlainText,
    MathJax
};

class MarkdownService
{
public:
    static QString markdownToHtml(const QString& markdown, MathRenderMode mathMode = MathRenderMode::PlainText);
    static QString completeHtmlDocument(const QString& markdown, const HtmlExportOptions& options);
    static QVector<MarkdownHeading> outline(const QString& markdown);
    static DocumentStatistics statistics(const QString& markdown);

    static QString wrapSelection(const QString& selectedText, const QString& before, const QString& after, const QString& placeholder);
    static QString headingPrefix(int level);
    static QString tableTemplate(int rows, int columns);

private:
    static QString inlineMarkup(QString text, MathRenderMode mathMode);
    static QString escapeHtml(const QString& text);
};
