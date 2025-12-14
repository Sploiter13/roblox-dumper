#pragma once
#include "control/control.hpp"
#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/offset_registry.hpp"
#include <chrono>
#include <thread>

namespace scanner::phases {

    inline bool sky(Context& ctx) {
        auto lighting = sdk::instance_t(ctx.data_model).find_first_child("Lighting");
        if (!lighting.is_valid()) {
            LOG_ERR("Lighting not valid");
            return false;
        }

        auto sky = lighting.find_first_child_of_class("Sky");
        if (!sky.is_valid()) {
            LOG_ERR("Failed to find Sky");
            return false;
        }

        control::Controller controller("http://localhost:8000");
        constexpr int SLEEP_MS = 400;

        auto skybox_bk = memory->find_roblox_string_direct(
            sky.address, "http://www.roblox.com/asset/?id=144933338");
        if (!skybox_bk) {
            LOG_ERR("Failed to get SkyboxBk");
            return false;
        }

        offset_registry.add("Sky", "SkyboxBk", *skybox_bk);

        auto skybox_dn = memory->find_roblox_string_direct(
            sky.address, "http://www.roblox.com/asset/?id=144931530");
        if (!skybox_dn) {
            LOG_ERR("Failed to get SkyboxDn");
            return false;
        }

        offset_registry.add("Sky", "SkyboxDn", *skybox_dn);

        auto skybox_ft = memory->find_roblox_string_direct(
            sky.address, "http://www.roblox.com/asset/?id=144933262");
        if (!skybox_ft) {
            LOG_ERR("Failed to get SkyboxFt");
            return false;
        }

        offset_registry.add("Sky", "SkyboxFt", *skybox_ft);

        auto skybox_lf = memory->find_roblox_string_direct(
            sky.address, "http://www.roblox.com/asset/?id=144933244");
        if (!skybox_lf) {
            LOG_ERR("Failed to get SkyboxLf");
            return false;
        }

        offset_registry.add("Sky", "SkyboxLf", *skybox_lf);

        auto skybox_rt = memory->find_roblox_string_direct(
            sky.address, "http://www.roblox.com/asset/?id=144933299");
        if (!skybox_rt) {
            LOG_ERR("Failed to get SkyboxRt");
            return false;
        }

        offset_registry.add("Sky", "SkyboxRt", *skybox_rt);

        auto skybox_up = memory->find_roblox_string_direct(
            sky.address, "http://www.roblox.com/asset/?id=144931564");
        if (!skybox_up) {
            LOG_ERR("Failed to get SkyboxUp");
            return false;
        }

        offset_registry.add("Sky", "SkyboxUp", *skybox_up);

        auto moon_texture =
            memory->find_roblox_string_direct(sky.address, "rbxasset://sky/moon.jpg");
        if (!moon_texture) {
            LOG_ERR("Failed to get MoonTextureId");
            return false;
        }

        offset_registry.add("Sky", "MoonTextureId", *moon_texture);

        auto sun_texture = memory->find_roblox_string_direct(sky.address, "rbxasset://sky/sun.jpg");
        if (!sun_texture) {
            LOG_ERR("Failed to get SunTextureId");
            return false;
        }

        offset_registry.add("Sky", "SunTextureId", *sun_texture);

        const auto sun_angular_size_offset =
            memory->find_verified_offset_float({sky.address}, {27.89f}, 0x1000, 0x2);

        if (!sun_angular_size_offset) {
            LOG_ERR("Failed to find SunAngularSize offset in Sky");
            return false;
        }

        offset_registry.add("Sky", "SunAngularSize", *sun_angular_size_offset);

        const auto star_count_offset =
            memory->find_value_offset<int>(sky.address, 1337, 0x1000, 0x2);

        if (!star_count_offset) {
            LOG_ERR("Failed to find SunAngularSize offset in Sky");
            return false;
        }

        offset_registry.add("Sky", "StarCount", *star_count_offset);

        const auto moon_angular_size_offset =
            memory->find_verified_offset_float({sky.address}, {11.69f}, 0x1000, 0x2);

        if (!moon_angular_size_offset) {
            LOG_ERR("Failed to find MoonAngularSize offset in Sky");
            return false;
        }

        offset_registry.add("Sky", "MoonAngularSize", *moon_angular_size_offset);

        {
            controller.set_skybox_orientation(0.0f, 0.0f, 0.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<float> values = {0.0f, 90.0f};
            auto offset = memory->find_offsets_with_snapshots<float>(
                sky.address, values,
                [&](size_t i) {
                    controller.set_skybox_orientation(0.0f, values[i], 0.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Sky", "SkyboxOrientation", offset[0]);
                controller.set_skybox_orientation(0.0f, 0.0f, 0.0f);
            } else {
                LOG_ERR("Failed to find SkyboxOrientation offset");
            }
        }

        return true;
    }

} // namespace scanner::phases