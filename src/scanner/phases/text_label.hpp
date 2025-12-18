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

    inline bool text_label(Context& ctx) {
        const auto replicated_storage =
            sdk::instance_t(ctx.data_model).find_first_child("ReplicatedStorage");
        if (!replicated_storage.is_valid()) {
            LOG_ERR("Failed to find ReplicatedStorage");
            return false;
        }

        const auto screen_gui = replicated_storage.find_first_child_of_class("ScreenGui");
        if (!screen_gui.is_valid()) {
            LOG_ERR("Failed to find ScreenGui in 'ReplicatedStorage'");
            return false;
        }

        const auto text_label =
            screen_gui.find_first_child_of_class("Frame").find_first_child("TextLabel");
        if (!text_label.is_valid()) {
            LOG_ERR("Failed to find TextLabel in 'ReplicatedStorage.ScreenGui.TextLabel'");
            return false;
        }

        const auto line_height_offset =
            memory->find_verified_offset_float({text_label.address}, {2.246f}, 0x1000, 0x4);

        if (!line_height_offset) {
            LOG_ERR("Failed to find LineHeight offset for TextLabel");
            return false;
        }

        offset_registry.add("TextLabel", "LineHeight", *line_height_offset);

        const auto max_visible_graphemes_offset =
            memory->find_value_offset<uint32_t>(text_label.address, 4323, 0x1000, 0x4);

        if (!max_visible_graphemes_offset) {
            LOG_ERR("Failed to find MaxVisibleGraphemes offset for TextLabel");
            return false;
        }

        offset_registry.add("TextLabel", "MaxVisibleGraphemes", *max_visible_graphemes_offset);

        Vector2 current_text_bounds = memory->read<Vector2>(text_label.address + 0x200);
        for (size_t offset = 0; offset < 0x1000; offset += 0x4) {
            Vector2 value = memory->read<Vector2>(text_label.address + offset);
            if (value.y > 70.0f && value.y < 76.0f && value.x > 0.0f && value.x < 1000.0f) {
                current_text_bounds = value;
                break;
            }
        }

        std::vector<Vector2> text_bounds_values = {current_text_bounds};

        auto text_bounds_offsets = memory->find_offsets_with_snapshots<Vector2>(
            text_label.address, text_bounds_values, [&](size_t i) {}, 0x1000, 0x4, 100);

        std::optional<size_t> text_bounds_offset;
        if (!text_bounds_offsets.empty()) {
            text_bounds_offset = text_bounds_offsets[0];
            offset_registry.add("TextLabel", "TextBounds", *text_bounds_offset);
        } else {
            LOG_ERR("Failed to find TextBounds offset for TextLabel");
        }

        const auto text_stroke_transparency_offset =
            memory->find_verified_offset_float({text_label.address}, {0.952f}, 0x1000, 0x4);

        if (!text_stroke_transparency_offset) {
            LOG_ERR("Failed to find TextStrokeTransparency offset for TextLabel");
            return false;
        }

        offset_registry.add("TextLabel", "TextStrokeTransparency",
                            *text_stroke_transparency_offset);

        const auto text_transparency_offset =
            memory->find_verified_offset_float({text_label.address}, {0.456f}, 0x1000, 0x4);

        if (!text_transparency_offset) {
            LOG_ERR("Failed to find TextTransparency offset for TextLabel");
            return false;
        }

        offset_registry.add("TextLabel", "TextTransparency", *text_transparency_offset);

        control::Controller controller("http://localhost:8000");

        {
            controller.set_text_label_color(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                text_label.address, colors,
                [&](size_t i) {
                    controller.set_text_label_color(colors[i].r * 255.0f, colors[i].g * 255.0f,
                                                    colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                },
                0x1000, 0x4);

            if (!offset.empty()) {
                offset_registry.add("TextLabel", "TextColor3", offset[0]);
                controller.set_text_label_color(0, 0, 0);
            } else {
                LOG_ERR("Failed to find TextColor3 offset for TextLabel");
            }
        }

        {
            controller.set_text_label_stroke_color(0, 0, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::vector<RGB> colors = {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}};

            auto offset = memory->find_rgb_offsets_with_snapshots(
                text_label.address, colors,
                [&](size_t i) {
                    controller.set_text_label_stroke_color(
                        colors[i].r * 255.0f, colors[i].g * 255.0f, colors[i].b * 255.0f);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                },
                0x1000, 0x4);

            if (!offset.empty()) {
                offset_registry.add("TextLabel", "TextStrokeColor3", offset[0]);
                controller.set_text_label_stroke_color(0, 0, 0);
            } else {
                LOG_ERR("Failed to find TextStrokeColor3 offset for TextLabel");
            }
        }

        {
            std::vector<size_t> exclude_offsets;
            if (text_bounds_offset) {
                exclude_offsets.push_back(*text_bounds_offset);
                exclude_offsets.push_back(*text_bounds_offset + 4);
            }

            auto text_size_offset = memory->find_value_offset<int32_t>(
                text_label.address, 73, 0x1000, 0x4, 0, exclude_offsets);

            if (text_size_offset) {
                offset_registry.add("TextLabel", "TextSize", *text_size_offset);
            } else if (text_bounds_offset) {
                offset_registry.add("TextLabel", "TextSize", *text_bounds_offset + 4);
            } else {
                LOG_ERR("Failed to find TextSize offset for TextLabel");
            }
        }

        return true;
    }

} // namespace scanner::phases