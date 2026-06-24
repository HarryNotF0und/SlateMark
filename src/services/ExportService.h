#pragma once

#include "services/FileService.h"
#include "services/MarkdownService.h"

#include <QString>

class ExportService
{
public:
    static FileResult exportHtml(const QString& path, const QString& markdown, const HtmlExportOptions& options);
};
