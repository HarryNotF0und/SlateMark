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
