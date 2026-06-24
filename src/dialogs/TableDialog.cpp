#include "dialogs/TableDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>

TableDialog::TableDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Insert Table"));
    auto* layout = new QFormLayout(this);
    m_rows = new QSpinBox(this);
    m_rows->setRange(1, 20);
    m_rows->setValue(3);
    m_columns = new QSpinBox(this);
    m_columns->setRange(1, 10);
    m_columns->setValue(3);
    layout->addRow(QStringLiteral("Rows"), m_rows);
    layout->addRow(QStringLiteral("Columns"), m_columns);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

int TableDialog::rows() const { return m_rows->value(); }
int TableDialog::columns() const { return m_columns->value(); }

