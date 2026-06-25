#include "services/MarkdownService.h"

#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QStringConverter>
#include <QTextStream>

#include <algorithm>

namespace {
QString readResource(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

bool isTableSeparator(const QString& line)
{
    static const QRegularExpression separator(QStringLiteral("^\\s*\\|?\\s*:?-{3,}:?\\s*(\\|\\s*:?-{3,}:?\\s*)+\\|?\\s*$"));
    return separator.match(line).hasMatch();
}

QStringList splitTableRow(QString line)
{
    line = line.trimmed();
    if (line.startsWith('|')) {
        line.remove(0, 1);
    }
    if (line.endsWith('|')) {
        line.chop(1);
    }
    QStringList cells = line.split('|');
    for (QString& cell : cells) {
        cell = cell.trimmed();
    }
    return cells;
}

QString localEscapeHtml(const QString& text)
{
    QString out = text;
    out.replace('&', QStringLiteral("&amp;"));
    out.replace('<', QStringLiteral("&lt;"));
    out.replace('>', QStringLiteral("&gt;"));
    out.replace('"', QStringLiteral("&quot;"));
    return out;
}

QString spanToken(const QString& kind, const QString& text)
{
    return QStringLiteral("<span class=\"tok-%1\">%2</span>").arg(kind, localEscapeHtml(text));
}

QString normalizedCodeLanguage(QString language)
{
    language = language.trimmed().toLower();
    language = language.split(QRegularExpression(QStringLiteral("\\s+"))).value(0);
    if (language == QStringLiteral("c++") || language == QStringLiteral("cxx") || language == QStringLiteral("cc") || language == QStringLiteral("hpp")) return QStringLiteral("cpp");
    if (language == QStringLiteral("c#")) return QStringLiteral("csharp");
    if (language == QStringLiteral("js") || language == QStringLiteral("jsx") || language == QStringLiteral("mjs")) return QStringLiteral("javascript");
    if (language == QStringLiteral("ts") || language == QStringLiteral("tsx")) return QStringLiteral("typescript");
    if (language == QStringLiteral("py") || language == QStringLiteral("py3")) return QStringLiteral("python");
    if (language == QStringLiteral("sh") || language == QStringLiteral("shell") || language == QStringLiteral("zsh")) return QStringLiteral("bash");
    if (language == QStringLiteral("ps1")) return QStringLiteral("powershell");
    if (language == QStringLiteral("yml")) return QStringLiteral("yaml");
    if (language == QStringLiteral("htm")) return QStringLiteral("html");
    return language;
}

bool isIdentifierStart(QChar ch)
{
    return ch.isLetter() || ch == '_' || ch == '$';
}

bool isIdentifierPart(QChar ch)
{
    return ch.isLetterOrNumber() || ch == '_' || ch == '$' || ch == '-';
}

QString highlightMarkupCode(const QString& code)
{
    QString out;
    int i = 0;
    while (i < code.size()) {
        if (code.mid(i, 4) == QStringLiteral("<!--")) {
            const int end = code.indexOf(QStringLiteral("-->"), i + 4);
            const int n = end < 0 ? code.size() - i : end + 3 - i;
            out += spanToken(QStringLiteral("comment"), code.mid(i, n));
            i += n;
            continue;
        }
        if (code.at(i) == '<') {
            const int end = code.indexOf('>', i + 1);
            const int n = end < 0 ? 1 : end + 1 - i;
            out += spanToken(QStringLiteral("tag"), code.mid(i, n));
            i += n;
            continue;
        }
        out += localEscapeHtml(QString(code.at(i)));
        ++i;
    }
    return out;
}

QString highlightJsonCode(const QString& code)
{
    QString out;
    int i = 0;
    while (i < code.size()) {
        const QChar ch = code.at(i);
        if (ch == '"') {
            int j = i + 1;
            bool escaped = false;
            while (j < code.size()) {
                const QChar c = code.at(j++);
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == '"') {
                    break;
                }
            }
            int k = j;
            while (k < code.size() && code.at(k).isSpace() && code.at(k) != '\n') ++k;
            out += spanToken(k < code.size() && code.at(k) == ':' ? QStringLiteral("property") : QStringLiteral("string"), code.mid(i, j - i));
            i = j;
            continue;
        }
        if (ch.isDigit() || ch == '-') {
            int j = i + 1;
            while (j < code.size() && (code.at(j).isLetterOrNumber() || code.at(j) == '.' || code.at(j) == '+' || code.at(j) == '-')) ++j;
            out += spanToken(QStringLiteral("number"), code.mid(i, j - i));
            i = j;
            continue;
        }
        if (isIdentifierStart(ch)) {
            int j = i + 1;
            while (j < code.size() && isIdentifierPart(code.at(j))) ++j;
            const QString word = code.mid(i, j - i);
            out += (word == QStringLiteral("true") || word == QStringLiteral("false") || word == QStringLiteral("null"))
                ? spanToken(QStringLiteral("literal"), word)
                : localEscapeHtml(word);
            i = j;
            continue;
        }
        out += localEscapeHtml(QString(ch));
        ++i;
    }
    return out;
}

QString highlightGenericCode(const QString& code, const QString& language)
{
    static const QSet<QString> cppKeywords = {
        QStringLiteral("alignas"), QStringLiteral("alignof"), QStringLiteral("auto"), QStringLiteral("break"), QStringLiteral("case"),
        QStringLiteral("catch"), QStringLiteral("class"), QStringLiteral("const"), QStringLiteral("constexpr"), QStringLiteral("continue"),
        QStringLiteral("default"), QStringLiteral("delete"), QStringLiteral("do"), QStringLiteral("else"), QStringLiteral("enum"),
        QStringLiteral("explicit"), QStringLiteral("export"), QStringLiteral("extern"), QStringLiteral("for"), QStringLiteral("friend"),
        QStringLiteral("if"), QStringLiteral("inline"), QStringLiteral("namespace"), QStringLiteral("new"), QStringLiteral("noexcept"),
        QStringLiteral("operator"), QStringLiteral("private"), QStringLiteral("protected"), QStringLiteral("public"), QStringLiteral("return"),
        QStringLiteral("sizeof"), QStringLiteral("static"), QStringLiteral("struct"), QStringLiteral("switch"), QStringLiteral("template"),
        QStringLiteral("this"), QStringLiteral("throw"), QStringLiteral("try"), QStringLiteral("typedef"), QStringLiteral("typename"),
        QStringLiteral("using"), QStringLiteral("virtual"), QStringLiteral("volatile"), QStringLiteral("while")
    };
    static const QSet<QString> cppTypes = {
        QStringLiteral("bool"), QStringLiteral("char"), QStringLiteral("double"), QStringLiteral("float"), QStringLiteral("int"),
        QStringLiteral("long"), QStringLiteral("short"), QStringLiteral("signed"), QStringLiteral("unsigned"), QStringLiteral("void"),
        QStringLiteral("wchar_t"), QStringLiteral("size_t"), QStringLiteral("std"), QStringLiteral("string")
    };
    static const QSet<QString> jsKeywords = {
        QStringLiteral("async"), QStringLiteral("await"), QStringLiteral("break"), QStringLiteral("case"), QStringLiteral("catch"),
        QStringLiteral("class"), QStringLiteral("const"), QStringLiteral("continue"), QStringLiteral("debugger"), QStringLiteral("default"),
        QStringLiteral("delete"), QStringLiteral("do"), QStringLiteral("else"), QStringLiteral("export"), QStringLiteral("extends"),
        QStringLiteral("finally"), QStringLiteral("for"), QStringLiteral("from"), QStringLiteral("function"), QStringLiteral("if"),
        QStringLiteral("import"), QStringLiteral("in"), QStringLiteral("instanceof"), QStringLiteral("let"), QStringLiteral("new"),
        QStringLiteral("of"), QStringLiteral("return"), QStringLiteral("static"), QStringLiteral("super"), QStringLiteral("switch"),
        QStringLiteral("this"), QStringLiteral("throw"), QStringLiteral("try"), QStringLiteral("typeof"), QStringLiteral("var"),
        QStringLiteral("void"), QStringLiteral("while"), QStringLiteral("yield")
    };
    static const QSet<QString> pyKeywords = {
        QStringLiteral("and"), QStringLiteral("as"), QStringLiteral("assert"), QStringLiteral("async"), QStringLiteral("await"),
        QStringLiteral("break"), QStringLiteral("class"), QStringLiteral("continue"), QStringLiteral("def"), QStringLiteral("del"),
        QStringLiteral("elif"), QStringLiteral("else"), QStringLiteral("except"), QStringLiteral("finally"), QStringLiteral("for"),
        QStringLiteral("from"), QStringLiteral("global"), QStringLiteral("if"), QStringLiteral("import"), QStringLiteral("in"),
        QStringLiteral("is"), QStringLiteral("lambda"), QStringLiteral("nonlocal"), QStringLiteral("not"), QStringLiteral("or"),
        QStringLiteral("pass"), QStringLiteral("raise"), QStringLiteral("return"), QStringLiteral("try"), QStringLiteral("while"),
        QStringLiteral("with"), QStringLiteral("yield")
    };
    static const QSet<QString> shellKeywords = {
        QStringLiteral("case"), QStringLiteral("do"), QStringLiteral("done"), QStringLiteral("elif"), QStringLiteral("else"),
        QStringLiteral("esac"), QStringLiteral("fi"), QStringLiteral("for"), QStringLiteral("function"), QStringLiteral("if"),
        QStringLiteral("in"), QStringLiteral("select"), QStringLiteral("then"), QStringLiteral("until"), QStringLiteral("while")
    };
    static const QSet<QString> literals = {
        QStringLiteral("False"), QStringLiteral("None"), QStringLiteral("True"), QStringLiteral("false"), QStringLiteral("null"),
        QStringLiteral("nullptr"), QStringLiteral("true"), QStringLiteral("undefined")
    };
    static const QSet<QString> builtins = {
        QStringLiteral("Array"), QStringLiteral("Console"), QStringLiteral("Math"), QStringLiteral("Object"), QStringLiteral("Promise"),
        QStringLiteral("String"), QStringLiteral("bool"), QStringLiteral("cd"), QStringLiteral("console"), QStringLiteral("echo"),
        QStringLiteral("exit"), QStringLiteral("grep"), QStringLiteral("len"), QStringLiteral("list"), QStringLiteral("map"),
        QStringLiteral("print"), QStringLiteral("printf"), QStringLiteral("set"), QStringLiteral("str"), QStringLiteral("vector")
    };

    const bool isCpp = language == QStringLiteral("cpp") || language == QStringLiteral("c") || language == QStringLiteral("java") || language == QStringLiteral("csharp");
    const bool isJs = language == QStringLiteral("javascript") || language == QStringLiteral("typescript");
    const bool isPy = language == QStringLiteral("python");
    const bool isShell = language == QStringLiteral("bash") || language == QStringLiteral("powershell");
    const bool lineHashComment = isPy || isShell;
    const bool slashComments = isCpp || isJs;
    const bool blockComments = isCpp || isJs;
    const bool backtickStrings = isJs || isShell;

    QString out;
    int i = 0;
    while (i < code.size()) {
        const QChar ch = code.at(i);

        if ((i == 0 || code.at(i - 1) == '\n') && isCpp && ch == '#') {
            int j = i;
            while (j < code.size() && code.at(j) != '\n') ++j;
            out += spanToken(QStringLiteral("preproc"), code.mid(i, j - i));
            i = j;
            continue;
        }
        if (lineHashComment && ch == '#') {
            int j = i;
            while (j < code.size() && code.at(j) != '\n') ++j;
            out += spanToken(QStringLiteral("comment"), code.mid(i, j - i));
            i = j;
            continue;
        }
        if (slashComments && code.mid(i, 2) == QStringLiteral("//")) {
            int j = i;
            while (j < code.size() && code.at(j) != '\n') ++j;
            out += spanToken(QStringLiteral("comment"), code.mid(i, j - i));
            i = j;
            continue;
        }
        if (blockComments && code.mid(i, 2) == QStringLiteral("/*")) {
            const int end = code.indexOf(QStringLiteral("*/"), i + 2);
            const int n = end < 0 ? code.size() - i : end + 2 - i;
            out += spanToken(QStringLiteral("comment"), code.mid(i, n));
            i += n;
            continue;
        }
        if (ch == '"' || ch == '\'' || (backtickStrings && ch == '`')) {
            const QChar quote = ch;
            int j = i + 1;
            bool escaped = false;
            while (j < code.size()) {
                const QChar c = code.at(j++);
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == quote) {
                    break;
                } else if (c == '\n' && quote != '`') {
                    break;
                }
            }
            out += spanToken(QStringLiteral("string"), code.mid(i, j - i));
            i = j;
            continue;
        }
        if (ch.isDigit()) {
            int j = i + 1;
            while (j < code.size() && (code.at(j).isLetterOrNumber() || code.at(j) == '.' || code.at(j) == '_' || code.at(j) == 'x' || code.at(j) == 'X')) ++j;
            out += spanToken(QStringLiteral("number"), code.mid(i, j - i));
            i = j;
            continue;
        }
        if (isShell && ch == '$') {
            int j = i + 1;
            if (j < code.size() && code.at(j) == '{') {
                ++j;
                while (j < code.size() && code.at(j) != '}') ++j;
                if (j < code.size()) ++j;
            } else {
                while (j < code.size() && isIdentifierPart(code.at(j))) ++j;
            }
            out += spanToken(QStringLiteral("variable"), code.mid(i, j - i));
            i = j;
            continue;
        }
        if (isIdentifierStart(ch)) {
            int j = i + 1;
            while (j < code.size() && isIdentifierPart(code.at(j))) ++j;
            const QString word = code.mid(i, j - i);
            QString kind;
            if ((isCpp && cppKeywords.contains(word)) || (isJs && jsKeywords.contains(word)) || (isPy && pyKeywords.contains(word)) || (isShell && shellKeywords.contains(word))) {
                kind = QStringLiteral("keyword");
            } else if (cppTypes.contains(word)) {
                kind = QStringLiteral("type");
            } else if (literals.contains(word)) {
                kind = QStringLiteral("literal");
            } else if (builtins.contains(word)) {
                kind = QStringLiteral("builtin");
            }
            out += kind.isEmpty() ? localEscapeHtml(word) : spanToken(kind, word);
            i = j;
            continue;
        }
        out += localEscapeHtml(QString(ch));
        ++i;
    }
    return out;
}

QString highlightCode(const QString& code, const QString& rawLanguage)
{
    const QString language = normalizedCodeLanguage(rawLanguage);
    if (language == QStringLiteral("html") || language == QStringLiteral("xml")) {
        return highlightMarkupCode(code);
    }
    if (language == QStringLiteral("json")) {
        return highlightJsonCode(code);
    }
    if (language == QStringLiteral("cpp") || language == QStringLiteral("c") || language == QStringLiteral("java") || language == QStringLiteral("csharp")
        || language == QStringLiteral("javascript") || language == QStringLiteral("typescript") || language == QStringLiteral("python")
        || language == QStringLiteral("bash") || language == QStringLiteral("powershell")) {
        return highlightGenericCode(code, language);
    }
    return localEscapeHtml(code);
}

QStringList splitDisplayMathRows(const QString& source)
{
    QStringList rows;
    QString current;
    for (int i = 0; i < source.size();) {
        if (source.at(i) == '\n') {
            rows << current.trimmed();
            current.clear();
            ++i;
            continue;
        }
        if (source.at(i) == '\\') {
            if (source.mid(i, 8) == QStringLiteral("\\newline")) {
                rows << current.trimmed();
                current.clear();
                i += 8;
                continue;
            }
            if (i + 1 < source.size() && source.at(i + 1) == '\\') {
                rows << current.trimmed();
                current.clear();
                i += 2;
                continue;
            }
        }
        current += source.at(i);
        ++i;
    }
    rows << current.trimmed();
    rows.removeAll(QString());
    return rows;
}

QString normalizeDisplayMath(QString source)
{
    source = source.trimmed();
    source.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    source.replace('\r', '\n');

    static const QRegularExpression environmentRe(QStringLiteral("\\\\begin\\s*\\{"));
    if (environmentRe.match(source).hasMatch()) {
        return source;
    }

    const QStringList rows = splitDisplayMathRows(source);
    if (rows.size() <= 1) {
        return source;
    }

    const bool hasAlignment = std::any_of(rows.cbegin(), rows.cend(), [](const QString& row) {
        return row.contains('&');
    });
    const QString environment = hasAlignment ? QStringLiteral("aligned") : QStringLiteral("gathered");
    return QStringLiteral("\\begin{%1}%2\\end{%1}").arg(environment, rows.join(QStringLiteral("\\\\")));
}

QString mathJaxToken(const QString& source, bool block)
{
    const QString tex = localEscapeHtml(block ? normalizeDisplayMath(source) : source.trimmed());
    if (block) {
        return QStringLiteral("<div class=\"math math-block\">\\[%1\\]</div>").arg(tex);
    }
    return QStringLiteral("<span class=\"math math-inline\">\\(%1\\)</span>").arg(tex);
}

QString rawMathBlock(const QString& source)
{
    return QStringLiteral("<pre class=\"math math-raw\">$$\n%1\n$$</pre>").arg(localEscapeHtml(source.trimmed()));
}

QString mathBlockToken(const QString& source, MathRenderMode mathMode)
{
    return mathMode == MathRenderMode::MathJax ? mathJaxToken(source, true) : rawMathBlock(source);
}

QString extractMathTokens(const QString& text, const QRegularExpression& expression, QStringList& tokens)
{
    QString out;
    int last = 0;
    const auto matches = expression.globalMatch(text);
    QRegularExpressionMatchIterator it = matches;
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        out += text.mid(last, match.capturedStart() - last);
        const QString token = QStringLiteral("\uE000MATH%1\uE001").arg(tokens.size());
        tokens << mathJaxToken(match.captured(1), false);
        out += token;
        last = match.capturedEnd();
    }
    out += text.mid(last);
    return out;
}

QString restoreMathTokens(QString text, const QStringList& tokens)
{
    for (int i = 0; i < tokens.size(); ++i) {
        text.replace(QStringLiteral("\uE000MATH%1\uE001").arg(i), tokens.at(i));
    }
    return text;
}
}

QString MarkdownService::escapeHtml(const QString& text)
{
    QString out = text;
    out.replace('&', QStringLiteral("&amp;"));
    out.replace('<', QStringLiteral("&lt;"));
    out.replace('>', QStringLiteral("&gt;"));
    out.replace('"', QStringLiteral("&quot;"));
    return out;
}

QString MarkdownService::inlineMarkup(QString text, MathRenderMode mathMode)
{
    QStringList mathTokens;
    if (mathMode == MathRenderMode::MathJax) {
        text = extractMathTokens(text, QRegularExpression(QStringLiteral("\\$\\$([\\s\\S]+?)\\$\\$")), mathTokens);
        text = extractMathTokens(text, QRegularExpression(QStringLiteral("\\$([^$\\n]+?)\\$")), mathTokens);
    }
    text = escapeHtml(text);
    text.replace(QRegularExpression(QStringLiteral("`([^`]+)`")), QStringLiteral("<code>\\1</code>"));
    text.replace(QRegularExpression(QStringLiteral("!\\[([^\\]]*)\\]\\(([^\\)]+)\\)")), QStringLiteral("<img alt=\"\\1\" src=\"\\2\">"));
    text.replace(QRegularExpression(QStringLiteral("\\[([^\\]]+)\\]\\(([^\\)]+)\\)")), QStringLiteral("<a href=\"\\2\">\\1</a>"));
    text.replace(QRegularExpression(QStringLiteral("\\*\\*([^*]+)\\*\\*")), QStringLiteral("<strong>\\1</strong>"));
    text.replace(QRegularExpression(QStringLiteral("~~([^~]+)~~")), QStringLiteral("<del>\\1</del>"));
    text.replace(QRegularExpression(QStringLiteral("(^|\\s)\\*([^*]+)\\*")), QStringLiteral("\\1<em>\\2</em>"));
    return restoreMathTokens(text, mathTokens);
}

QString MarkdownService::markdownToHtml(const QString& markdown, MathRenderMode mathMode)
{
    QString html;
    const QStringList lines = markdown.split('\n');
    bool inCode = false;
    QString codeLanguage;
    QStringList codeLines;
    QStringList mathLines;
    QStringList paragraph;
    bool inUl = false;
    bool inOl = false;
    bool inBlockquote = false;
    bool inMathBlock = false;

    auto closeParagraph = [&]() {
        if (!paragraph.isEmpty()) {
            html += QStringLiteral("<p>") + inlineMarkup(paragraph.join(' '), mathMode) + QStringLiteral("</p>\n");
            paragraph.clear();
        }
    };
    auto closeLists = [&]() {
        if (inUl) {
            html += QStringLiteral("</ul>\n");
            inUl = false;
        }
        if (inOl) {
            html += QStringLiteral("</ol>\n");
            inOl = false;
        }
        if (inBlockquote) {
            html += QStringLiteral("</blockquote>\n");
            inBlockquote = false;
        }
    };

    for (int i = 0; i < lines.size(); ++i) {
        const QString rawLine = lines.at(i);
        const QString line = rawLine.trimmed();

        if (inMathBlock) {
            if (line.endsWith(QStringLiteral("$$"))) {
                const int closeIndex = rawLine.lastIndexOf(QStringLiteral("$$"));
                const QString beforeClose = closeIndex < 0 ? QString() : rawLine.left(closeIndex);
                if (!beforeClose.trimmed().isEmpty()) {
                    mathLines << beforeClose;
                }
                html += mathBlockToken(mathLines.join('\n'), mathMode) + QStringLiteral("\n");
                mathLines.clear();
                inMathBlock = false;
            } else {
                mathLines << rawLine;
            }
            continue;
        }

        if (line.startsWith(QStringLiteral("```"))) {
            closeParagraph();
            closeLists();
            if (!inCode) {
                inCode = true;
                codeLanguage = line.mid(3).trimmed();
                codeLines.clear();
                if (normalizedCodeLanguage(codeLanguage).compare(QStringLiteral("mermaid"), Qt::CaseInsensitive) == 0) {
                    html += QStringLiteral("<div class=\"mermaid\">");
                } else {
                    const QString language = normalizedCodeLanguage(codeLanguage);
                    html += QStringLiteral("<figure class=\"code-block\"><figcaption><span>")
                        + escapeHtml(language.isEmpty() ? QStringLiteral("text") : language)
                        + QStringLiteral("</span><button class=\"copy-code\" type=\"button\">Copy</button></figcaption><pre><code class=\"language-")
                        + escapeHtml(language)
                        + QStringLiteral("\">");
                }
            } else {
                if (normalizedCodeLanguage(codeLanguage).compare(QStringLiteral("mermaid"), Qt::CaseInsensitive) == 0) {
                    html += escapeHtml(codeLines.join('\n'));
                    html += QStringLiteral("</div>\n");
                } else {
                    html += highlightCode(codeLines.join('\n'), codeLanguage);
                    html += QStringLiteral("</code></pre></figure>\n");
                }
                inCode = false;
                codeLanguage.clear();
                codeLines.clear();
            }
            continue;
        }

        if (inCode) {
            codeLines << rawLine;
            continue;
        }

        if (line.startsWith(QStringLiteral("$$"))) {
            closeParagraph();
            closeLists();

            QString mathStart = line.mid(2).trimmed();
            if (mathStart.endsWith(QStringLiteral("$$")) && mathStart.size() >= 2) {
                mathStart.chop(2);
                html += mathBlockToken(mathStart, mathMode) + QStringLiteral("\n");
            } else {
                if (!mathStart.isEmpty()) {
                    mathLines << mathStart;
                }
                inMathBlock = true;
            }
            continue;
        }

        if (line.isEmpty()) {
            closeParagraph();
            closeLists();
            continue;
        }

        if (i + 1 < lines.size() && rawLine.contains('|') && isTableSeparator(lines.at(i + 1))) {
            closeParagraph();
            closeLists();
            const QStringList headers = splitTableRow(rawLine);
            html += QStringLiteral("<div class=\"table-wrap\"><table><thead><tr>");
            for (const QString& header : headers) {
                html += QStringLiteral("<th>") + inlineMarkup(header, mathMode) + QStringLiteral("</th>");
            }
            html += QStringLiteral("</tr></thead><tbody>");
            i += 2;
            while (i < lines.size() && lines.at(i).contains('|') && !lines.at(i).trimmed().isEmpty()) {
                html += QStringLiteral("<tr>");
                for (const QString& cell : splitTableRow(lines.at(i))) {
                    html += QStringLiteral("<td>") + inlineMarkup(cell, mathMode) + QStringLiteral("</td>");
                }
                html += QStringLiteral("</tr>");
                ++i;
            }
            --i;
            html += QStringLiteral("</tbody></table></div>\n");
            continue;
        }

        static const QRegularExpression headingRe(QStringLiteral("^(#{1,6})\\s+(.+)$"));
        const QRegularExpressionMatch headingMatch = headingRe.match(line);
        if (headingMatch.hasMatch()) {
            closeParagraph();
            closeLists();
            const int level = headingMatch.captured(1).size();
            const QString text = headingMatch.captured(2).trimmed();
            html += QStringLiteral("<h%1 data-source-line=\"%2\">%3</h%1>\n")
                .arg(level)
                .arg(i + 1)
                .arg(inlineMarkup(text, mathMode));
            continue;
        }

        if (line == QStringLiteral("---") || line == QStringLiteral("***")) {
            closeParagraph();
            closeLists();
            html += QStringLiteral("<hr>\n");
            continue;
        }

        static const QRegularExpression taskRe(QStringLiteral("^[-*+]\\s+\\[([ xX])\\]\\s+(.+)$"));
        const QRegularExpressionMatch taskMatch = taskRe.match(line);
        if (taskMatch.hasMatch()) {
            closeParagraph();
            if (!inUl) {
                closeLists();
                html += QStringLiteral("<ul class=\"task-list\">\n");
                inUl = true;
            }
            const QString checked = taskMatch.captured(1).trimmed().isEmpty() ? QString() : QStringLiteral(" checked");
            html += QStringLiteral("<li><input type=\"checkbox\" disabled%1> %2</li>\n").arg(checked, inlineMarkup(taskMatch.captured(2), mathMode));
            continue;
        }

        static const QRegularExpression ulRe(QStringLiteral("^[-*+]\\s+(.+)$"));
        const QRegularExpressionMatch ulMatch = ulRe.match(line);
        if (ulMatch.hasMatch()) {
            closeParagraph();
            if (!inUl) {
                closeLists();
                html += QStringLiteral("<ul>\n");
                inUl = true;
            }
            html += QStringLiteral("<li>") + inlineMarkup(ulMatch.captured(1), mathMode) + QStringLiteral("</li>\n");
            continue;
        }

        static const QRegularExpression olRe(QStringLiteral("^\\d+[.)]\\s+(.+)$"));
        const QRegularExpressionMatch olMatch = olRe.match(line);
        if (olMatch.hasMatch()) {
            closeParagraph();
            if (!inOl) {
                closeLists();
                html += QStringLiteral("<ol>\n");
                inOl = true;
            }
            html += QStringLiteral("<li>") + inlineMarkup(olMatch.captured(1), mathMode) + QStringLiteral("</li>\n");
            continue;
        }

        if (line.startsWith('>')) {
            closeParagraph();
            if (!inBlockquote) {
                closeLists();
                html += QStringLiteral("<blockquote>\n");
                inBlockquote = true;
            }
            html += QStringLiteral("<p>") + inlineMarkup(line.mid(1).trimmed(), mathMode) + QStringLiteral("</p>\n");
            continue;
        }

        closeLists();
        paragraph << rawLine.trimmed();
    }

    closeParagraph();
    closeLists();
    if (inCode) {
        if (normalizedCodeLanguage(codeLanguage).compare(QStringLiteral("mermaid"), Qt::CaseInsensitive) == 0) {
            html += escapeHtml(codeLines.join('\n')) + QStringLiteral("</div>\n");
            return html;
        }
        html += highlightCode(codeLines.join('\n'), codeLanguage);
        html += QStringLiteral("</code></pre></figure>\n");
    }
    if (inMathBlock) {
        html += mathBlockToken(mathLines.join('\n'), mathMode) + QStringLiteral("\n");
    }
    return html;
}

QString MarkdownService::completeHtmlDocument(const QString& markdown, const HtmlExportOptions& options)
{
    const QString css = readResource(options.darkTheme ? QStringLiteral(":/themes/dark.qss") : QStringLiteral(":/themes/light.qss"))
        + readResource(QStringLiteral(":/preview/preview.css"))
        + options.customCss;
    const QString body = markdownToHtml(markdown);
    const QString title = escapeHtml(options.title.isEmpty() ? QStringLiteral("SlateMark Export") : options.title);
    QString authorMeta;
    if (!options.author.isEmpty()) {
        authorMeta = QStringLiteral("<meta name=\"author\" content=\"%1\">\n").arg(escapeHtml(options.author));
    }

    return QStringLiteral("<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n"
                          "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n%1<title>%2</title>\n"
                          "<style>%3</style>\n</head>\n<body class=\"preview-document\"><main class=\"markdown-body\">\n%4\n"
                          "</main><script>%5</script></body>\n</html>\n")
        .arg(authorMeta, title, css, body, readResource(QStringLiteral(":/preview/preview.js")));
}

QVector<MarkdownHeading> MarkdownService::outline(const QString& markdown)
{
    QVector<MarkdownHeading> headings;
    static const QRegularExpression headingRe(QStringLiteral("^(#{1,6})\\s+(.+)$"));
    const QStringList lines = markdown.split('\n');
    for (int i = 0; i < lines.size(); ++i) {
        const QRegularExpressionMatch match = headingRe.match(lines.at(i).trimmed());
        if (match.hasMatch()) {
            headings.push_back({static_cast<int>(match.captured(1).size()), i + 1, match.captured(2).trimmed()});
        }
    }
    return headings;
}

DocumentStatistics MarkdownService::statistics(const QString& markdown)
{
    DocumentStatistics stats;
    stats.characters = markdown.size();
    stats.charactersNoSpaces = markdown.size() - markdown.count(QRegularExpression(QStringLiteral("\\s")));
    stats.lines = markdown.isEmpty() ? 1 : markdown.count('\n') + 1;
    stats.words = markdown.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size();
    stats.paragraphs = markdown.split(QRegularExpression(QStringLiteral("\\n\\s*\\n")), Qt::SkipEmptyParts).size();
    stats.headings = outline(markdown).size();
    stats.images = markdown.count(QRegularExpression(QStringLiteral("!\\[[^\\]]*\\]\\([^\\)]+\\)")));
    stats.links = markdown.count(QRegularExpression(QStringLiteral("(?<!!)\\[[^\\]]+\\]\\([^\\)]+\\)")));
    stats.readingMinutes = qMax(1, (stats.words + 199) / 200);
    return stats;
}

QString MarkdownService::wrapSelection(const QString& selectedText, const QString& before, const QString& after, const QString& placeholder)
{
    return before + (selectedText.isEmpty() ? placeholder : selectedText) + after;
}

QString MarkdownService::headingPrefix(int level)
{
    if (level <= 0) {
        return {};
    }
    return QString(level, '#') + ' ';
}

QString MarkdownService::tableTemplate(int rows, int columns)
{
    rows = qBound(1, rows, 20);
    columns = qBound(1, columns, 10);
    QString header;
    QString separator;
    for (int c = 0; c < columns; ++c) {
        header += QStringLiteral("| Header %1 ").arg(c + 1);
        separator += QStringLiteral("| --- ");
    }
    QString table = header + QStringLiteral("|\n") + separator + QStringLiteral("|\n");
    for (int r = 0; r < rows; ++r) {
        QString row;
        for (int c = 0; c < columns; ++c) {
            row += QStringLiteral("| Cell ");
        }
        table += row + QStringLiteral("|\n");
    }
    return table;
}
