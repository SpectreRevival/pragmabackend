//
// Created by astro on 27/09/2025.
//

#include "GetFriendsListAndRegisterOnlineHandler.h"
#include "PlayerConnectionThread.h"
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
#include <PersistenceUtilities.h>
#include <PlayerDatabase.h>
#include <SaveOutfitLoadoutProcessor.h>
#include <SavePlayerDataProcessor.h>
#include <SaveWeaponLoadoutProcessor.h>
#include <SetPlayerPresenceHandler.h>
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
#include <csignal>
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

static uint16_t gamePort = 8081;
static uint16_t socialPort = 8082;
static uint16_t wsPort = 80;

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;
namespace fs = std::filesystem;

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

std::vector<PlayerConnectionThread*> playerConnections{};

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
            playerConnections.push_back(new PlayerConnectionThread(std::move(sock)));
        }
    } catch (std::exception& e) {
        logger->error("fatal exception(rip acceptor thread): ");
        logger->error(e.what());
    }
}

bool bStop = false;

static void HandleInterrupt(int /*sigint*/) {
    bStop = true;
}

static void ShutdownServer() {
    for (PlayerConnectionThread* connection : playerConnections) {
        spdlog::info("Shutting down connection to {}:{}", connection->GetIPAddress(), connection->GetPort());
        delete connection;
    }
}

// the main accept loop
// binds to 127.0.0.1:443, accepts a connection, spins a thread, repeat
int main(int argc, char** argv) {
    (void)signal(2, HandleInterrupt);
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
        new SetPlayerPresenceHandler(
            SpectreRpcType("FriendRpc.SetPresenceV1Request"));
        new GetFriendsListAndRegisterOnlineHandler(
            SpectreRpcType("FriendRpc.GetFriendListAndRegisterOnlineV1Request")
            );
    } catch (std::exception& e) {
        spdlog::error("Failed to initialize handlers, exiting...", e.what());
    }
    try{
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
        std::ofstream serverLockFile("./server.lock", std::ios::trunc | std::ios::out);
        while (!bStop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        serverLockFile.close();
        fs::remove("./server.lock");
    } catch (const std::exception& e) {
        logger->error("unhandled exception caught in main: {}", e.what());
    }
    spdlog::info("Shutting down server...");
    ShutdownServer();
    spdlog::info("Shutting down logging...");
    spdlog::shutdown();
    return 0;
}