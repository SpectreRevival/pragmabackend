#include "SpectreRpcType.h"

#include <Notification.h>

Notification::Notification(const SpectreRpcType notificationType)
    : m_notificationType(notificationType) {
}

const SpectreRpcType& Notification::GetNotificationType() const {
    return m_notificationType;
}