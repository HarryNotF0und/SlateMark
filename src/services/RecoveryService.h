#pragma once

#include <QString>

class RecoveryService
{
public:
    static QString recoveryDirectory();
    static QString recoveryPathFor(const QString& documentKey);
    static bool saveRecovery(const QString& documentKey, const QString& text, QString* error = nullptr);
    static QString loadRecovery(const QString& documentKey);
    static void removeRecovery(const QString& documentKey);
};

