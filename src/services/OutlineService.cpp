#include "services/OutlineService.h"

QVector<MarkdownHeading> OutlineService::parse(const QString& markdown)
{
    return MarkdownService::outline(markdown);
}

