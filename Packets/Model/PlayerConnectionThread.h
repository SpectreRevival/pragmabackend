#pragma once
#include <string>
#include <Notification.h>

class PlayerConnectionThread {
private:
    std::jthread connectionThread;
    std::stop_token connectionStopToken;
public:
    PlayerConnectionThread(unsigned int port);
    PlayerConnectionThread(PlayerConnectionThread& other) = delete;
    PlayerConnectionThread() = delete;
    void EnqueueNotification(std::unique_ptr<Notification> notification);
    const std::string& GetPlayerId();
    SpectreWebsocket* GetWebsocketConnection();
    const std::string& GetIPAddress();
    void Shutdown();
};