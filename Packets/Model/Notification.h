#pragma once
#include <SpectreRpcType.h>
#include <SpectreWebsocket.h>

class Notification {
  private:
    SpectreRpcType notificationType;

  public:
    virtual ~Notification() = default;
    Notification(Notification& other) = default;
    Notification(Notification&& other) = default;
    explicit Notification(SpectreRpcType notificationType);

    [[nodiscard]] const SpectreRpcType& GetNotificationType() const;
    virtual void SendTo(SpectreWebsocket& sock) const = 0;
};