#pragma once
#include "control/control.hpp"
#include "memory/memory.h"
#include "scanner.hpp"
#include "utils/offset_registry.hpp"
#include <chrono>
#include <thread>

namespace scanner::phases {

    inline bool workspace(Context& ctx) {
        control::Controller controller("http://localhost:8000");

        if (!ctx.workspace.is_valid()) {
            LOG_ERR("Workspace not valid");
            return false;
        }

        auto camera = memory->find_rtti_offset(ctx.workspace.address, "Camera@RBX");
        if (!camera) {
            LOG_ERR("Failed to find Camera offset");
            return false;
        }
        offset_registry.add("Workspace", "Camera", *camera);

        controller.set_gravity(196.2f);

        std::vector<float> gravity_values = {196.2f, 50.0f};

        auto gravity_offsets = memory->find_offsets_with_snapshots<float>(
            ctx.workspace.address, gravity_values,
            [&](size_t i) {
                controller.set_gravity(gravity_values[i]);
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
            },
            0x1000, 0x4, 2000);

        if (gravity_offsets.empty()) {
            LOG_ERR("Failed to find Gravity offset");
            return false;
        }

        for (size_t i = 0; i < gravity_offsets.size(); i++) {
            std::string name = (i == 0) ? "Gravity" : "Gravity" + std::to_string(i + 1);
            offset_registry.add("Workspace", name, gravity_offsets[i]);
        }

        controller.set_gravity(196.2f);

        return true;
    }

} // namespace scanner::phases