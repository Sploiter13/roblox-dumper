#pragma once
#include "control/control.hpp"
#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/offset_registry.hpp"
#include "utils/structs.h"
#include <chrono>
#include <thread>


namespace scanner::phases {

    inline bool color_correction_effect(Context& ctx) {
        const auto lighting = sdk::instance_t(ctx.data_model).find_first_child("Lighting");
        if (!lighting.is_valid()) {
            LOG_ERR("Failed to find Lighting");
            return false;
        }

        const auto color_correct_inst = lighting.find_first_child_of_class("ColorCorrectionEffect");
        if (!lighting.is_valid()) {
            LOG_ERR("Failed to find ColorCorrectionEffect in 'Lighting'");
            return false;
        }

        control::Controller controller("http://localhost:8000");

        const auto brightness_offset =
            memory->find_verified_offset_float({color_correct_inst.address}, {-0.67f}, 0x200, 0x2);

        if (!brightness_offset) {
            LOG_ERR("Failed to find Brightness offset for ColorCorrectEffect");
            return false;
        }

        offset_registry.add("ColorCorrectionEffect", "Brightness", *brightness_offset);

        const auto contrast_offset =
            memory->find_verified_offset_float({color_correct_inst.address}, {1.123f}, 0x200, 0x2);

        if (!contrast_offset) {
            LOG_ERR("Failed to find Contrast offset for ColorCorrectEffect");
            return false;
        }

        offset_registry.add("ColorCorrectionEffect", "Contrast", *contrast_offset);

        const auto saturation_offset =
            memory->find_verified_offset_float({color_correct_inst.address}, {0.269f}, 0x200, 0x2);

        if (!saturation_offset) {
            LOG_ERR("Failed to find Saturation offset for ColorCorrectEffect");
            return false;
        }

        offset_registry.add("ColorCorrectionEffect", "Saturation", *saturation_offset);

        // tint color
        {
            controller.set_color_correction_tint(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                color_correct_inst.address, colors,
                [&](size_t i) {
                    controller.set_color_correction_tint(colors[i].r * 255.0f, colors[i].g * 255.0f,
                                                         colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("ColorCorrectionEffect", "TintColor", offset[0]);
                controller.set_atmosphere_decay(0, 0, 0);
            } else {
                LOG_ERR("Failed to find TintColor offset for ColorCorrectionEffect");
            }
        }

        return true;
    }

} // namespace scanner::phases