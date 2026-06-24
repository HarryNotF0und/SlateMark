#include "dialogs/ExportDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPlainTextEdit>

ExportDialog::ExportDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Export HTML"));
    auto* layout = new QFormLayout(this);
    m_title = new QLineEdit(QStringLiteral("SlateMark Export"), this);
    m_author = new QLineEdit(this);
    m_inlineCss = new QCheckBox(QStringLiteral("Inline CSS"), this);
    m_inlineCss->setChecked(true);
    m_toc = new QCheckBox(QStringLiteral("Include table of contents"), this);
    m_codeCopy = new QCheckBox(QStringLiteral("Show code copy buttons"), this);
    m_codeCopy->setChecked(true);
    m_dark = new QCheckBox(QStringLiteral("Use dark preview theme"), this);
    m_dark->setChecked(true);
    m_customCss = new QPlainTextEdit(this);
    m_customCss->setPlaceholderText(QStringLiteral("Optional custom CSS"));
    m_customCss->setFixedHeight(100);
    layout->addRow(QStringLiteral("Title"), m_title);
    layout->addRow(QStringLiteral("Author"), m_author);
    layout->addRow(m_inlineCss);
    layout->addRow(m_toc);
    layout->addRow(m_codeCopy);
    layout->addRow(m_dark);
    layout->addRow(QStringLiteral("Custom CSS"), m_customCss);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

HtmlExportOptions ExportDialog::options() const
{
    return {
        m_title->text(),
        m_author->text(),
        m_inlineCss->isChecked(),
        m_toc->isChecked(),
        m_codeCopy->isChecked(),
        m_dark->isChecked(),
        m_customCss->toPlainText()
    };
}

