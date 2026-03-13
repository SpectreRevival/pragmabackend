#include <Notification.h>
#include <uuid.h>

static std::mt19937 rng = std::mt19937{std::random_device{}()};
static uuids::uuid_random_generator gen{rng};

Notification::Notification(const SpectreRpcType& notificationType, std::unique_ptr<pbuf::Message> notificationData)
    : notificationType(notificationType), notificationData(std::move(notificationData)), notificationId(uuids::to_string(gen())) {
}

const SpectreRpcType& Notification::GetNotificationType() const {
    return notificationType;
}