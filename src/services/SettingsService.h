#pragma once

#include <QFont>
#include <QObject>
#include <QSettings>

class SettingsService : public QObject
{
    Q_OBJECT

public:
    explicit SettingsService(QObject* parent = nullptr);

    QString theme() const;
    void setTheme(const QString& theme);
    QString previewEngine() const;
    void setPreviewEngine(const QString& engine);
    QFont editorFont() const;
    void setEditorFont(const QFont& font);
    int tabWidth() const;
    void setTabWidth(int width);
    bool wordWrap() const;
    void setWordWrap(bool enabled);
    bool scrollSync() const;
    void setScrollSync(bool enabled);
    bool autoSave() const;
    void setAutoSave(bool enabled);
    int autoSaveIntervalSeconds() const;
    void setAutoSaveIntervalSeconds(int seconds);
    QString defaultSaveDir() const;
    void setDefaultSaveDir(const QString& path);
    bool restoreSession() const;
    void setRestoreSession(bool enabled);

signals:
    void changed();

private:
    QSettings m_settings;
};
