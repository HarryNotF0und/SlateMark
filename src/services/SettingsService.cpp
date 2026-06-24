#include "services/SettingsService.h"

#include <QStandardPaths>

SettingsService::SettingsService(QObject* parent)
    : QObject(parent)
    , m_settings(QStringLiteral("SlateMark"), QStringLiteral("SlateMark"))
{
}

QString SettingsService::theme() const { return m_settings.value(QStringLiteral("theme"), QStringLiteral("dark")).toString(); }
void SettingsService::setTheme(const QString& theme) { m_settings.setValue(QStringLiteral("theme"), theme); emit changed(); }
QFont SettingsService::editorFont() const { return qvariant_cast<QFont>(m_settings.value(QStringLiteral("editorFont"), QFont(QStringLiteral("Consolas"), 12))); }
void SettingsService::setEditorFont(const QFont& font) { m_settings.setValue(QStringLiteral("editorFont"), font); emit changed(); }
int SettingsService::tabWidth() const { return m_settings.value(QStringLiteral("tabWidth"), 4).toInt(); }
void SettingsService::setTabWidth(int width) { m_settings.setValue(QStringLiteral("tabWidth"), qBound(2, width, 12)); emit changed(); }
bool SettingsService::wordWrap() const { return m_settings.value(QStringLiteral("wordWrap"), true).toBool(); }
void SettingsService::setWordWrap(bool enabled) { m_settings.setValue(QStringLiteral("wordWrap"), enabled); emit changed(); }
bool SettingsService::scrollSync() const { return m_settings.value(QStringLiteral("scrollSync"), true).toBool(); }
void SettingsService::setScrollSync(bool enabled) { m_settings.setValue(QStringLiteral("scrollSync"), enabled); emit changed(); }
bool SettingsService::autoSave() const { return m_settings.value(QStringLiteral("autoSave"), true).toBool(); }
void SettingsService::setAutoSave(bool enabled) { m_settings.setValue(QStringLiteral("autoSave"), enabled); emit changed(); }
int SettingsService::autoSaveIntervalSeconds() const { return m_settings.value(QStringLiteral("autoSaveIntervalSeconds"), 20).toInt(); }
void SettingsService::setAutoSaveIntervalSeconds(int seconds) { m_settings.setValue(QStringLiteral("autoSaveIntervalSeconds"), qBound(5, seconds, 600)); emit changed(); }
QString SettingsService::defaultSaveDir() const { return m_settings.value(QStringLiteral("defaultSaveDir"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(); }
void SettingsService::setDefaultSaveDir(const QString& path) { m_settings.setValue(QStringLiteral("defaultSaveDir"), path); emit changed(); }
bool SettingsService::restoreSession() const { return m_settings.value(QStringLiteral("restoreSession"), true).toBool(); }
void SettingsService::setRestoreSession(bool enabled) { m_settings.setValue(QStringLiteral("restoreSession"), enabled); emit changed(); }

