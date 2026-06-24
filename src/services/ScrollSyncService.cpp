#include "services/ScrollSyncService.h"

ScrollSyncService::ScrollSyncService(QObject* parent)
    : QObject(parent)
{
}

bool ScrollSyncService::enabled() const { return m_enabled; }

void ScrollSyncService::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }
    m_enabled = enabled;
    emit enabledChanged(enabled);
}

