#include "Core/PickHealthScan.h"

#include <cstring>

namespace
{
    // RIP-relative target of an instruction whose disp32 sits at `dispAt` and which ends at `end`.
    [[nodiscard]] std::ptrdiff_t RipTarget(std::span<const std::uint8_t> code, std::size_t dispAt, std::size_t end)
    {
        std::int32_t disp = 0;
        std::memcpy(&disp, code.data() + dispAt, sizeof(disp));
        return static_cast<std::ptrdiff_t>(end) + disp;
    }

    // mov dword ptr [rip+disp32], imm32  ->  C7 05 <disp32> <imm32>
    [[nodiscard]] bool IsMovImmToRip(std::span<const std::uint8_t> code, std::size_t i, std::uint32_t imm)
    {
        if (i + 10 > code.size() || code[i] != 0xC7 || code[i + 1] != 0x05) {
            return false;
        }
        std::uint32_t value = 0;
        std::memcpy(&value, code.data() + i + 6, sizeof(value));
        return value == imm;
    }

    // movss dword ptr [rip+disp32], xmm0-7  ->  F3 0F 11 <modrm 00 rrr 101> <disp32>
    [[nodiscard]] bool IsMovssStoreToRip(std::span<const std::uint8_t> code, std::size_t i)
    {
        return i + 8 <= code.size() && code[i] == 0xF3 && code[i + 1] == 0x0F && code[i + 2] == 0x11 && (code[i + 3] & 0xC7) == 0x05;
    }

    constexpr std::uint32_t kFloat100 = 0x42C80000;
    constexpr std::size_t   kStoreLookBack = 48;
    constexpr std::size_t   kZeroLookAhead = 40;
}

namespace lrb
{
    std::optional<std::ptrdiff_t> FindPickHealthGlobal(std::span<const std::uint8_t> code)
    {
        std::optional<std::ptrdiff_t> found;
        int matches = 0;

        for (std::size_t i = 0; i + 10 <= code.size(); ++i) {
            if (!IsMovImmToRip(code, i, kFloat100)) {
                continue;
            }
            const auto global = RipTarget(code, i + 2, i + 10);

            bool clampsToZero = false;
            for (std::size_t k = i + 10; k < i + 10 + kZeroLookAhead && !clampsToZero; ++k) {
                clampsToZero = IsMovImmToRip(code, k, 0) && RipTarget(code, k + 2, k + 10) == global;
            }

            bool storesResult = false;
            for (std::size_t k = i > kStoreLookBack ? i - kStoreLookBack : 0; k < i && !storesResult; ++k) {
                storesResult = IsMovssStoreToRip(code, k) && RipTarget(code, k + 4, k + 8) == global;
            }

            if (clampsToZero && storesResult) {
                found = global;
                ++matches;
            }
        }

        return matches == 1 ? found : std::nullopt;
    }
}
