#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit MarkdownHighlighter(QTextDocument* parent = nullptr);
    void setDarkTheme(bool dark);

protected:
    void highlightBlock(const QString& text) override;

private:
    void rebuildFormats();

    struct Rule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<Rule> m_rules;
    QTextCharFormat m_codeBlockFormat;
    bool m_darkTheme = true;
};
