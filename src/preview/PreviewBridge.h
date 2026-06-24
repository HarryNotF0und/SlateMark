#pragma once

#include <QObject>

class PreviewBridge : public QObject
{
    Q_OBJECT

public:
    explicit PreviewBridge(QObject* parent = nullptr);

public slots:
    void previewClicked(int sourceLine);
    void previewScrolled(double ratio);

signals:
    void sourceLineClicked(int sourceLine);
    void scrollRatioChanged(double ratio);
};

