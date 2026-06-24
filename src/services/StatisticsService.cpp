#include "services/StatisticsService.h"

DocumentStatistics StatisticsService::calculate(const QString& markdown)
{
    return MarkdownService::statistics(markdown);
}

