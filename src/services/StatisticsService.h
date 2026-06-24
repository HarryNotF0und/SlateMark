#pragma once

#include "services/MarkdownService.h"

class StatisticsService
{
public:
    static DocumentStatistics calculate(const QString& markdown);
};

