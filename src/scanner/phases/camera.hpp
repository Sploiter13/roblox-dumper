#pragma once
#include "control/control.hpp"
#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/offset_registry.hpp"
#include <chrono>
#include <cmath>
#include <thread>

namespace scanner::phases {

    inline bool camera(Context& ctx) {
        if (!ctx.workspace.is_valid()) {
            LOG_ERR("Workspace not valid");
            return false;
        }

        auto camera = ctx.workspace.find_first_child("Camera");
        if (!camera.is_valid()) {
            LOG_ERR("Failed to find Camera in Workspace");
            return false;
        }

        control::Controller controller("http://localhost:8000");
        constexpr int SLEEP_MS = 1000;

        // fov (stored in radians, displayed as degrees)
        {
            controller.set_camera_fov(70.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<float> fov_degrees = {70.0f, 30.0f};
            std::vector<float> fov_radians;
            for (float deg : fov_degrees) {
                fov_radians.push_back(deg * 3.14159265f / 180.0f);
            }

            auto offset = memory->find_offsets_with_snapshots<float>(
                camera.address, fov_radians,
                [&](size_t i) {
                    controller.set_camera_fov(fov_degrees[i]);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Camera", "FieldOfView", offset[0]);
                controller.set_camera_fov(70.0f);
            } else {
                LOG_ERR("Failed to find FieldOfView offset");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

        // position
        {
            controller.set_camera_position(0.0f, 50.0f, 0.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<float> y_values = {50.0f, 150.0f};
            auto offset = memory->find_offsets_with_snapshots<float>(
                camera.address, y_values,
                [&](size_t i) {
                    controller.set_camera_position(0.0f, y_values[i], 0.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Camera", "Position", offset[0] - 0x4);
                controller.set_camera_position(0.0f, 50.0f, 0.0f);
            } else {
                LOG_ERR("Failed to find Position offset");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

        // cframe/rot
        {
            controller.set_camera_rotation(0.0f, 0.0f, 0.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<float> rotation_values = {1.0f, 0.70710677f};
            auto offset = memory->find_offsets_with_snapshots<float>(
                camera.address, rotation_values,
                [&](size_t i) {
                    float angle = (i == 1) ? 45.0f : 0.0f;
                    controller.set_camera_rotation(angle, 0.0f, 0.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x600, 0x4);

            if (!offset.empty()) {
                offset_registry.add("Camera", "Rotation", offset[0] - 0x10);
                controller.set_camera_rotation(0.0f, 0.0f, 0.0f);
            } else {
                LOG_ERR("Failed to find CFrame/Rotation offset for Camera");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

        if (!ctx.visual_engine) {
            LOG_ERR("VisualEngine not available, skipping ViewMatrix");
            return true;
        }

        auto is_valid_matrix = [](const float mat[16]) -> bool {
            return std::abs(mat[14] + 1.0f) < 0.01f && !std::isnan(mat[15]) && !std::isinf(mat[15]);
        };

        std::vector<size_t> candidates;
        for (size_t offset = 0; offset < 0x2000; offset += 0x10) {
            float mat[16];
            for (int i = 0; i < 16; i++) {
                mat[i] = memory->read<float>(ctx.visual_engine + offset + (i * 4));
            }
            if (is_valid_matrix(mat)) {
                candidates.push_back(offset);
            }
        }

        if (!candidates.empty()) {
            controller.set_camera_position(0.0f, 50.0f, 0.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<float> y_snapshots;
            for (size_t i = 0; i < std::min(candidates.size(), size_t(5)); i++) {
                y_snapshots.push_back(memory->read<float>(ctx.visual_engine + candidates[i] + 28));
            }

            controller.set_camera_position(0.0f, 150.0f, 0.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            for (size_t i = 0; i < y_snapshots.size(); i++) {
                float y_new = memory->read<float>(ctx.visual_engine + candidates[i] + 28);
                if (std::abs(y_snapshots[i] - y_new) > 50.0f) {
                    offset_registry.add("VisualEngine", "ViewMatrix", candidates[i]);
                    break;
                }
            }
        }

        return true;
    }

} // namespace scanner::phases