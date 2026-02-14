//
// Created by astro on 27/09/2025.
//

#include "StaticHTTPPackets.cpp" // NOLINT
#include "StaticWSPackets.cpp"   // NOLINT
#include "SubmitProviderIdHandler.h"

#include <AuthenticateHandler.h>
#include <CreatePartyProcessor.h>
#include <EnterMatchmakingProcessor.h>
#include <FetchPlayerLoadoutsProcessor.h>
#include <FieldFetchProcessor.h>
#include <FieldKey.h>
#include <GetBulkProfileDataProcessor.h>
#include <GetLoginDataProcessor.h>
#include <GetPlayerDataProcessor.h>
#include <HeartbeatProcessor.h>
#include <Inventory.pb.h>
#include <IsInPartyHandler.h>
#include <LegacyPlayerData.pb.h>
#include <OutfitLoadout.pb.h>
#include <PacketProcessor.h>
#include <PlayerDatabase.h>
#include <SaveOutfitLoadoutProcessor.h>
#include <SavePlayerDataProcessor.h>
#include <SaveWeaponLoadoutProcessor.h>
#include <SetReadyProcessor.h>
#include <SpectreWebsocket.h>
#include <SpectreWebsocketRequest.h>
#include <StaticResponseProcessorHTTP.h>
#include <StaticResponseProcessorWS.h>
#include <UpdateItemV4Processor.h>
#include <UpdateItemsV0Processor.h>
#include <UpdatePartyPlayerProcessor.h>
#include <UpdatePartyProcessor.h>
#include <WeaponLoadout.pb.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <memory>
#include <spdlog/async.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>
#include <PersistenceUtilities.h>

static uint16_t gamePort = 8081;
static uint16_t socialPort = 8082;
static uint16_t wsPort = 80;

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

static std::shared_ptr<spdlog::logger> logger;

static void SetupLogger() {
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>((PersistenceUtilities::GetSavePath() / "logs" / "app.log").string(), true);

    // Optional: Customize sink formats
    consoleSink->set_pattern("[%T] [%^%l%$] %v");
    fileSink->set_pattern("[%Y-%m-%d %T] [%l] %v");

    // Combine sinks into one logger
    std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};
    logger = std::make_shared<spdlog::logger>("pragma", sinks.begin(), sinks.end());

    // Register and use the logger
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
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

static void Session(tcp::socket sock) {
    try {
        beast::flat_buffer buffer; // http parser scratch space
        http::request<http::string_body> req;

        // blocking read
        http::read(sock, buffer, req);

        // detect websocket upgrade and switch protocols if requested
        if (websocket::is_upgrade(req)) {
            logger->info("trying to accept ws handshake");
            websocket::stream<tcp::socket> ws(std::move(sock));
            ws.accept(req); // complete ws handshake
            SpectreWebsocket sock(ws, req);
            logger->info("upgraded connection with {}:{} to websocket",
                         ws.next_layer().remote_endpoint().address().to_string(), ws.next_layer().remote_endpoint().port());

            // basic echo loop so clients have something to talk to
            for (;;) {
                beast::flat_buffer wsbuf;
                ws.read(wsbuf); // this blocks until a message arrives or the peer closes
                SpectreWebsocketRequest req(sock, wsbuf);
                auto route = WebsocketPacketProcessor::GetProcessorForRpc(req.GetRequestType());
                if (route == nullptr) {
                    logger->warn("no packet processor found for WS requestType: " + req.GetRequestType().GetName());
                    continue;
                }
                route->Process(req, sock);
            }
        }
        auto target = StripQueryParams(std::string(req.target())); // remove ?query so routing is stable
        HTTPPacketProcessor* processor = HTTPPacketProcessor::GetProcessorForRoute(target);
        if (processor == nullptr) {
            logger->warn("missing a handler for http route " + target);
            // send a 404 if no processor found
            http::response<http::string_body> res;
            res.version(req.version());
            res.keep_alive(req.keep_alive());
            res.result(http::status::not_found);
            res.set(http::field::content_type, "application/json; charset=UTF-8");
            res.body() = "{}";
            res.prepare_payload();
            http::write(sock, res);
            return;
        }
        processor->Process(req, sock);
    } catch (std::exception& e) {
        // any parse error, socket close, or ws close lands here
        logger->error("session error: ");
        logger->error(e.what());
    }
}

static void ConnectionAcceptor(unsigned short port) {
    try {
        asio::io_context ioc; // we use sync ops but asio still wants an io_context around

        // tls setup. load cert / key

        tcp::acceptor acc(ioc, tcp::endpoint(asio::ip::make_address("0.0.0.0"), port));

        // accept loop forever. each client gets one detached thread
        // if we want to shut down clean, don't detach, keep thread handles
        for (;;) {
            tcp::socket sock(ioc);
            acc.accept(sock); // blocks until a client connects
            // each session owns its TLS handshake and stream
            std::thread([s = std::move(sock)]() mutable {
                Session(std::move(s));
            }).detach();
        }
    } catch (std::exception& e) {
        logger->error("fatal exception(rip acceptor thread): ");
        logger->error(e.what());
    }
}

// the main accept loop
// binds to 127.0.0.1:443, accepts a connection, spins a thread, repeat
int main(int argc, char** argv) {
    if (argc == 4) {
        gamePort = std::stoi(std::string(argv[1]));
        socialPort = std::stoi(std::string(argv[2]));
        wsPort = std::stoi(std::string(argv[3]));
    }
    try {
        SetupLogger();
        logger->info("starting server...");
        logger->info("Registering handlers...");
        RegisterStaticHTTPHandlers();
        RegisterStaticWSHandlers();

        // feel like this needs cleaning up T_T

        new SubmitProviderIdHandler("/v1/submitproviderid");
        new AuthenticateHandler("/v1/account/authenticateorcreatev2");
        new HeartbeatProcessor(SpectreRpcType("PlayerSessionRpc.HeartbeatV1Request"));
        new FieldFetchProcessor<Inventory>(
            SpectreRpcType("InventoryRpc.GetInventoryV2Request"),
            FieldKey::PLAYER_INVENTORY);
        logger->info("Finished registering handlers");
        new UpdateItemsV0Processor(SpectreRpcType("InventoryRpc.UpdateItemsV0Request"));
        new UpdateItemV4Processor(SpectreRpcType("InventoryRpc.UpdateItemV4Request"));
        new FieldFetchProcessor<OutfitLoadouts>(
            SpectreRpcType("MtnLoadoutServiceRpc.FetchPlayerOutfitLoadoutsV1Request"),
            FieldKey::PLAYER_OUTFIT_LOADOUT);
        new FetchPlayerLoadoutsProcessor(
            SpectreRpcType("MtnLoadoutServiceRpc.FetchPlayerWeaponLoadoutsV1Request"));
        new SaveWeaponLoadoutProcessor(
            SpectreRpcType("MtnLoadoutServiceRpc.SaveWeaponLoadoutV1Request"));
        new SaveOutfitLoadoutProcessor(
            SpectreRpcType("MtnLoadoutServiceRpc.SaveOutfitLoadoutV1Request"));
        new GetPlayerDataProcessor(
            SpectreRpcType("MtnPlayerDataServiceRpc.GetAllPlayerDataClientV1Request"));
        new GetBulkProfileDataProcessor(
            SpectreRpcType("MtnPlayerDataServiceRpc.GetBulkProfileDataClientV1Request"));
        new SavePlayerDataProcessor(
            SpectreRpcType("MtnPlayerDataServiceRpc.SavePlayerDataForClientV1Request"));
        new FieldFetchProcessor<LegacyPlayerData>(
            SpectreRpcType("MtnPlayerDataServiceRpc.GetPlayerLegacyDataV1Request"),
            FieldKey::PLAYER_LEGACY_DATA);
        new GetLoginDataProcessor(
            SpectreRpcType("GameDataRpc.GetLoginDataV3Request"));
        new CreatePartyProcessor(
            SpectreRpcType("PartyRpc.CreateV1Request"));
        new SetReadyProcessor(
            SpectreRpcType("PartyRpc.SetReadyStateV1Request"));
        new UpdatePartyProcessor(
            SpectreRpcType("PartyRpc.UpdatePartyV1Request"));
        new UpdatePartyPlayerProcessor(
            SpectreRpcType("PartyRpc.UpdatePartyPlayerV1Request"));
        new EnterMatchmakingProcessor(
            SpectreRpcType("PartyRpc.EnterMatchmakingV1Request"));
        new IsInPartyHandler(
            SpectreRpcType("MultiplayerRpc.InitializePartyV1Request"));
        new IsInPartyHandler(
            SpectreRpcType("MultiplayerRpc.SyncPartyV1Request"));
        std::thread gameThread = std::thread([] {
            ConnectionAcceptor(gamePort); // game
        });
        std::thread socialThread = std::thread([] {
            ConnectionAcceptor(socialPort); // social
        });
        std::thread wsThread = std::thread([] {
            ConnectionAcceptor(wsPort); // websockets
        });
        logger->info("acceptor threads started");
        gameThread.join();
        socialThread.join();
        wsThread.join();
    } catch (const std::exception& e) {
        logger->error("unhandled exception caught in main: {}", e.what());
    }
    return 0;
}