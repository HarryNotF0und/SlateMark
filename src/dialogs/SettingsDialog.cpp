#include "dialogs/SettingsDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>

SettingsDialog::SettingsDialog(SettingsService* settings, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(QStringLiteral("Settings"));
    auto* layout = new QFormLayout(this);
    m_theme = new QComboBox(this);
    m_theme->addItems({QStringLiteral("dark"), QStringLiteral("light"), QStringLiteral("system")});
    m_theme->setCurrentText(settings->theme());
    m_fontSize = new QSpinBox(this);
    m_fontSize->setRange(8, 28);
    m_fontSize->setValue(settings->editorFont().pointSize());
    m_tabWidth = new QSpinBox(this);
    m_tabWidth->setRange(2, 12);
    m_tabWidth->setValue(settings->tabWidth());
    m_wrap = new QCheckBox(this);
    m_wrap->setChecked(settings->wordWrap());
    m_scrollSync = new QCheckBox(this);
    m_scrollSync->setChecked(settings->scrollSync());
    m_autoSave = new QCheckBox(this);
    m_autoSave->setChecked(settings->autoSave());
    m_autoSaveInterval = new QSpinBox(this);
    m_autoSaveInterval->setRange(5, 600);
    m_autoSaveInterval->setValue(settings->autoSaveIntervalSeconds());
    layout->addRow(QStringLiteral("Theme"), m_theme);
    layout->addRow(QStringLiteral("Editor size"), m_fontSize);
    layout->addRow(QStringLiteral("Tab width"), m_tabWidth);
    layout->addRow(QStringLiteral("Word wrap"), m_wrap);
    layout->addRow(QStringLiteral("Scroll sync"), m_scrollSync);
    layout->addRow(QStringLiteral("Auto save"), m_autoSave);
    layout->addRow(QStringLiteral("Auto save seconds"), m_autoSaveInterval);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        QFont font = m_settings->editorFont();
        font.setPointSize(m_fontSize->value());
        m_settings->setTheme(m_theme->currentText());
        m_settings->setEditorFont(font);
        m_settings->setTabWidth(m_tabWidth->value());
        m_settings->setWordWrap(m_wrap->isChecked());
        m_settings->setScrollSync(m_scrollSync->isChecked());
        m_settings->setAutoSave(m_autoSave->isChecked());
        m_settings->setAutoSaveIntervalSeconds(m_autoSaveInterval->value());
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

