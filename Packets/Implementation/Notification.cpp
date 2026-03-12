#include <Notification.h>

Notification::Notification(const SpectreRpcType notificationType)
    : notificationType(notificationType) {
}

const SpectreRpcType& Notification::GetNotificationType() const {
    return notificationType;
}