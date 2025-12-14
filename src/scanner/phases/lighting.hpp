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

    inline bool lighting(Context& ctx) {
        auto lighting = sdk::instance_t(ctx.data_model).find_first_child("Lighting");
        if (!lighting.is_valid()) {
            LOG_ERR("Failed to find Lighting");
            return false;
        }

        control::Controller controller("http://localhost:8000");
        constexpr int SLEEP_MS = 500;

        auto sky = memory->find_rtti_offset(lighting.address, "Sky@RBX");
        if (!sky) {
            LOG_ERR("Failed to get Sky pointer inside Lighting");
            return false;
        }

        offset_registry.add("Lighting", "Sky", *sky);

        auto atmosphere = memory->find_rtti_offset(lighting.address, "Atmosphere@RBX");
        if (!atmosphere) {
            LOG_ERR("Failed to get Atmosphere pointer inside Lighting");
            return false;
        }

        offset_registry.add("Lighting", "Atmosphere", *atmosphere);

        const auto brightness_offset =
            memory->find_verified_offset_float({lighting.address}, {3.456f}, 0x400, 0x2);

        if (!brightness_offset) {
            LOG_ERR("Failed to find Brightness offset");
            return false;
        }

        offset_registry.add("Lighting", "Brightness", *brightness_offset);

        // note for future me if i forget. clock time is stored in microseconds
        const auto clock_time =
            memory->find_value_offset<uint64_t>(lighting.address, 21600000000, 0x400, 0x4); // 6am

        if (!clock_time) {
            LOG_ERR("Failed to find ClockTime offset");
            return false;
        }

        offset_registry.add("Lighting", "ClockTime", *clock_time);

        const auto environment_diffuse_scale_offset =
            memory->find_verified_offset_float({lighting.address}, {0.632f}, 0x400, 0x4);

        if (!environment_diffuse_scale_offset) {
            LOG_ERR("Failed to find EnvironmentDiffuseScale offset");
            return false;
        }

        offset_registry.add("Lighting", "EnvironmentDiffuseScale",
                            *environment_diffuse_scale_offset);

        const auto environment_specular_scale_offset =
            memory->find_verified_offset_float({lighting.address}, {0.521f}, 0x400, 0x4);

        if (!environment_specular_scale_offset) {
            LOG_ERR("Failed to find EnvironmentSpecularScale offset");
            return false;
        }

        offset_registry.add("Lighting", "EnvironmentSpecularScale",
                            *environment_specular_scale_offset);

        // Ambient
        {
            controller.set_ambient(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                lighting.address, colors,
                [&](size_t i) {
                    controller.set_ambient(colors[i].r * 255.0f, colors[i].g * 255.0f,
                                           colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Lighting", "Ambient", offset[0]);
                controller.set_ambient(0, 0, 0);
            } else {
                LOG_ERR("Failed to find Ambient offset");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

        // OutdoorAmbient
        {
            controller.set_outdoor_ambient(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                lighting.address, colors,
                [&](size_t i) {
                    controller.set_outdoor_ambient(colors[i].r * 255.0f, colors[i].g * 255.0f,
                                                   colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Lighting", "OutdoorAmbient", offset[0]);
                controller.set_outdoor_ambient(0, 0, 0);
            } else {
                LOG_ERR("Failed to find OutdoorAmbient offset");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

        // ColorShift_Top
        {
            controller.set_color_shift_top(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                lighting.address, colors,
                [&](size_t i) {
                    controller.set_color_shift_top(colors[i].r * 255.0f, colors[i].g * 255.0f,
                                                   colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Lighting", "ColorShift_Top", offset[0]);
                controller.set_color_shift_top(0, 0, 0);
            } else {
                LOG_ERR("Failed to find ColorShift_Top offset");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

        // ColorShift_Bottom
        {
            controller.set_color_shift_bottom(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                lighting.address, colors,
                [&](size_t i) {
                    controller.set_color_shift_bottom(colors[i].r * 255.0f, colors[i].g * 255.0f,
                                                      colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Lighting", "ColorShift_Bottom", offset[0]);
                controller.set_color_shift_bottom(0, 0, 0);
            } else {
                LOG_ERR("Failed to find ColorShift_Bottom offset");
            }
        }

        return true;
    }

} // namespace scanner::phases