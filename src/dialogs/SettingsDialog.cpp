#include "dialogs/SettingsDialog.h"

#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(SettingsService* settings, QWidget* parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(QStringLiteral("Settings"));
    setObjectName(QStringLiteral("settingsDialog"));
    setMinimumWidth(400);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(16);

    auto* form = new QGridLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(12);
    form->setColumnStretch(0, 1);
    root->addLayout(form);

    auto addRow = [form](int row, const QString& text, QWidget* field) {
        auto* label = new QLabel(text, field->parentWidget());
        label->setObjectName(QStringLiteral("settingsLabel"));
        label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        field->setObjectName(field->objectName().isEmpty() ? QStringLiteral("settingsField") : field->objectName());
        field->setFixedHeight(38);
        field->setMinimumWidth(180);
        form->addWidget(label, row, 0);
        form->addWidget(field, row, 1);
    };

    m_theme = new QComboBox(this);
    m_theme->setObjectName(QStringLiteral("settingsCombo"));
    m_theme->addItems({QStringLiteral("dark"), QStringLiteral("light"), QStringLiteral("system")});
    m_theme->setCurrentText(settings->theme());

    m_fontSize = new QSpinBox(this);
    m_fontSize->setObjectName(QStringLiteral("settingsSpin"));
    m_fontSize->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_fontSize->setRange(8, 28);
    m_fontSize->setValue(settings->editorFont().pointSize());

    m_tabWidth = new QSpinBox(this);
    m_tabWidth->setObjectName(QStringLiteral("settingsSpin"));
    m_tabWidth->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_tabWidth->setRange(2, 12);
    m_tabWidth->setValue(settings->tabWidth());

    m_wrap = new QCheckBox(this);
    m_wrap->setObjectName(QStringLiteral("settingsToggle"));
    m_wrap->setChecked(settings->wordWrap());

    m_scrollSync = new QCheckBox(this);
    m_scrollSync->setObjectName(QStringLiteral("settingsToggle"));
    m_scrollSync->setChecked(settings->scrollSync());

    m_autoSave = new QCheckBox(this);
    m_autoSave->setObjectName(QStringLiteral("settingsToggle"));
    m_autoSave->setChecked(settings->autoSave());

    m_autoSaveInterval = new QSpinBox(this);
    m_autoSaveInterval->setObjectName(QStringLiteral("settingsSpin"));
    m_autoSaveInterval->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_autoSaveInterval->setRange(5, 600);
    m_autoSaveInterval->setValue(settings->autoSaveIntervalSeconds());

    addRow(0, QStringLiteral("Theme"), m_theme);
    addRow(1, QStringLiteral("Editor size"), m_fontSize);
    addRow(2, QStringLiteral("Tab width"), m_tabWidth);
    addRow(3, QStringLiteral("Word wrap"), m_wrap);
    addRow(4, QStringLiteral("Scroll sync"), m_scrollSync);
    addRow(5, QStringLiteral("Auto save"), m_autoSave);
    addRow(6, QStringLiteral("Auto save seconds"), m_autoSaveInterval);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->setObjectName(QStringLiteral("settingsButtons"));
    buttons->button(QDialogButtonBox::Ok)->setObjectName(QStringLiteral("settingsPrimaryButton"));
    buttons->button(QDialogButtonBox::Cancel)->setObjectName(QStringLiteral("settingsSecondaryButton"));
    buttons->button(QDialogButtonBox::Ok)->setFixedSize(88, 38);
    buttons->button(QDialogButtonBox::Cancel)->setFixedSize(88, 38);
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
    root->addWidget(buttons, 0, Qt::AlignRight);
}
