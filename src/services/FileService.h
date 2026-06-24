#pragma once

#include <QString>

struct FileResult
{
    bool ok = false;
    QString text;
    QString error;
};

class FileService
{
public:
    static FileResult readTextFile(const QString& path);
    static FileResult writeTextFile(const QString& path, const QString& text);
    static bool isSupportedMarkdownPath(const QString& path);
};

