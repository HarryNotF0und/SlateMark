#pragma once

#include <QDialog>

class QSpinBox;

class TableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TableDialog(QWidget* parent = nullptr);
    int rows() const;
    int columns() const;

private:
    QSpinBox* m_rows = nullptr;
    QSpinBox* m_columns = nullptr;
};

