#include "services/ExportService.h"

#include "services/FileService.h"

FileResult ExportService::exportHtml(const QString& path, const QString& markdown, const HtmlExportOptions& options)
{
    return FileService::writeTextFile(path, MarkdownService::completeHtmlDocument(markdown, options));
}

