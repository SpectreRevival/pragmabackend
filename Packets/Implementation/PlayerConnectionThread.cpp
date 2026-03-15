#include "PacketProcessor.h"
#include "PlayerDatabase.h"
#include "SavedNotificationData.pb.h"

#include <PlayerConnectionThread.h>
#include <PlayerData.pb.h>

class HTTPPacketProcessor;
PlayerConnectionThread::PlayerConnectionThread(tcp::socket playerConnectionSocket)
    : playerConnectionSocket(std::move(playerConnectionSocket)){
    // Load any undelivered notifications into the notification queue
    std::unique_ptr<SavedNotificationData> savedNotifs = PlayerDatabase::Get().GetField<SavedNotificationData>(FieldKey::NOTIFICATION_DATA, playerId);
    for (int i = 0; i < savedNotifs->notificationstodeliver_size(); i++) {
        const SavedNotification& savedNotif = savedNotifs->notificationstodeliver(i);
        std::unique_ptr<Notification> notification = std::make_unique<Notification>(SpectreRpcType(savedNotif.rpctype()), savedNotif.notificationid(), savedNotif.notificationdata());
        notificationQueue.push(std::move(notification));
    }
    httpConnectionThread = std::jthread([this](std::stop_token stopToken) {
        HTTPConnectionThread(stopToken);
    });
}

const std::string& PlayerConnectionThread::GetPlayerId() {
    return playerId;
}

SpectreWebsocket* PlayerConnectionThread::GetWebsocketConnection() {
    return websocketConnection;
}

std::string PlayerConnectionThread::GetIPAddress() const {
    if (!IsWebsocketConnection()) {
        return playerConnectionSocket.remote_endpoint().address().to_string();
    } else {
        return ws->next_layer().remote_endpoint().address().to_string();
    }
}

unsigned int PlayerConnectionThread::GetPort() const {
    if (!IsWebsocketConnection()) {
        return playerConnectionSocket.remote_endpoint().port();
    } else {
        return ws->next_layer().remote_endpoint().port();
    }
}

void PlayerConnectionThread::EnqueueNotification(std::unique_ptr<Notification> notification) {
    std::unique_lock lock(notificationQueueMutex);
    notificationQueue.push(std::move(notification));
}

void PlayerConnectionThread::HTTPConnectionThread(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        http::async_read(playerConnectionSocket, httpDataBuffer, httpRequest,
            boost::beast::bind_front_handler(&PlayerConnectionThread::OnHTTPRequestReceive, shared_from_this()));
        while (!stopToken.stop_requested()) {
            if (httpMessageReceivedMutex.try_lock() && httpMessageReceived) {
                httpMessageReceived = false;
                break;
            }
            std::this_thread::yield();
        }
    }
}

void PlayerConnectionThread::WebsocketConnectionThread(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        ws->async_read(websocketDataBuffer, boost::beast::bind_front_handler(&PlayerConnectionThread::OnWebsocketMessageReceive, shared_from_this()));
        while (!stopToken.stop_requested()) {
            if (webSocketMessageReceivedMutex.try_lock() && webSocketMessageReceived) {
                webSocketMessageReceived = false;
                break;
            }
            std::this_thread::yield();
        }
    }
}

static std::string StripQueryParams(const std::string& url) {
    size_t pos = url.find('?');
    if (pos != std::string::npos) {
        if (url.at(0) == '/' && url.at(1) == '/') {
            return url.substr(1, pos);
        }
        return url.substr(0, pos);
    }
    if (url.at(0) == '/' && url.at(1) == '/') {
        return url.substr(1);
    }
    return url;
}

void PlayerConnectionThread::OnHTTPRequestReceive(boost::beast::error_code err, std::size_t readSize) {
    std::unique_lock lock(httpMessageReceivedMutex);
    httpMessageReceived = true;
    if (err.failed()) {
        spdlog::error("Failed to receive HTTP request due to error: {}", err.message());
        return;
    }
    if (boost::beast::websocket::is_upgrade(httpRequest)) {
        spdlog::info("Upgrading connection with {}:{} to websocket", GetIPAddress(), GetPort());
        ws = new boost::beast::websocket::stream<tcp::socket>(std::move(playerConnectionSocket));
        ws->accept(httpRequest);
        websocketConnection = new SpectreWebsocket(*ws, httpRequest);
        websocketConnectionThread = std::jthread([this](std::stop_token stopToken) {
            WebsocketConnectionThread(stopToken);
        });
        return;
    }
    std::string target = StripQueryParams(httpRequest.target());
    HTTPPacketProcessor* processor = HTTPPacketProcessor::GetProcessorForRoute(target);
    if (processor == nullptr) {
        spdlog::warn("Missing a handler for HTTP route {}", target);
        http::response<http::string_body> res;
        res.version(httpRequest.version());
        res.keep_alive(httpRequest.keep_alive());
        res.result(http::status::not_found);
        res.set(http::field::content_type, "application/json; charset=UTF-8");
        res.body() = "{}";
        res.prepare_payload();
        http::write(playerConnectionSocket, res);
        return;
    }
    processor->Process(httpRequest, playerConnectionSocket);
}

void PlayerConnectionThread::OnWebsocketMessageReceive(boost::beast::error_code err, std::size_t readSize) {
    std::unique_lock lock(webSocketMessageReceivedMutex);
    webSocketMessageReceived = true;
    if (err.failed()) {
        spdlog::error("Failed to receive websocket message: {}", err.message());
        return;
    }
    SpectreWebsocketRequest wsReq(*websocketConnection, websocketDataBuffer);
    WebsocketPacketProcessor* processor = WebsocketPacketProcessor::GetProcessorForRpc(wsReq.GetRequestType());
    if (processor == nullptr) {
        spdlog::warn("No processor for message type {}, dropping packet", wsReq.GetRequestType().GetName());
        return;
    }
    processor->Process(wsReq, *websocketConnection);
}

PlayerConnectionThread::~PlayerConnectionThread() {
    httpConnectionThread.request_stop();
    websocketConnectionThread.request_stop();
    httpConnectionThread.join();
    websocketConnectionThread.join();
    std::unique_ptr<SavedNotificationData> notificationData = PlayerDatabase::Get().GetField<SavedNotificationData>(FieldKey::NOTIFICATION_DATA, playerId);
    while (!notificationQueue.empty()){
        Notification& notif = *notificationQueue.front();
        SavedNotification notifSaved;
        notifSaved.set_notificationid(notif.GetNotificationId());
        notifSaved.set_rpctype(notif.GetNotificationType().GetName());
        notifSaved.set_notificationdata(notif.GetNotificationData());
        notificationData->add_notificationstodeliver()->CopyFrom(notifSaved);
        notificationQueue.pop();
    }
    PlayerDatabase::Get().SetField(FieldKey::NOTIFICATION_DATA, notificationData.get(),  playerId);
    delete websocketConnection;
    delete ws;
}

bool PlayerConnectionThread::IsWebsocketConnection() const {
    return ws != nullptr;
}