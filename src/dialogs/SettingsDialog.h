#pragma once

#include "services/SettingsService.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QSpinBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(SettingsService* settings, QWidget* parent = nullptr);

private:
    SettingsService* m_settings = nullptr;
    QComboBox* m_theme = nullptr;
    QComboBox* m_previewEngine = nullptr;
    QSpinBox* m_fontSize = nullptr;
    QSpinBox* m_tabWidth = nullptr;
    QCheckBox* m_wrap = nullptr;
    QCheckBox* m_scrollSync = nullptr;
    QCheckBox* m_autoSave = nullptr;
    QSpinBox* m_autoSaveInterval = nullptr;
};
