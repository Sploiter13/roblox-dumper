#pragma once
#include "control/control.hpp"
#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/offset_registry.hpp"
#include <chrono>
#include <thread>

namespace scanner::phases {

    inline bool depth_of_field_effect(Context& ctx) {
        const auto lighting = sdk::instance_t(ctx.data_model).find_first_child("Lighting");
        if (!lighting.is_valid()) {
            LOG_ERR("Failed to find Lighting");
            return false;
        }

        const auto depth_of_field = lighting.find_first_child_of_class("DepthOfFieldEffect");
        if (!depth_of_field.is_valid()) {
            LOG_ERR("Failed to find DepthOfField in 'Lighting'");
            return false;
        }

        const auto far_intensity_offset =
            memory->find_verified_offset_float({depth_of_field.address}, {0.138f}, 0x1000, 0x2);

        if (!far_intensity_offset) {
            LOG_ERR("Failed to find FarIntensity offset in DepthOfField");
            return false;
        }

        offset_registry.add("DepthOfFieldEffect", "Density", *far_intensity_offset);

        const auto focus_distance_offset =
            memory->find_verified_offset_float({depth_of_field.address}, {131.62f}, 0x1000, 0x2);

        if (!focus_distance_offset) {
            LOG_ERR("Failed to find FocusDistance offset in DepthOfField");
            return false;
        }

        offset_registry.add("DepthOfFieldEffect", "FocusDistance", *focus_distance_offset);

        const auto in_focus_radius_offset =
            memory->find_verified_offset_float({depth_of_field.address}, {21.58f}, 0x1000, 0x2);

        if (!in_focus_radius_offset) {
            LOG_ERR("Failed to find InFocusRadius offset in DepthOfField");
            return false;
        }

        offset_registry.add("DepthOfFieldEffect", "InFocusRadius", *in_focus_radius_offset);

        const auto near_intensity_offset =
            memory->find_verified_offset_float({depth_of_field.address}, {0.91f}, 0x1000, 0x2);

        if (!near_intensity_offset) {
            LOG_ERR("Failed to find NearIntensity offset in DepthOfField");
            return false;
        }

        offset_registry.add("DepthOfFieldEffect", "NearIntensity", *near_intensity_offset);

        return true;
    }

} // namespace scanner::phases