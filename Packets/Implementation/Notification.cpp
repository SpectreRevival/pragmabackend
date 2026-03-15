#include <Notification.h>
#include <uuid.h>

static std::mt19937 rng = std::mt19937{std::random_device{}()};
static uuids::uuid_random_generator gen{rng};

Notification::Notification(const SpectreRpcType& notificationType, const pbuf::Message& notificationData)
    : notificationType(notificationType), notificationId(uuids::to_string(gen())), notificationData(notificationData.SerializeAsString()) {
}

Notification::Notification(const SpectreRpcType& notificationType, const std::string& notificationId, const std::string& notificationData)
    : notificationType(notificationType), notificationId(notificationId), notificationData(notificationData) {

}

const SpectreRpcType& Notification::GetNotificationType() const {
    return notificationType;
}

const std::string& Notification::GetNotificationData() const {
    return notificationData;
}

const std::string& Notification::GetNotificationId() const {
    return notificationId;
}