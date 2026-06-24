#pragma once

#include "services/MarkdownService.h"

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget* parent = nullptr);
    HtmlExportOptions options() const;

private:
    QLineEdit* m_title = nullptr;
    QLineEdit* m_author = nullptr;
    QCheckBox* m_inlineCss = nullptr;
    QCheckBox* m_toc = nullptr;
    QCheckBox* m_codeCopy = nullptr;
    QCheckBox* m_dark = nullptr;
    QPlainTextEdit* m_customCss = nullptr;
};

