#pragma once
#include <SpectreRpcType.h>
#include <SpectreWebsocket.h>

class Notification {
  private:
    SpectreRpcType notificationType;
    const std::string notificationId;
    std::unique_ptr<pbuf::Message> notificationData;

  public:
    Notification(const SpectreRpcType& notificationType, std::unique_ptr<pbuf::Message> notificationData);
    const SpectreRpcType& GetNotificationType() const;
};