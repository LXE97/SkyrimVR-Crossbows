#undef ENABLE_SKYRIM_AE
#undef ENABLE_SKYRIM_SE

#include <spdlog/sinks/basic_file_sink.h>
#include "SKSE/API.h"
#include "SKSE/Impl/Stubs.h"
#include "VRCR.h"
#include "higgsinterface001.h"
#include "vrikinterface001.h"
#include "Windows.h"

void MessageListener(SKSE::MessagingInterface::Message *message);
void OnPapyrusVRMessage(SKSE::MessagingInterface::Message *message);
void SetupLog();
uint8_t GetPluginID();

// Interfaces for communicating with other SKSE plugins.
static SKSE::detail::SKSEMessagingInterface *g_messaging;
static SKSE::PluginHandle g_pluginHandle = 0xFFFFFFFF;
PapyrusVRAPI *g_papyrusvr;

void InitializeHooking()
{
    SKSE::log::trace("Initializing trampoline...");
    auto &trampoline = SKSE::GetTrampoline();
    trampoline.create(14);
    SKSE::log::trace("Trampoline initialized.");
}

/// Main plugin entry point.
SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    SKSE::Init(skse);
    SetupLog();

    SKSE::GetMessagingInterface()->RegisterListener(MessageListener);

    g_pluginHandle = skse->GetPluginHandle();
    g_messaging = (SKSE::detail::SKSEMessagingInterface *)skse->QueryInterface(SKSE::LoadInterface::kMessaging);
    VRCR::g_task = (SKSE::detail::SKSETaskInterface *)(skse->QueryInterface(SKSE::LoadInterface::kTask));

    return true;
}

/// Receives messages about the game's state that SKSE broadcasts to all plugins.
void MessageListener(SKSE::MessagingInterface::Message *message)
{
    using namespace SKSE::log;

    switch (message->type)
    {
    case SKSE::MessagingInterface::kPostLoad:
        info("kPostLoad: sent to registered plugins once all plugins have been loaded");
        info("Registering for SkyrimVRTools messages");
        // SkyrimVRTools registered here
        g_messaging->RegisterListener(g_pluginHandle, "SkyrimVRTools", OnPapyrusVRMessage);
        break;

    case SKSE::MessagingInterface::kPostPostLoad:
        info("kPostPostLoad: querying higgs interface");

        g_higgsInterface = HiggsPluginAPI::GetHiggsInterface001(g_pluginHandle, g_messaging);
        if (g_higgsInterface)
        {
            info("Got higgs interface");
        }
        info("kPostPostLoad: querying VRIK interface");
        g_vrikInterface = vrikPluginApi::getVrikInterface001(g_pluginHandle, g_messaging);
        if (g_vrikInterface)
        {
            info("Got higgs interface");
        }
        break;

    case SKSE::MessagingInterface::kInputLoaded:
        info("kInputLoaded: sent right after game input is loaded, right before the main menu initializes");
        break;

    case SKSE::MessagingInterface::kDataLoaded:
        info("kDataLoaded: sent after the data handler has loaded all its forms");
        InitializeHooking();
        VRCR::thisPluginID = GetPluginID();
        // Initialize our mod.
        VRCR::StartMod();
        break;

    case SKSE::MessagingInterface::kPreLoadGame:
        info("kPreLoadGame: sent immediately before savegame is read");
        VRCR::PreGameLoad();
        break;

    case SKSE::MessagingInterface::kPostLoadGame:
        info("kPostLoadGame: sent after an attempt to load a saved game has finished");
        VRCR::GameLoad();
        break;

    case SKSE::MessagingInterface::kSaveGame:
        info("kSaveGame");
        break;

    case SKSE::MessagingInterface::kDeleteGame:
        info("kDeleteGame: sent right before deleting the .skse cosave and the .ess save");
        break;

    case SKSE::MessagingInterface::kNewGame:
        info("kNewGame: sent after a new game is created, before the game has loaded");
        break;

    default:
        info("Unknown system message of type: {}", message->type);
        break;
    }
}

// Listener for papyrusvr Messages
void OnPapyrusVRMessage(SKSE::MessagingInterface::Message *message)
{
    if (message)
    {
        if (message->type == kPapyrusVR_Message_Init && message->data)
        {
            SKSE::log::info("SkyrimVRTools Init Message recived with valid data, registering for callback");
            g_papyrusvr = (PapyrusVRAPI *)message->data;

            VRCR::g_VRManager = g_papyrusvr->GetVRManager();
            VRCR::g_OVRHookManager = g_papyrusvr->GetOpenVRHook();
            // TODO: Might not want to store these in case controller powers off during game
            VRCR::l_controller = VRCR::g_OVRHookManager->GetVRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
            VRCR::r_controller = VRCR::g_OVRHookManager->GetVRSystem()->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
        }
    }
}

/// Initialize logging system.
void SetupLog()
{
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder)
    {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
        return;
    }
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::info);
}

uint8_t GetPluginID()
{
    RE::TESDataHandler *dataHandler = RE::TESDataHandler::GetSingleton();

    if (dataHandler)
    {
        const RE::TESFile *modInfo = dataHandler->LookupLoadedModByName(VRCR::thisPluginName.c_str());

        if (modInfo)
        {
            return modInfo->GetCompileIndex();
        }
    }
    return 0;
}
