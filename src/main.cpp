#include "Core/Config.h"
#include "Runtime/LockpickHook.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <exception>
#include <filesystem>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>

namespace
{
    void InitializeLogging()
    {
        auto logDirectory = SKSE::log::log_directory();
        if (!logDirectory) {
            return;
        }

        std::error_code error;
        std::filesystem::create_directories(*logDirectory, error);
        if (error) {
            return;
        }

        try {
            const auto logPath = *logDirectory / "LockpickRevealBar.log";
            auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.string(), true);
            auto logger = std::make_shared<spdlog::logger>("lockpick_reveal_bar", std::move(sink));

            spdlog::set_default_logger(std::move(logger));
            spdlog::set_level(spdlog::level::info);
            spdlog::flush_on(spdlog::level::info);
        } catch (const std::exception&) {
            return;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    InitializeLogging();

    constexpr auto configPath = "Data/SKSE/Plugins/LockpickRevealBar.ini";
    const auto config = lrb::LoadConfig(std::filesystem::path(configPath));
    if (!config.enable) {
        SKSE::log::info("LockpickRevealBar disabled by ini");
        return true;
    }

    lrb::InstallLockpickHook(config);
    SKSE::log::info("LockpickRevealBar startup complete");
    return true;
}
