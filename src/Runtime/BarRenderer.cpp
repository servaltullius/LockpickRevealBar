#include "Runtime/BarRenderer.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace
{
    // Far above anything the vanilla or skinned lockpicking SWF uses.
    constexpr double kContainerDepth = 1048000.0;
    constexpr const char* kContainerName = "LockpickRevealBar_mc";
}

namespace lrb
{
    struct BarRenderer::Clips
    {
        RE::GFxValue container;
        RE::GFxValue background;
        RE::GFxValue marker;
        std::vector<RE::GFxValue> cells;
    };

    BarRenderer::BarRenderer() = default;

    // A still-attached renderer is only destroyed at process exit, when the movie may already be gone.
    BarRenderer::~BarRenderer()
    {
        Abandon();
    }

    void BarRenderer::Abandon()
    {
        // Releasing Scaleform references of a dead movie would touch freed memory, so leak them on purpose.
        static_cast<void>(_clips.release());
        _drawn.clear();
        _movie = nullptr;
    }

    bool BarRenderer::Attach(RE::GFxMovieView* movie, const Config& config, int cells)
    {
        Detach();
        if (!movie || cells <= 0) {
            return false;
        }

        RE::GFxValue root;
        if (!movie->GetVariable(&root, "_root") || !root.IsDisplayObject()) {
            SKSE::log::warn("Lockpicking movie has no _root display object");
            return false;
        }

        auto clips = std::make_unique<Clips>();
        {
            const std::array<RE::GFxValue, 2> args{ RE::GFxValue(kContainerName), RE::GFxValue(kContainerDepth) };
            if (!root.Invoke("createEmptyMovieClip", &clips->container, args) || !clips->container.IsDisplayObject()) {
                SKSE::log::warn("createEmptyMovieClip failed on the lockpicking movie");
                return false;
            }
        }

        auto createChild = [&](const std::string& name, double depth, RE::GFxValue& out) {
            const std::array<RE::GFxValue, 2> args{ RE::GFxValue(std::string_view(name)), RE::GFxValue(depth) };
            return clips->container.Invoke("createEmptyMovieClip", &out, args) && out.IsDisplayObject();
        };

        bool created = createChild("bg", 0.0, clips->background);
        clips->cells.resize(static_cast<std::size_t>(cells));
        for (int i = 0; created && i < cells; ++i) {
            created = createChild("c" + std::to_string(i), static_cast<double>(i + 1), clips->cells[static_cast<std::size_t>(i)]);
        }
        created = created && createChild("marker", static_cast<double>(cells + 10), clips->marker);
        if (!created) {
            SKSE::log::warn("Failed to create bar child clips");
            clips->container.Invoke("removeMovieClip");
            return false;
        }

        const auto rect = movie->GetVisibleFrameRect();
        const double visibleW = static_cast<double>(rect.right - rect.left);
        const double visibleH = static_cast<double>(rect.bottom - rect.top);
        _width = std::max(visibleW * config.widthPct, 1.0);
        _height = std::max(visibleH * config.heightPct, 1.0);
        _x0 = rect.left + (visibleW - _width) * 0.5;
        _y0 = rect.top + visibleH * config.topPct;
        _alpha = config.alpha;

        // Background with a thin frame; cells start black, i.e. "nothing known yet".
        const double pad = std::max(_height * 0.12, 1.0);
        {
            auto& bg = clips->background;
            const std::array<RE::GFxValue, 3> line{ RE::GFxValue(1.0), RE::GFxValue(static_cast<double>(config.borderColor)), RE::GFxValue(_alpha) };
            bg.Invoke("lineStyle", line);
            DrawRect(bg, _x0 - pad, _y0 - pad, _x0 + _width + pad, _y0 + _height + pad, 0x000000, _alpha);
        }

        if (config.showPickMarker) {
            const double markerW = std::max(_height * 0.18, 1.5);
            DrawRect(clips->marker, -markerW * 0.5, _y0 - pad * 2.5, markerW * 0.5, _y0 + _height + pad * 2.5, config.markerColor, 100.0);
        } else {
            RE::GFxValue::DisplayInfo info;
            info.SetVisible(false);
            clips->marker.SetDisplayInfo(info);
        }

        _clips = std::move(clips);
        _drawn.assign(static_cast<std::size_t>(cells), -1);
        _lastMarkerX = -1.0;
        _movie = movie;

        for (int i = 0; i < cells; ++i) {
            SetCellColor(i, 0x000000);
        }
        return true;
    }

    void BarRenderer::Detach()
    {
        if (_clips && _clips->container.IsDisplayObject()) {
            _clips->container.Invoke("removeMovieClip");
        }
        _clips.reset();
        _drawn.clear();
        _movie = nullptr;
    }

    void BarRenderer::SetCellColor(int cell, std::uint32_t rgb)
    {
        if (!_clips || cell < 0 || cell >= static_cast<int>(_drawn.size())) {
            return;
        }
        auto& drawn = _drawn[static_cast<std::size_t>(cell)];
        if (drawn == static_cast<std::int64_t>(rgb)) {
            return;
        }
        drawn = rgb;

        const double cellW = _width / static_cast<double>(_drawn.size());
        const double x0 = _x0 + cellW * cell;
        // Slight overlap hides hairline seams between neighbouring cells.
        const double x1 = std::min(x0 + cellW + 0.35, _x0 + _width);

        auto& clip = _clips->cells[static_cast<std::size_t>(cell)];
        clip.Invoke("clear");
        DrawRect(clip, x0, _y0, x1, _y0 + _height, rgb, _alpha);
    }

    void BarRenderer::SetMarker(float fraction)
    {
        if (!_clips) {
            return;
        }
        const double x = _x0 + _width * std::clamp(static_cast<double>(fraction), 0.0, 1.0);
        if (std::fabs(x - _lastMarkerX) < 0.05) {
            return;
        }
        _lastMarkerX = x;

        RE::GFxValue::DisplayInfo info;
        info.SetX(x);
        _clips->marker.SetDisplayInfo(info);
    }

    void BarRenderer::DrawRect(RE::GFxValue& clip, double x0, double y0, double x1, double y1, std::uint32_t rgb, double alpha)
    {
        const std::array<RE::GFxValue, 2> fill{ RE::GFxValue(static_cast<double>(rgb)), RE::GFxValue(alpha) };
        clip.Invoke("beginFill", fill);
        clip.Invoke("moveTo", std::array<RE::GFxValue, 2>{ RE::GFxValue(x0), RE::GFxValue(y0) });
        clip.Invoke("lineTo", std::array<RE::GFxValue, 2>{ RE::GFxValue(x1), RE::GFxValue(y0) });
        clip.Invoke("lineTo", std::array<RE::GFxValue, 2>{ RE::GFxValue(x1), RE::GFxValue(y1) });
        clip.Invoke("lineTo", std::array<RE::GFxValue, 2>{ RE::GFxValue(x0), RE::GFxValue(y1) });
        clip.Invoke("lineTo", std::array<RE::GFxValue, 2>{ RE::GFxValue(x0), RE::GFxValue(y0) });
        clip.Invoke("endFill");
    }
}
