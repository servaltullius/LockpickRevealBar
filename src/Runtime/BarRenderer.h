#pragma once

#include "Core/Config.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace RE
{
    class GFxMovieView;
    class GFxValue;
}

namespace lrb
{
    // Draws the bar into the lockpicking menu's own Scaleform movie with the AS2 drawing API,
    // so it works with any lockpickingmenu.swf skin.
    class BarRenderer
    {
    public:
        BarRenderer();
        ~BarRenderer();

        BarRenderer(const BarRenderer&) = delete;
        BarRenderer& operator=(const BarRenderer&) = delete;

        bool Attach(RE::GFxMovieView* movie, const Config& config, int cells);

        // Must run while the movie is still alive (menu kHide), never at process exit.
        void Detach();

        // Forgets the clips without releasing them, for when the movie may already be destroyed.
        void Abandon();

        [[nodiscard]] bool IsAttachedTo(const RE::GFxMovieView* movie) const noexcept { return _movie != nullptr && _movie == movie; }

        // Redraws the cell only if its color changed since the last draw.
        void SetCellColor(int cell, std::uint32_t rgb);

        // `fraction` is 0 (left edge) .. 1 (right edge).
        void SetMarker(float fraction);

    private:
        struct Clips;

        void DrawRect(RE::GFxValue& clip, double x0, double y0, double x1, double y1, std::uint32_t rgb, double alpha);

        const RE::GFxMovieView* _movie{ nullptr };
        std::unique_ptr<Clips> _clips;
        std::vector<std::int64_t> _drawn;  // last drawn color per cell, -1 = never
        double _x0{ 0.0 };
        double _y0{ 0.0 };
        double _width{ 0.0 };
        double _height{ 0.0 };
        double _alpha{ 100.0 };
        double _lastMarkerX{ -1.0 };
    };
}
