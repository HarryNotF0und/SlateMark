#include "editor/MarkdownHighlighter.h"

#include <QColor>
#include <QFont>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    rebuildFormats();
}

void MarkdownHighlighter::setDarkTheme(bool dark)
{
    if (m_darkTheme == dark) {
        return;
    }
    m_darkTheme = dark;
    rebuildFormats();
    rehighlight();
}

void MarkdownHighlighter::rebuildFormats()
{
    QTextCharFormat heading;
    heading.setForeground(QColor(QStringLiteral("#5aa7ff")));
    heading.setFontWeight(QFont::DemiBold);

    QTextCharFormat emphasis;
    emphasis.setFontItalic(true);
    emphasis.setForeground(QColor(m_darkTheme ? QStringLiteral("#d8dee9") : QStringLiteral("#4b5563")));

    QTextCharFormat strong;
    strong.setFontWeight(QFont::Bold);
    strong.setForeground(QColor(m_darkTheme ? QStringLiteral("#f2f5f8") : QStringLiteral("#111827")));

    QTextCharFormat code;
    code.setForeground(QColor(m_darkTheme ? QStringLiteral("#8bd49c") : QStringLiteral("#0f766e")));
    code.setBackground(QColor(m_darkTheme ? QStringLiteral("#202833") : QStringLiteral("#eef6ff")));

    QTextCharFormat link;
    link.setForeground(QColor(m_darkTheme ? QStringLiteral("#78b8ff") : QStringLiteral("#1d7fe8")));
    link.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    QTextCharFormat quote;
    quote.setForeground(QColor(m_darkTheme ? QStringLiteral("#aab3c2") : QStringLiteral("#6b7280")));
    quote.setFontItalic(true);

    m_codeBlockFormat = code;
    m_rules = {
        {QRegularExpression(QStringLiteral("^#{1,6}\\s+.*$")), heading},
        {QRegularExpression(QStringLiteral("\\*\\*[^*]+\\*\\*")), strong},
        {QRegularExpression(QStringLiteral("~~[^~]+~~")), emphasis},
        {QRegularExpression(QStringLiteral("(^|\\s)\\*[^*]+\\*")), emphasis},
        {QRegularExpression(QStringLiteral("`[^`]+`")), code},
        {QRegularExpression(QStringLiteral("!?\\[[^\\]]+\\]\\([^\\)]+\\)")), link},
        {QRegularExpression(QStringLiteral("^>.*$")), quote},
        {QRegularExpression(QStringLiteral("^\\s*([-*+]|\\d+[.)])\\s+.*$")), quote}
    };
}

void MarkdownHighlighter::highlightBlock(const QString& text)
{
    if (previousBlockState() == 1) {
        setCurrentBlockState(1);
        setFormat(0, text.size(), m_codeBlockFormat);
        if (text.trimmed().startsWith(QStringLiteral("```"))) {
            setCurrentBlockState(0);
        }
        return;
    }

    if (text.trimmed().startsWith(QStringLiteral("```"))) {
        setCurrentBlockState(1);
        setFormat(0, text.size(), m_codeBlockFormat);
        return;
    }

    setCurrentBlockState(0);
    for (const Rule& rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
