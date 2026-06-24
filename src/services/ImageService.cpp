#include "services/ImageService.h"

#include <QFileInfo>

QString ImageService::markdownForImagePath(const QString& path)
{
    return QStringLiteral("![%1](%2)").arg(QFileInfo(path).completeBaseName(), path);
}

