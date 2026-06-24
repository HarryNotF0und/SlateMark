#include "window/MainWindow.h"

#include "dialogs/ExportDialog.h"
#include "dialogs/SettingsDialog.h"
#include "dialogs/TableDialog.h"
#include "editor/MarkdownEditor.h"
#include "preview/PreviewWidget.h"
#include "services/ExportService.h"
#include "services/FileService.h"
#include "services/ImageService.h"
#include "services/LoggingService.h"
#include "services/MarkdownService.h"
#include "services/RecoveryService.h"
#include "widgets/CommandPalette.h"
#include "widgets/DocumentTabBar.h"
#include "widgets/NavigationRail.h"
#include "widgets/StatusBarWidget.h"
#include "widgets/SvgIconButton.h"
#include "widgets/ToolPanel.h"
#include "widgets/ViewModeSwitcher.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollBar>
#include <QShortcut>
#include <QSplitter>
#include <QStyle>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
QString welcomeMarkdown()
{
    return QStringLiteral("# SlateMark\n\nA modern **Markdown editor** with live preview.\n\n## Features\n\n- Live preview\n- Syntax highlighting\n- Clean and focused\n\n> *Write in Markdown. See it beautifully.*\n");
}

ThemeMode modeFromString(const QString& value)
{
    if (value == QStringLiteral("light")) return ThemeMode::Light;
    if (value == QStringLiteral("system")) return ThemeMode::System;
    return ThemeMode::Dark;
}
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_theme(new ThemeManager(this))
    , m_settings(new SettingsService(this))
    , m_recentFiles(new RecentFilesService(this))
    , m_commands(new CommandRegistry(this))
{
    setWindowTitle(QStringLiteral("SlateMark"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/app-icon.png")));
    resize(1500, 860);
    setMinimumSize(960, 620);
    buildUi();
    buildMenus();
    registerCommands();
    applySettings();

    m_updateTimer.setSingleShot(true);
    m_updateTimer.setInterval(160);
    connect(&m_updateTimer, &QTimer::timeout, this, &MainWindow::updatePreviewAndStats);
    connect(&m_recoveryTimer, &QTimer::timeout, this, &MainWindow::autoSaveRecovery);
    m_recoveryTimer.start(qMax(5, m_settings->autoSaveIntervalSeconds()) * 1000);

    newDocument();
}

void MainWindow::buildUi()
{
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* body = new QWidget(central);
    auto* bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    m_navigation = new NavigationRail(m_theme, body);
    m_toolPanel = new ToolPanel(m_theme, body);
    bodyLayout->addWidget(m_navigation);
    bodyLayout->addWidget(m_toolPanel);

    auto* work = new QWidget(body);
    auto* workLayout = new QVBoxLayout(work);
    workLayout->setContentsMargins(10, 10, 10, 10);
    workLayout->setSpacing(8);

    auto* tabRow = new QWidget(work);
    auto* tabLayout = new QHBoxLayout(tabRow);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(8);
    m_tabs = new DocumentTabBar(tabRow);
    auto* addTab = new SvgIconButton(QStringLiteral("plus"), m_theme, tabRow);
    addTab->setToolTip(QStringLiteral("New file"));
    addTab->setFixedSize(44, 44);
    m_viewModes = new ViewModeSwitcher(m_theme, tabRow);
    tabLayout->addWidget(m_tabs, 0, Qt::AlignVCenter);
    tabLayout->addWidget(addTab, 0, Qt::AlignVCenter);
    tabLayout->addStretch();
    tabLayout->addWidget(m_viewModes, 0, Qt::AlignVCenter);
    workLayout->addWidget(tabRow);

    m_splitter = new QSplitter(Qt::Horizontal, work);
    m_editor = new MarkdownEditor(m_splitter);
    m_preview = new PreviewWidget(m_splitter);
    m_editor->setMinimumWidth(360);
    m_preview->setMinimumWidth(360);
    m_splitter->addWidget(m_editor);
    m_splitter->addWidget(m_preview);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setSizes({720, 720});
    workLayout->addWidget(m_splitter, 1);
    QTimer::singleShot(0, this, [this] {
        if (m_splitter && m_preview->isVisible()) {
            m_splitter->setSizes({720, 720});
        }
    });

    bodyLayout->addWidget(work, 1);
    root->addWidget(body, 1);
    m_status = new StatusBarWidget(central);
    root->addWidget(m_status);
    setCentralWidget(central);

    m_palette = new CommandPalette(m_commands, this);

    connect(addTab, &QToolButton::clicked, this, &MainWindow::newDocument);
    connect(m_navigation, &NavigationRail::toolPanelToggled, this, [this] { m_toolPanel->setVisible(!m_toolPanel->isVisible()); });
    connect(m_navigation, &NavigationRail::settingsRequested, this, &MainWindow::openSettings);
    connect(m_tabs, &QTabBar::currentChanged, this, &MainWindow::switchTab);
    connect(m_tabs, &QTabBar::tabCloseRequested, this, &MainWindow::closeTab);
    connect(m_tabs, &DocumentTabBar::middleCloseRequested, this, &MainWindow::closeTab);
    connect(m_tabs, &QTabBar::tabMoved, this, [this](int from, int to) { m_documents.move(from, to); m_loadedIndex = to; });
    connect(m_editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateCurrentDocumentText);
    connect(m_editor, &MarkdownEditor::cursorLineColumnChanged, m_status, &StatusBarWidget::setCursorInfo);
    connect(m_editor, &MarkdownEditor::fileDropped, this, &MainWindow::openDocumentPath);
    connect(m_editor, &MarkdownEditor::imageDropped, this, [this](const QString& path) { m_editor->insertBlockText(ImageService::markdownForImagePath(path)); });
    connect(m_toolPanel, &ToolPanel::formatRequested, m_editor, &MarkdownEditor::insertWrapped);
    connect(m_toolPanel, &ToolPanel::headingRequested, m_editor, &MarkdownEditor::setHeadingLevel);
    connect(m_toolPanel, &ToolPanel::blockRequested, m_editor, &MarkdownEditor::insertBlockText);
    connect(m_toolPanel, &ToolPanel::tableRequested, this, &MainWindow::insertTable);
    connect(m_toolPanel, &ToolPanel::outlineLineRequested, m_editor, &MarkdownEditor::gotoLine);
    connect(m_viewModes, &ViewModeSwitcher::modeChanged, this, &MainWindow::setViewMode);
    connect(m_preview, &PreviewWidget::sourceLineClicked, m_editor, &MarkdownEditor::gotoLine);
    connect(m_settings, &SettingsService::changed, this, &MainWindow::applySettings);
    connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        if (m_syncingScroll || !m_settings->scrollSync() || m_currentViewMode != ViewMode::Split) return;
        const int max = m_editor->verticalScrollBar()->maximum();
        m_syncingScroll = true;
        m_preview->scrollToRatio(max <= 0 ? 0.0 : static_cast<double>(value) / max);
        m_syncingScroll = false;
    });
    setViewMode(ViewMode::EditorOnly);
}

void MainWindow::buildMenus()
{
    QMenu* file = menuBar()->addMenu(QStringLiteral("&File"));
    file->addAction(QStringLiteral("&New"), this, &MainWindow::newDocument, QKeySequence::New);
    file->addAction(QStringLiteral("&Open"), this, &MainWindow::openDocument, QKeySequence::Open);
    file->addAction(QStringLiteral("&Save"), this, &MainWindow::saveCurrentDocument, QKeySequence::Save);
    file->addAction(QStringLiteral("Save &As"), this, &MainWindow::saveCurrentDocumentAs, QKeySequence::SaveAs);
    file->addAction(QStringLiteral("Export HTML"), this, &MainWindow::exportCurrentHtml);
    m_recentMenu = file->addMenu(QStringLiteral("Recent Files"));
    file->addSeparator();
    file->addAction(QStringLiteral("E&xit"), this, &QWidget::close);

    QMenu* edit = menuBar()->addMenu(QStringLiteral("&Edit"));
    edit->addAction(QStringLiteral("Undo"), m_editor, &QPlainTextEdit::undo, QKeySequence::Undo);
    edit->addAction(QStringLiteral("Redo"), m_editor, &QPlainTextEdit::redo, QKeySequence::Redo);
    edit->addSeparator();
    edit->addAction(QStringLiteral("Cut"), m_editor, &QPlainTextEdit::cut, QKeySequence::Cut);
    edit->addAction(QStringLiteral("Copy"), m_editor, &QPlainTextEdit::copy, QKeySequence::Copy);
    edit->addAction(QStringLiteral("Paste"), m_editor, &QPlainTextEdit::paste, QKeySequence::Paste);
    edit->addAction(QStringLiteral("Select All"), m_editor, &QPlainTextEdit::selectAll, QKeySequence::SelectAll);

    QMenu* view = menuBar()->addMenu(QStringLiteral("&View"));
    view->addAction(QStringLiteral("Editor Only"), this, [this] { setViewMode(ViewMode::EditorOnly); }, QKeySequence(QStringLiteral("Ctrl+Alt+E")));
    view->addAction(QStringLiteral("Split View"), this, [this] { setViewMode(ViewMode::Split); }, QKeySequence(QStringLiteral("Ctrl+Alt+S")));
    view->addAction(QStringLiteral("Preview Only"), this, [this] { setViewMode(ViewMode::PreviewOnly); }, QKeySequence(QStringLiteral("Ctrl+Alt+P")));
    view->addAction(QStringLiteral("Full Screen"), this, [this] { isFullScreen() ? showNormal() : showFullScreen(); }, QKeySequence(QStringLiteral("F11")));

    QMenu* insert = menuBar()->addMenu(QStringLiteral("&Insert"));
    insert->addAction(QStringLiteral("Link"), this, [this] { m_editor->insertWrapped(QStringLiteral("["), QStringLiteral("](https://example.com)"), QStringLiteral("link text")); }, QKeySequence(QStringLiteral("Ctrl+K")));
    insert->addAction(QStringLiteral("Code Block"), this, [this] { m_editor->insertBlockText(QStringLiteral("```cpp\n// code\n```")); }, QKeySequence(QStringLiteral("Ctrl+Shift+K")));
    insert->addAction(QStringLiteral("Table"), this, &MainWindow::insertTable);
    insert->addAction(QStringLiteral("Mermaid"), this, [this] { m_editor->insertBlockText(QStringLiteral("```mermaid\ngraph TD\n    A --> B\n```")); });

    QMenu* format = menuBar()->addMenu(QStringLiteral("F&ormat"));
    format->addAction(QStringLiteral("Bold"), this, [this] { m_editor->insertWrapped(QStringLiteral("**"), QStringLiteral("**"), QStringLiteral("bold text")); }, QKeySequence::Bold);
    format->addAction(QStringLiteral("Italic"), this, [this] { m_editor->insertWrapped(QStringLiteral("*"), QStringLiteral("*"), QStringLiteral("italic text")); }, QKeySequence::Italic);
    for (int i = 0; i <= 6; ++i) {
        format->addAction(i == 0 ? QStringLiteral("Normal Text") : QStringLiteral("Heading %1").arg(i),
            this, [this, i] { m_editor->setHeadingLevel(i); },
            QKeySequence(i == 0 ? QStringLiteral("Ctrl+0") : QStringLiteral("Ctrl+%1").arg(i)));
    }

    QMenu* tools = menuBar()->addMenu(QStringLiteral("&Tools"));
    tools->addAction(QStringLiteral("Command Palette"), this, [this] { m_palette->openPalette(); }, QKeySequence(QStringLiteral("Ctrl+Shift+P")));
    tools->addAction(QStringLiteral("Settings"), this, &MainWindow::openSettings);

    QMenu* help = menuBar()->addMenu(QStringLiteral("&Help"));
    help->addAction(QStringLiteral("About SlateMark"), this, [this] {
        QMessageBox::about(this, QStringLiteral("SlateMark"), QStringLiteral("SlateMark 0.1\nQt/C++ Markdown editor."));
    });
    refreshRecentFilesMenu();
}

void MainWindow::registerCommands()
{
    auto add = [this](const QString& id, const QString& title, const QString& shortcut, std::function<void()> fn) {
        m_commands->add({id, title, shortcut, std::move(fn)});
    };
    add(QStringLiteral("file.new"), QStringLiteral("New File"), QStringLiteral("Ctrl+N"), [this] { newDocument(); });
    add(QStringLiteral("file.open"), QStringLiteral("Open File"), QStringLiteral("Ctrl+O"), [this] { openDocument(); });
    add(QStringLiteral("file.save"), QStringLiteral("Save File"), QStringLiteral("Ctrl+S"), [this] { saveCurrentDocument(); });
    add(QStringLiteral("file.exportHtml"), QStringLiteral("Export HTML"), QString(), [this] { exportCurrentHtml(); });
    add(QStringLiteral("view.theme"), QStringLiteral("Toggle Theme"), QString(), [this] {
        m_settings->setTheme(m_theme->isDark() ? QStringLiteral("light") : QStringLiteral("dark"));
    });
    add(QStringLiteral("view.split"), QStringLiteral("Split View"), QStringLiteral("Ctrl+Alt+S"), [this] { setViewMode(ViewMode::Split); });
    add(QStringLiteral("insert.table"), QStringLiteral("Insert Table"), QString(), [this] { insertTable(); });
    add(QStringLiteral("insert.mermaid"), QStringLiteral("Insert Mermaid"), QString(), [this] { m_editor->insertBlockText(QStringLiteral("```mermaid\ngraph TD\n    A --> B\n```")); });
    add(QStringLiteral("app.settings"), QStringLiteral("Open Settings"), QString(), [this] { openSettings(); });
}

void MainWindow::newDocument()
{
    if (m_loadedIndex >= 0) {
        m_documents[m_loadedIndex].content = m_editor->toPlainText();
    }
    Document doc;
    doc.content = m_documents.isEmpty() ? welcomeMarkdown() : QString();
    doc.title = m_documents.isEmpty() ? QStringLiteral("Welcome.md") : QStringLiteral("Untitled.md");
    doc.untitled = true;
    doc.modified = false;
    m_documents.push_back(doc);
    const int index = m_documents.size() - 1;
    m_tabs->addTab(documentDisplayName(doc));
    m_tabs->setCurrentIndex(index);
    loadDocumentToEditor(index);
}

void MainWindow::openDocument()
{
    const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Open Markdown"), m_settings->defaultSaveDir(),
        QStringLiteral("Markdown (*.md *.markdown *.txt);;All files (*.*)"));
    if (!path.isEmpty()) {
        openDocumentPath(path);
    }
}

void MainWindow::openDocumentPath(const QString& path)
{
    const FileResult result = FileService::readTextFile(path);
    if (!result.ok) {
        QMessageBox::warning(this, QStringLiteral("Open failed"), result.error);
        return;
    }
    if (m_loadedIndex >= 0) {
        m_documents[m_loadedIndex].content = m_editor->toPlainText();
    }
    Document doc;
    doc.filePath = path;
    doc.title = QFileInfo(path).fileName();
    doc.content = result.text;
    doc.untitled = false;
    doc.modified = false;
    m_documents.push_back(doc);
    const int index = m_documents.size() - 1;
    m_tabs->addTab(documentDisplayName(doc));
    m_tabs->setCurrentIndex(index);
    m_recentFiles->addFile(path);
    refreshRecentFilesMenu();
    loadDocumentToEditor(index);
}

bool MainWindow::saveCurrentDocument()
{
    return saveDocument(currentIndex(), false);
}

bool MainWindow::saveCurrentDocumentAs()
{
    return saveDocument(currentIndex(), true);
}

void MainWindow::exportCurrentHtml()
{
    ExportDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export HTML"), m_settings->defaultSaveDir(), QStringLiteral("HTML (*.html)"));
    if (path.isEmpty()) {
        return;
    }
    const FileResult result = ExportService::exportHtml(path, m_editor->toPlainText(), dialog.options());
    if (!result.ok) {
        QMessageBox::warning(this, QStringLiteral("Export failed"), result.error);
    }
}

void MainWindow::closeTab(int index)
{
    if (index < 0 || index >= m_documents.size() || !maybeSave(index)) {
        return;
    }
    m_documents.removeAt(index);
    m_tabs->removeTab(index);
    if (m_documents.isEmpty()) {
        m_loadedIndex = -1;
        newDocument();
        return;
    }
    const int next = qMin(index, m_documents.size() - 1);
    m_tabs->setCurrentIndex(next);
    loadDocumentToEditor(next);
}

void MainWindow::switchTab(int index)
{
    if (index < 0 || index >= m_documents.size() || index == m_loadedIndex) {
        return;
    }
    if (m_loadedIndex >= 0 && m_loadedIndex < m_documents.size()) {
        m_documents[m_loadedIndex].content = m_editor->toPlainText();
    }
    loadDocumentToEditor(index);
}

void MainWindow::updateCurrentDocumentText()
{
    if (m_loading || m_loadedIndex < 0) {
        return;
    }
    Document& doc = currentDocument();
    doc.content = m_editor->toPlainText();
    doc.modified = true;
    refreshTabTitle(m_loadedIndex);
    m_status->setSaved(false);
    m_updateTimer.start();
}

void MainWindow::updatePreviewAndStats()
{
    const QString text = m_editor->toPlainText();
    if (m_currentViewMode != ViewMode::EditorOnly) {
        ensurePreviewReady();
        m_preview->setMarkdown(text);
    }
    const DocumentStatistics stats = MarkdownService::statistics(text);
    m_status->setStatistics(stats);
    m_toolPanel->setStatistics(stats);
    m_toolPanel->setOutline(MarkdownService::outline(text));
}

void MainWindow::insertTable()
{
    TableDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_editor->insertBlockText(MarkdownService::tableTemplate(dialog.rows(), dialog.columns()));
    }
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(m_settings, this);
    dialog.exec();
}

void MainWindow::applySettings()
{
    m_theme->setMode(modeFromString(m_settings->theme()));
    qApp->setStyleSheet(m_theme->styleSheet());
    m_editor->setEditorFont(m_settings->editorFont());
    m_editor->setDarkTheme(m_theme->isDark());
    m_editor->setTabWidthSpaces(m_settings->tabWidth());
    m_editor->setLineWrapMode(m_settings->wordWrap() ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    m_preview->setEngine(m_settings->previewEngine() == QStringLiteral("webengine") ? PreviewEngine::WebEngine : PreviewEngine::Lightweight);
    m_preview->setDarkTheme(m_theme->isDark());
    m_recoveryTimer.setInterval(qMax(5, m_settings->autoSaveIntervalSeconds()) * 1000);
    if (m_currentViewMode != ViewMode::EditorOnly) {
        updatePreviewAndStats();
    }
}

void MainWindow::setViewMode(ViewMode mode)
{
    m_currentViewMode = mode;
    m_viewModes->setMode(mode);
    m_editor->setVisible(mode != ViewMode::PreviewOnly);
    if (mode == ViewMode::EditorOnly) {
        m_preview->setVisible(false);
        m_preview->releaseResources();
        return;
    }

    ensurePreviewReady();
    m_preview->setVisible(true);
    updatePreviewAndStats();
}

void MainWindow::autoSaveRecovery()
{
    if (!m_settings->autoSave() || m_loadedIndex < 0) {
        return;
    }
    const Document& doc = currentDocument();
    RecoveryService::saveRecovery(doc.filePath.isEmpty() ? doc.title : doc.filePath, m_editor->toPlainText());
}

void MainWindow::loadDocumentToEditor(int index)
{
    m_loading = true;
    m_loadedIndex = index;
    m_editor->setPlainText(m_documents.at(index).content);
    m_loading = false;
    m_status->setSaved(!m_documents.at(index).modified);
    updatePreviewAndStats();
}

void MainWindow::refreshTabTitle(int index)
{
    if (index >= 0 && index < m_documents.size()) {
        m_tabs->setTabText(index, documentDisplayName(m_documents.at(index)));
    }
}

void MainWindow::refreshRecentFilesMenu()
{
    if (!m_recentMenu) {
        return;
    }
    m_recentMenu->clear();
    for (const QString& path : m_recentFiles->files()) {
        m_recentMenu->addAction(path, this, [this, path] { openDocumentPath(path); });
    }
    if (m_recentMenu->actions().isEmpty()) {
        m_recentMenu->addAction(QStringLiteral("(Empty)"))->setEnabled(false);
    }
}

void MainWindow::ensurePreviewReady()
{
    m_preview->setEngine(m_settings->previewEngine() == QStringLiteral("webengine") ? PreviewEngine::WebEngine : PreviewEngine::Lightweight);
    m_preview->setDarkTheme(m_theme->isDark());
}

bool MainWindow::maybeSave(int index)
{
    if (index < 0 || index >= m_documents.size() || !m_documents.at(index).modified) {
        return true;
    }
    const QMessageBox::StandardButton choice = QMessageBox::warning(this, QStringLiteral("Unsaved changes"),
        QStringLiteral("Save changes to %1?").arg(documentDisplayName(m_documents.at(index))),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (choice == QMessageBox::Cancel) {
        return false;
    }
    if (choice == QMessageBox::Save) {
        return saveDocument(index, false);
    }
    return true;
}

bool MainWindow::saveDocument(int index, bool saveAs)
{
    if (index < 0 || index >= m_documents.size()) {
        return false;
    }
    if (index == m_loadedIndex) {
        m_documents[index].content = m_editor->toPlainText();
    }
    Document& doc = m_documents[index];
    QString path = doc.filePath;
    if (saveAs || path.isEmpty()) {
        path = QFileDialog::getSaveFileName(this, QStringLiteral("Save Markdown"), m_settings->defaultSaveDir(), QStringLiteral("Markdown (*.md);;Markdown (*.markdown);;Text (*.txt)"));
        if (path.isEmpty()) {
            return false;
        }
        if (QFileInfo(path).suffix().isEmpty()) {
            path += QStringLiteral(".md");
        }
    }
    const FileResult result = FileService::writeTextFile(path, doc.content);
    if (!result.ok) {
        QMessageBox::warning(this, QStringLiteral("Save failed"), result.error);
        return false;
    }
    doc.filePath = path;
    doc.title = QFileInfo(path).fileName();
    doc.untitled = false;
    doc.modified = false;
    RecoveryService::removeRecovery(path);
    refreshTabTitle(index);
    if (index == m_loadedIndex) {
        m_status->setSaved(true);
    }
    m_recentFiles->addFile(path);
    refreshRecentFilesMenu();
    return true;
}

QString MainWindow::documentDisplayName(const Document& document) const
{
    return (document.modified ? QStringLiteral("*") : QString()) + document.title;
}

int MainWindow::currentIndex() const
{
    return m_loadedIndex;
}

Document& MainWindow::currentDocument()
{
    return m_documents[m_loadedIndex];
}

const Document& MainWindow::currentDocument() const
{
    return m_documents[m_loadedIndex];
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_loadedIndex >= 0) {
        m_documents[m_loadedIndex].content = m_editor->toPlainText();
    }
    for (int i = 0; i < m_documents.size(); ++i) {
        if (!maybeSave(i)) {
            event->ignore();
            return;
        }
    }
    event->accept();
}
