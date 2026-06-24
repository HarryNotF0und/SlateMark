#pragma once

#include <QString>

struct Document
{
    QString filePath;
    QString title = QStringLiteral("Untitled.md");
    QString content;
    QString recoveryPath;
    bool modified = false;
    bool untitled = true;
};

