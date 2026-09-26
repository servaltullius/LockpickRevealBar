#include "Runtime/LockpickHook.h"

#include "Core/Heat.h"
#include "Core/LockLayout.h"
#include "Core/Palette.h"
#include "Core/RevealField.h"
#include "Core/Style.h"
#include "Runtime/BarRenderer.h"
#include "Runtime/PickHealth.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <random>

namespace
{
    using Clock = std::chrono::steady_clock;

    // 1.7.x inserted 0x14 bytes before the sweet spot block (see LockLayout.h).
    constexpr std::ptrdiff_t kShiftedDelta = 0x14;

    struct Session
    {
        bool closed{ false };
        bool haveLock{ false };
        bool attachFailed{ false };
        lrb::LockGeometry lock;
        std::uint32_t brokenPicks{ 0 };
        lrb::AttemptStyle style;
        lrb::RevealField field;
        Clock::time_point lastTick{};
        Clock::time_point lastDebugLog{};
    };

    lrb::Config g_config;
    lrb::Layout g_layout{ lrb::Layout::kLegacy };
    std::mt19937 g_rng{ std::random_device{}() };
    Session g_session;
    lrb::BarRenderer g_renderer;

    template <class T>
    [[nodiscard]] T ReadShifted(const T& legacyField)
    {
        return *reinterpret_cast<const T*>(reinterpret_cast<const std::byte*>(&legacyField) + kShiftedDelta);
    }

    [[nodiscard]] lrb::RawLockValues ReadLock(const RE::LockpickingMenu::RUNTIME_DATA& rd)
    {
        if (g_layout == lrb::Layout::kShifted) {
            return { ReadShifted(rd.unk0F8), ReadShifted(rd.sweetSpotAngle), ReadShifted(rd.partialPickAngle) };
        }
        return { rd.unk0F8, rd.sweetSpotAngle, rd.partialPickAngle };
    }

    [[nodiscard]] std::uint32_t ReadBrokenPicks(const RE::LockpickingMenu::RUNTIME_DATA& rd)
    {
        return g_layout == lrb::Layout::kShifted ? ReadShifted(rd.numBrokenPicks) : rd.numBrokenPicks;
    }

    [[nodiscard]] float PlayerLockpickingSkill()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        return player ? player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLockpicking) : 0.0F;
    }

    void RollStyle(const char* reason)
    {
        g_session.style = lrb::RollStyle(g_config, g_rng());
        const auto& s = g_session.style;
        SKSE::log::info("Style rolled ({}): palette={} inverted={} bands={}", reason, lrb::PaletteName(s.palette), s.inverted, s.bands);
    }

    void BeginSession()
    {
        // A renderer still attached here belongs to a menu that never sent kHide; its movie may be gone.
        g_renderer.Abandon();
        g_session = Session{};
        g_session.lastTick = Clock::now();
    }

    void EndSession()
    {
        g_renderer.Detach();
        g_session = Session{};
        // AdvanceMovie can still run while the menu fades out; do not re-attach until the next kShow.
        g_session.closed = true;
    }

    void Tick(RE::LockpickingMenu* menu)
    {
        auto* movie = menu->uiMovie.get();
        if (!movie || g_session.closed || g_session.attachFailed) {
            return;
        }

        const auto now = Clock::now();
        const float dt = std::clamp(std::chrono::duration<float>(now - g_session.lastTick).count(), 0.0F, 0.1F);
        g_session.lastTick = now;

        const auto& rd = menu->GetRuntimeData();
        const auto raw = ReadLock(rd);

        if (g_config.debugLog && now - g_session.lastDebugLog > std::chrono::seconds(1)) {
            g_session.lastDebugLog = now;
            SKSE::log::info(
                "[debug] layout={} center={:.3f} width={:.3f} partial={:.3f} pick={:.3f} lock={:.3f} broken={} skill={:.1f} health={:.3f} plausible={}",
                lrb::LayoutName(g_layout), raw.center, raw.width, raw.partial, rd.pickAngle, rd.lockAngle,
                ReadBrokenPicks(rd), PlayerLockpickingSkill(), lrb::ReadPickHealth().value_or(-1.0F), lrb::IsPlausible(raw));
        }

        // Before the lock is rolled the fields are zero; wait instead of drawing garbage.
        if (!lrb::IsPlausible(raw)) {
            return;
        }

        const lrb::LockGeometry lock{ raw.center, raw.width, raw.partial };
        if (!g_session.haveLock || std::fabs(lock.center - g_session.lock.center) > 0.001F) {
            g_session.haveLock = true;
            g_session.brokenPicks = ReadBrokenPicks(rd);
            g_session.field.Reset(g_config.cells);
            SKSE::log::info("Lock ready: layout={} width={:.2f} partial={:.2f} skill={:.0f}",
                lrb::LayoutName(g_layout), lock.width, lock.partial, PlayerLockpickingSkill());
            RollStyle("new lock");
        }
        // Other mods may rescale the zones mid-session; always use the live values.
        g_session.lock = lock;

        const auto broken = ReadBrokenPicks(rd);
        if (broken != g_session.brokenPicks) {
            g_session.brokenPicks = broken;
            if (g_config.resetRevealOnPickBreak) {
                g_session.field.Reset(g_config.cells);
            }
            if (g_config.rerollOnPickBreak) {
                RollStyle("pick broke");
            }
        }

        if (!g_renderer.IsAttachedTo(movie)) {
            g_renderer.Abandon();
            if (!g_renderer.Attach(movie, g_config, g_config.cells)) {
                g_session.attachFailed = true;
                SKSE::log::error("Could not attach the reveal bar to the lockpicking menu");
                return;
            }
        }

        const bool turning = rd.lockAngle > 0.5F;
        const auto params = lrb::ComputeRevealParams(g_config, PlayerLockpickingSkill(), turning);
        g_session.field.Step(rd.pickAngle, params, dt, g_config.fadePerSecond);

        const int cells = g_session.field.Cells();
        for (int i = 0; i < cells; ++i) {
            const auto color = lrb::CellColor(
                g_config, g_session.style, g_session.lock,
                g_session.field.CellMin(i), g_session.field.CellMax(i), i, g_session.field.Get(i));
            g_renderer.SetCellColor(g_config.flipDirection ? cells - 1 - i : i, color);
        }

        const float fraction = (rd.pickAngle - lrb::kPickMin) / lrb::kPickRange;
        g_renderer.SetMarker(g_config.flipDirection ? 1.0F - fraction : fraction);

        const auto health = lrb::ReadPickHealth();
        g_renderer.SetHealth(health, health ? lrb::HealthColor(g_config, *health) : 0);
    }

    struct LockpickingMenuHooks
    {
        static RE::UI_MESSAGE_RESULTS ProcessMessage(RE::LockpickingMenu* a_this, RE::UIMessage& a_message)
        {
            switch (*a_message.type) {
            case RE::UI_MESSAGE_TYPE::kShow:
                BeginSession();
                break;
            case RE::UI_MESSAGE_TYPE::kHide:
            case RE::UI_MESSAGE_TYPE::kForceHide:
                EndSession();
                break;
            default:
                break;
            }
            return _ProcessMessage(a_this, a_message);
        }

        static void AdvanceMovie(RE::LockpickingMenu* a_this, float a_interval, std::uint32_t a_currentTime)
        {
            _AdvanceMovie(a_this, a_interval, a_currentTime);
            Tick(a_this);
        }

        static inline REL::Relocation<decltype(ProcessMessage)> _ProcessMessage;
        static inline REL::Relocation<decltype(AdvanceMovie)> _AdvanceMovie;
    };
}

namespace lrb
{
    void InstallLockpickHook(const Config& config)
    {
        g_config = config;

        const auto version = REL::Module::get().version();
        g_layout = ResolveLayout(config.layout, version.major(), version.minor());
        SKSE::log::info("Runtime {} -> sweet spot layout {}", version.string(), LayoutName(g_layout));

        if (config.showPickHealth) {
            ResolvePickHealth();
        }

        REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_LockpickingMenu[0] };
        LockpickingMenuHooks::_ProcessMessage = vtbl.write_vfunc(0x4, LockpickingMenuHooks::ProcessMessage);
        LockpickingMenuHooks::_AdvanceMovie = vtbl.write_vfunc(0x5, LockpickingMenuHooks::AdvanceMovie);
        SKSE::log::info("LockpickingMenu hooks installed");
    }
}
