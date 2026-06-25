#include "services/ExportService.h"
#include "services/FileService.h"
#include "services/MarkdownService.h"
#include "services/RecentFilesService.h"
#include "services/RecoveryService.h"
#include "services/SettingsService.h"

#include <QTemporaryDir>
#include <QtTest>

class CoreTests : public QObject
{
    Q_OBJECT

private slots:
    void markdownToHtml();
    void fileOpenSave();
    void exportHtml();
    void recentFiles();
    void settingsReadWrite();
    void recovery();
    void formatInsertion();
    void outlineParse();
    void statistics();
};

void CoreTests::markdownToHtml()
{
    const QString html = MarkdownService::markdownToHtml(QStringLiteral("# Title\n\n- [x] Task\n\n```cpp\nint main(){}\n```"));
    QVERIFY(html.contains(QStringLiteral("<h1")));
    QVERIFY(html.contains(QStringLiteral("task-list")));
    QVERIFY(html.contains(QStringLiteral("language-cpp")));
    QVERIFY(html.contains(QStringLiteral("tok-type\">int")));
    QVERIFY(MarkdownService::markdownToHtml(QStringLiteral("```python\ndef main():\n    print(\"ok\")\n```")).contains(QStringLiteral("tok-keyword\">def")));
    QVERIFY(MarkdownService::markdownToHtml(QStringLiteral("```bash\nif [ -f a ]; then echo $HOME; fi\n```")).contains(QStringLiteral("tok-variable\">$HOME")));
    QVERIFY(MarkdownService::markdownToHtml(QStringLiteral("```js\nconst x = true\n```")).contains(QStringLiteral("language-javascript")));
    QVERIFY(MarkdownService::markdownToHtml(QStringLiteral("```js\nconst x = true\n```")).contains(QStringLiteral("tok-keyword\">const")));
    const QString mathSource = QStringLiteral("Inline $a^2 + b_1$.\n\n$$\n\\frac{1}{2} + \\sqrt{x} + \\int_0^1 x^2 dx\n$$");
    const QString rawMathHtml = MarkdownService::markdownToHtml(mathSource);
    QVERIFY(rawMathHtml.contains(QStringLiteral("$a^2 + b_1$")));
    QVERIFY(rawMathHtml.contains(QStringLiteral("math-raw")));
    const QString mathJaxHtml = MarkdownService::markdownToHtml(mathSource, MathRenderMode::MathJax);
    QVERIFY(mathJaxHtml.contains(QStringLiteral("math-inline")));
    QVERIFY(mathJaxHtml.contains(QStringLiteral("\\(a^2 + b_1\\)")));
    QVERIFY(mathJaxHtml.contains(QStringLiteral("math-block")));
    QVERIFY(mathJaxHtml.contains(QStringLiteral("\\[\\frac{1}{2} + \\sqrt{x} + \\int_0^1 x^2 dx\\]")));
    const QString gatheredMath = MarkdownService::markdownToHtml(QStringLiteral("$$\na \\newline b \\\\ c\n$$"), MathRenderMode::MathJax);
    QVERIFY(gatheredMath.contains(QStringLiteral("\\begin{gathered}a\\\\b\\\\c\\end{gathered}")));
    const QString alignedMath = MarkdownService::markdownToHtml(QStringLiteral("$$\na &= b\nc &= d\n$$"), MathRenderMode::MathJax);
    QVERIFY(alignedMath.contains(QStringLiteral("\\begin{aligned}a &amp;= b\\\\c &amp;= d\\end{aligned}")));
    const QString environmentMath = MarkdownService::markdownToHtml(QStringLiteral("$$\n\\begin{cases}a \\\\ b\\end{cases}\n$$"), MathRenderMode::MathJax);
    QVERIFY(environmentMath.contains(QStringLiteral("\\begin{cases}a \\\\ b\\end{cases}")));
}

void CoreTests::fileOpenSave()
{
    QTemporaryDir dir;
    const QString path = dir.path() + QStringLiteral("/a.md");
    QVERIFY(FileService::writeTextFile(path, QStringLiteral("# A")).ok);
    const FileResult result = FileService::readTextFile(path);
    QVERIFY(result.ok);
    QCOMPARE(result.text, QStringLiteral("# A"));
}

void CoreTests::exportHtml()
{
    QTemporaryDir dir;
    const QString path = dir.path() + QStringLiteral("/out.html");
    HtmlExportOptions options;
    options.title = QStringLiteral("Doc");
    QVERIFY(ExportService::exportHtml(path, QStringLiteral("# Doc"), options).ok);
    QVERIFY(FileService::readTextFile(path).text.contains(QStringLiteral("<!DOCTYPE html>")));
}

void CoreTests::recentFiles()
{
    RecentFilesService recent;
    recent.clear();
    recent.addFile(QStringLiteral("a.md"));
    recent.addFile(QStringLiteral("b.md"));
    QCOMPARE(recent.files().first(), QStringLiteral("b.md"));
}

void CoreTests::settingsReadWrite()
{
    SettingsService settings;
    settings.setTheme(QStringLiteral("light"));
    QCOMPARE(settings.theme(), QStringLiteral("light"));
}

void CoreTests::recovery()
{
    QTemporaryDir dir;
    qputenv("SLATEMARK_RECOVERY_DIR", dir.path().toLocal8Bit());
    QVERIFY(RecoveryService::saveRecovery(QStringLiteral("unit-test"), QStringLiteral("draft")));
    QCOMPARE(RecoveryService::loadRecovery(QStringLiteral("unit-test")), QStringLiteral("draft"));
    RecoveryService::removeRecovery(QStringLiteral("unit-test"));
    qunsetenv("SLATEMARK_RECOVERY_DIR");
}

void CoreTests::formatInsertion()
{
    QCOMPARE(MarkdownService::wrapSelection(QStringLiteral("x"), QStringLiteral("**"), QStringLiteral("**"), QStringLiteral("b")), QStringLiteral("**x**"));
    QCOMPARE(MarkdownService::tableTemplate(1, 2).count('|'), 9);
}

void CoreTests::outlineParse()
{
    const auto outline = MarkdownService::outline(QStringLiteral("# A\n\n## B"));
    QCOMPARE(outline.size(), 2);
    QCOMPARE(outline.at(1).level, 2);
}

void CoreTests::statistics()
{
    const DocumentStatistics stats = MarkdownService::statistics(QStringLiteral("# A\n\nOne two ![x](a.png) [b](c)"));
    QCOMPARE(stats.headings, 1);
    QCOMPARE(stats.images, 1);
    QCOMPARE(stats.links, 1);
    QVERIFY(stats.words >= 4);
}

QTEST_MAIN(CoreTests)
#include "test_core.moc"
