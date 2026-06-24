#include "preview/PreviewBridge.h"

PreviewBridge::PreviewBridge(QObject* parent)
    : QObject(parent)
{
}

void PreviewBridge::previewClicked(int sourceLine)
{
    emit sourceLineClicked(sourceLine);
}

void PreviewBridge::previewScrolled(double ratio)
{
    emit scrollRatioChanged(ratio);
}

