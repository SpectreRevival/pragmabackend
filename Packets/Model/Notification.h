#pragma once
#include <SpectreRpcType.h>
#include <SpectreWebsocket.h>

class Notification {
  private:
    SpectreRpcType m_notificationType;

  public:
    virtual ~Notification() = default;
    explicit Notification(SpectreRpcType notificationType);

    const SpectreRpcType& GetNotificationType() const;
    virtual void SendTo(SpectreWebsocket& sock) const = 0;
};