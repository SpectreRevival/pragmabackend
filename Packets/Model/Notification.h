#pragma once
#include <SpectreRpcType.h>
#include <SpectreWebsocket.h>

class Notification {
  private:
    SpectreRpcType notificationType;
    const std::string notificationId;
    std::string notificationData;

  public:
    Notification(const SpectreRpcType& notificationType, const pbuf::Message& notificationData);
    Notification(const SpectreRpcType& notificationType, const std::string& notificationId, const std::string& notificationData);
    const SpectreRpcType& GetNotificationType() const;
    const std::string& GetNotificationId() const;
    const std::string& GetNotificationData() const;
};