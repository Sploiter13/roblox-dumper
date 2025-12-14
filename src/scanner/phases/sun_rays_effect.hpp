#pragma once
#include "control/control.hpp"
#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/offset_registry.hpp"
#include <chrono>
#include <thread>

namespace scanner::phases {

    inline bool sun_rays_effect(Context& ctx) {
        const auto lighting = sdk::instance_t(ctx.data_model).find_first_child("Lighting");
        if (!lighting.is_valid()) {
            LOG_ERR("Failed to find Lighting");
            return false;
        }

        const auto sun_rays_effect_inst = lighting.find_first_child_of_class("SunRaysEffect");
        if (!sun_rays_effect_inst.is_valid()) {
            LOG_ERR("Failed to find SunRaysEffect in 'Lighting'");
            return false;
        }

        const auto intensity_offset = memory->find_verified_offset_float(
            {sun_rays_effect_inst.address}, {0.419f}, 0x400, 0x2);

        if (!intensity_offset) {
            LOG_ERR("Failed to find Intensity offset for SunRaysEffect");
            return false;
        }

        offset_registry.add("SunRaysEffect", "Intensity", *intensity_offset);

        const auto spread_offset = memory->find_verified_offset_float(
            {sun_rays_effect_inst.address}, {0.556f}, 0x400, 0x2);

        if (!spread_offset) {
            LOG_ERR("Failed to find Spread offset for SunRaysEffect");
            return false;
        }

        offset_registry.add("SunRaysEffect", "Spread", *intensity_offset);

        return true;
    }

} // namespace scanner::phases