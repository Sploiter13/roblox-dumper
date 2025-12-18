#pragma once
#include "control/control.hpp"
#include "memory/memory.h"
#include "scanner.hpp"
#include "utils/offset_registry.hpp"
#include <chrono>
#include <thread>

namespace scanner::phases {

    inline bool tool(Context& ctx) {
        const auto replicated_storage =
            sdk::instance_t(ctx.data_model).find_first_child("ReplicatedStorage");
        if (!replicated_storage.is_valid()) {
            LOG_ERR("Failed to find ReplicatedStorage");
            return false;
        }

        auto tool1 = replicated_storage.find_first_child("Tool1");
        if (!tool1.is_valid()) {
            LOG_ERR("Failed to find 'Tool1' in ReplicatedStorage");
            return false;
        }

        control::Controller controller("http://localhost:8000");
        constexpr int SLEEP_MS = 200;

        {
            controller.set_tool_can_be_dropped(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<uint8_t> values = {1, 0, 1};
            auto offset = memory->find_offsets_with_snapshots<uint8_t>(
                tool1.address, values,
                [&](size_t i) {
                    controller.set_tool_can_be_dropped(values[i] == 1);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x800, 0x1);

            if (!offset.empty()) {
                offset_registry.add("Tool", "CanBeDropped", offset[0]);
            } else {
                LOG_ERR("Failed to find CanBeDropped offset");
            }
        }

        {
            controller.set_tool_enabled(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<uint8_t> values = {1, 0, 1};
            auto offset = memory->find_offsets_with_snapshots<uint8_t>(
                tool1.address, values,
                [&](size_t i) {
                    controller.set_tool_enabled(values[i] == 1);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x800, 0x1);

            if (!offset.empty()) {
                offset_registry.add("Tool", "Enabled", offset[0]);
            } else {
                LOG_ERR("Failed to find Enabled offset");
            }
        }

        {
            controller.set_tool_manual_activation(false);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<uint8_t> values = {0, 1, 0};
            auto offset = memory->find_offsets_with_snapshots<uint8_t>(
                tool1.address, values,
                [&](size_t i) {
                    controller.set_tool_manual_activation(values[i] == 1);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x800, 0x1);

            if (!offset.empty()) {
                offset_registry.add("Tool", "ManualActivationOnly", offset[0]);
            } else {
                LOG_ERR("Failed to find ManualActivationOnly offset");
            }
        }

        {
            controller.set_tool_requires_handle(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            std::vector<uint8_t> values = {1, 0, 1};
            auto offset = memory->find_offsets_with_snapshots<uint8_t>(
                tool1.address, values,
                [&](size_t i) {
                    controller.set_tool_requires_handle(values[i] == 1);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x800, 0x1);

            if (!offset.empty()) {
                offset_registry.add("Tool", "RequiresHandle", offset[0]);
            } else {
                LOG_ERR("Failed to find RequiresHandle offset");
            }
        }

        {
            std::vector<Vector3> grip_positions = {
                {10.0f, 20.0f, 30.0f},
                {-15.0f, 25.0f, -35.0f},
                {5.0f, 15.0f, 25.0f},
            };

            controller.set_tool_grip_pos(grip_positions[0].x, grip_positions[0].y,
                                         grip_positions[0].z);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));

            auto offsets = memory->find_offsets_with_snapshots<Vector3>(
                tool1.address, grip_positions,
                [&](size_t i) {
                    controller.set_tool_grip_pos(grip_positions[i].x, grip_positions[i].y,
                                                 grip_positions[i].z);
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
                },
                0x800, 0x4);

            if (!offsets.empty()) {
                offset_registry.add("Tool", "Grip", offsets[0] - 0x24);
                offset_registry.add("Tool", "GripPos", offsets[0]);
                offset_registry.add("Tool", "GripRight", offsets[0] - 0x24);
                offset_registry.add("Tool", "GripUp", offsets[0] - 0x18);
                offset_registry.add("Tool", "GripForward", offsets[0] - 0xC);
            } else {
                LOG_ERR("Failed to find Grip offset");
            }
        }

        for (size_t offset = 0; offset < 0x800; offset += 0x8) {
            std::string str = memory->read_string(tool1.address + offset, 32);
            if (str == "jonah") {
                offset_registry.add("Tool", "ToolTip", offset);
                break;
            }
        }

        return true;
    }

} // namespace scanner::phases
