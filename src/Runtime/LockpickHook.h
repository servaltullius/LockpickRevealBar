#pragma once

#include "Core/Config.h"

namespace lrb
{
    // Patches LockpickingMenu's ProcessMessage and AdvanceMovie vfuncs.
    void InstallLockpickHook(const Config& config);
}
