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

    inline bool atmosphere(Context& ctx) {
        const auto lighting = sdk::instance_t(ctx.data_model).find_first_child("Lighting");
        if (!lighting.is_valid()) {
            LOG_ERR("Failed to find Lighting");
            return false;
        }

        const auto atmosphere_inst = lighting.find_first_child_of_class("Atmosphere");
        if (!atmosphere_inst.is_valid()) {
            LOG_ERR("Failed to find Atmosphere in 'Lighting'");
            return false;
        }

        control::Controller controller("http://localhost:8000");

        const auto density_offset =
            memory->find_verified_offset_float({atmosphere_inst.address}, {0.677f}, 0x1000, 0x2);

        if (!density_offset) {
            LOG_ERR("Failed to find Density offset");
            return false;
        }

        offset_registry.add("Atmosphere", "Density", *density_offset);

        // holy fuck nice var name
        const auto offset_offset =
            memory->find_verified_offset_float({atmosphere_inst.address}, {0.385f}, 0x1000, 0x2);

        if (!density_offset) {
            LOG_ERR("Failed to find Offset offset");
            return false;
        }

        offset_registry.add("Atmosphere", "Offset", *offset_offset);

        const auto glare_offset =
            memory->find_verified_offset_float({atmosphere_inst.address}, {5.13f}, 0x1000, 0x2);

        if (!glare_offset) {
            LOG_ERR("Failed to find Glare offset");
            return false;
        }

        offset_registry.add("Atmosphere", "Glare", *glare_offset);

        const auto haze_offset =
            memory->find_verified_offset_float({atmosphere_inst.address}, {7.78f}, 0x1000, 0x2);

        if (!haze_offset) {
            LOG_ERR("Failed to find Haze offset");
            return false;
        }

        offset_registry.add("Atmosphere", "Haze", *haze_offset);

        // atmosphere color
        {
            controller.set_atmosphere_color(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                atmosphere_inst.address, colors,
                [&](size_t i) {
                    controller.set_atmosphere_color(colors[i].r * 255.0f, colors[i].g * 255.0f,
                                                    colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Atmosphere", "Color", offset[0]);
                controller.set_atmosphere_decay(0, 0, 0);
            } else {
                LOG_ERR("Failed to find Atmosphere Color offset");
            }
        }

        // decay color
        {
            controller.set_atmosphere_decay(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                atmosphere_inst.address, colors,
                [&](size_t i) {
                    controller.set_atmosphere_decay(colors[i].r * 255.0f, colors[i].g * 255.0f,
                                                    colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Atmosphere", "Decay", offset[0]);
                controller.set_atmosphere_decay(0, 0, 0);
            } else {
                LOG_ERR("Failed to find Atmosphere Decay offset");
            }
        }

        return true;
    }

} // namespace scanner::phases