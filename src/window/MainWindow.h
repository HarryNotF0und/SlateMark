#pragma once

#include "commands/CommandRegistry.h"
#include "documents/Document.h"
#include "services/RecentFilesService.h"
#include "services/SettingsService.h"
#include "theme/ThemeManager.h"
#include "widgets/ViewModeSwitcher.h"

#include <QMainWindow>
#include <QTimer>
#include <QVector>

class DocumentTabBar;
class MarkdownEditor;
class NavigationRail;
class PreviewWidget;
class StatusBarWidget;
class ToolPanel;
class CommandPalette;
class QMenu;
class QSplitter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void newDocument();
    void openDocument();
    void openDocumentPath(const QString& path);
    bool saveCurrentDocument();
    bool saveCurrentDocumentAs();
    void exportCurrentHtml();
    void closeTab(int index);
    void switchTab(int index);
    void updateCurrentDocumentText();
    void updatePreviewAndStats();
    void insertTable();
    void openSettings();
    void applySettings();
    void setViewMode(ViewMode mode);
    void autoSaveRecovery();

private:
    void buildUi();
    void buildMenus();
    void registerCommands();
    void loadDocumentToEditor(int index);
    void refreshTabTitle(int index);
    void refreshRecentFilesMenu();
    bool maybeSave(int index);
    bool saveDocument(int index, bool saveAs);
    QString documentDisplayName(const Document& document) const;
    int currentIndex() const;
    Document& currentDocument();
    const Document& currentDocument() const;

    ThemeManager* m_theme = nullptr;
    SettingsService* m_settings = nullptr;
    RecentFilesService* m_recentFiles = nullptr;
    CommandRegistry* m_commands = nullptr;
    CommandPalette* m_palette = nullptr;
    NavigationRail* m_navigation = nullptr;
    ToolPanel* m_toolPanel = nullptr;
    MarkdownEditor* m_editor = nullptr;
    PreviewWidget* m_preview = nullptr;
    DocumentTabBar* m_tabs = nullptr;
    StatusBarWidget* m_status = nullptr;
    ViewModeSwitcher* m_viewModes = nullptr;
    QSplitter* m_splitter = nullptr;
    QMenu* m_recentMenu = nullptr;
    QTimer m_updateTimer;
    QTimer m_recoveryTimer;
    QVector<Document> m_documents;
    int m_loadedIndex = -1;
    bool m_loading = false;
    bool m_syncingScroll = false;
};
