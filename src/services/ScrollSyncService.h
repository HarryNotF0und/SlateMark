#pragma once

#include <QObject>

class ScrollSyncService : public QObject
{
    Q_OBJECT

public:
    explicit ScrollSyncService(QObject* parent = nullptr);
    bool enabled() const;
    void setEnabled(bool enabled);

signals:
    void enabledChanged(bool enabled);

private:
    bool m_enabled = true;
};

