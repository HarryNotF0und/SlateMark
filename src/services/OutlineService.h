#pragma once

#include "services/MarkdownService.h"

class OutlineService
{
public:
    static QVector<MarkdownHeading> parse(const QString& markdown);
};

