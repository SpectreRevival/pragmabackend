#pragma once
#include <string>
#include <Notification.h>
#include <queue>
#include <memory>

class PlayerConnectionThread {
private:
    std::string playerId;
    std::queue<std::unique_ptr<Notification>> notificationQueue;
    std::mutex notificationQueueMutex;
    boost::beast::flat_buffer httpDataBuffer;
    http::request<http::string_body> httpRequest;
    std::jthread httpConnectionThread;
    boost::beast::flat_buffer websocketDataBuffer;
    std::jthread websocketConnectionThread;
    void HTTPConnectionThread(std::stop_token stopToken);
    void WebsocketConnectionThread(std::stop_token stopToken);
    tcp::socket playerConnectionSocket;
    SpectreWebsocket* websocketConnection{};
    boost::beast::websocket::stream<tcp::socket>* ws{};
    void OnWebsocketMessageReceive(boost::beast::error_code err, std::size_t readSize);
    void OnHTTPRequestReceive(boost::beast::error_code err, std::size_t readSize);
    bool httpMessageReceived = false;
    std::mutex httpMessageReceivedMutex;
    bool webSocketMessageReceived = false;
    std::mutex webSocketMessageReceivedMutex;
public:
    explicit PlayerConnectionThread(tcp::socket playerConnectionSocket);
    ~PlayerConnectionThread();
    PlayerConnectionThread(PlayerConnectionThread& other) = delete;
    PlayerConnectionThread() = delete;
    void EnqueueNotification(std::unique_ptr<Notification> notification);
    const std::string& GetPlayerId();
    SpectreWebsocket* GetWebsocketConnection();
    std::string GetIPAddress() const;
    unsigned int GetPort() const;
    bool IsWebsocketConnection() const;
};