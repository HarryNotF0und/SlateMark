#pragma once

#include <QString>

class LoggingService
{
public:
    static void install();
    static void log(const QString& message);
};

