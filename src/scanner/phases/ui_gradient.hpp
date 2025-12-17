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

    inline bool ui_gradient(Context& ctx) {
        const auto replicated_storage =
            sdk::instance_t(ctx.data_model).find_first_child("ReplicatedStorage");
        if (!replicated_storage.is_valid()) {
            LOG_ERR("Failed to find ReplicatedStorage");
            return false;
        }

        const auto ui_gradient = replicated_storage.find_first_child_of_class("UIGradient");
        if (!ui_gradient.is_valid()) {
            LOG_ERR("Failed to find UIGradient in 'ReplicatedStorage'");
            return false;
        }

        control::Controller controller("http://localhost:8000");

        {
            controller.set_ui_gradient_color(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                ui_gradient.address, colors,
                [&](size_t i) {
                    controller.set_ui_gradient_color(colors[i].r * 255.0f, colors[i].g * 255.0f,
                                                     colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("UIGradient", "Color", offset[0]);
                controller.set_ui_gradient_color(0, 0, 0);
            } else {
                LOG_ERR("Failed to find Color offset for UIGradient");
            }
        }

        const auto offset_offset =
            memory->find_verified_offset_float({ui_gradient.address}, {102.45f}, 0x400, 0x2);

        if (!offset_offset) {
            LOG_ERR("Failed to find offset offset for UIGradient");
            return false;
        }

        offset_registry.add("UIGradient", "Offset", *offset_offset);

        const auto rotation_offset =
            memory->find_verified_offset_float({ui_gradient.address}, {67.67f}, 0x400, 0x2);

        if (!rotation_offset) {
            LOG_ERR("Failed to find Rotation offset for UIGradient");
            return false;
        }

        offset_registry.add("UIGradient", "Rotation", *rotation_offset);

        const auto transparency_offset =
            memory->find_verified_offset_float({ui_gradient.address}, {42.56f}, 0x400, 0x2);

        if (!transparency_offset) {
            LOG_ERR("Failed to find Transparency offset for UIGradient");
            return false;
        }

        offset_registry.add("UIGradient", "Transparency", *transparency_offset);

        return true;
    }

} // namespace scanner::phases