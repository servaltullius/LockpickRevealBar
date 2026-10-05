#pragma once

namespace lrb
{
    // Short-lived bar animations, advanced once per frame. Times are in seconds.
    class BarEffects
    {
    public:
        static constexpr float kSweetFlashSeconds = 0.9F;
        static constexpr float kBreakFlashSeconds = 0.5F;
        static constexpr float kShakeSeconds = 0.35F;

        void TriggerSweetFlash() { _sweet = kSweetFlashSeconds; }
        void TriggerBreak(bool shake);

        void Advance(float dt);

        // Overlay alphas in 0..100 (AS2 _alpha), eased out.
        [[nodiscard]] float SweetFlashAlpha() const;
        [[nodiscard]] float BreakFlashAlpha() const;

        // Horizontal offset in units of `amplitude`, decaying oscillation; 0 when idle.
        [[nodiscard]] float ShakeOffset() const;

        [[nodiscard]] bool Active() const { return _sweet > 0.0F || _break > 0.0F || _shake > 0.0F; }

    private:
        float _sweet{ 0.0F };
        float _break{ 0.0F };
        float _shake{ 0.0F };
    };
}
