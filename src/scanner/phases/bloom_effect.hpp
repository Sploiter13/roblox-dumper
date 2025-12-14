#pragma once
#include "control/control.hpp"
#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/offset_registry.hpp"
#include <chrono>
#include <thread>

namespace scanner::phases {

    inline bool bloom_effect(Context& ctx) {
        const auto lighting = sdk::instance_t(ctx.data_model).find_first_child("Lighting");
        if (!lighting.is_valid()) {
            LOG_ERR("Failed to find Lighting");
            return false;
        }

        const auto bloom_effect_inst = lighting.find_first_child_of_class("BloomEffect");
        if (!lighting.is_valid()) {
            LOG_ERR("Failed to find BloomEffect in 'Lighting'");
            return false;
        }

        const auto intensity_offset =
            memory->find_verified_offset_float({bloom_effect_inst.address}, {0.6f}, 0x400, 0x2);

        if (!intensity_offset) {
            LOG_ERR("Failed to find Intensity offset for BloomEffect");
            return false;
        }

        offset_registry.add("BloomEffect", "Intensity", *intensity_offset);

        const auto size_offset =
            memory->find_verified_offset_float({bloom_effect_inst.address}, {33.0f}, 0x400, 0x2);

        if (!size_offset) {
            LOG_ERR("Failed to find Size offset for BloomEffect");
            return false;
        }

        offset_registry.add("BloomEffect", "Size", *size_offset);

        const auto threshold_offset =
            memory->find_verified_offset_float({bloom_effect_inst.address}, {2.947f}, 0x400, 0x2);

        if (!threshold_offset) {
            LOG_ERR("Failed to find Threshold offset for BloomEffect");
            return false;
        }

        offset_registry.add("BloomEffect", "Threshold", *threshold_offset);

        return true;
    }

} // namespace scanner::phases