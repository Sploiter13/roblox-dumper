#pragma once
#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/offset_registry.hpp"
#include "utils/structs.h"
#include <chrono>
#include <thread>

namespace scanner::phases {

    inline bool text_button(Context& ctx) {
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

        const auto text_button =
            screen_gui.find_first_child_of_class("Frame").find_first_child("TextButton");
        if (!text_button.is_valid()) {
            LOG_ERR("Failed to find TextButton in 'ReplicatedStorage.ScreenGui.TextButton'");
            return false;
        }

        const auto line_height_offset =
            memory->find_verified_offset_float({text_button.address}, {2.246f}, 0x1000, 0x4);

        if (!line_height_offset) {
            LOG_ERR("Failed to find LineHeight offset for TextButton");
            return false;
        }

        offset_registry.add("TextButton", "LineHeight", *line_height_offset);

        Vector2 current_text_bounds = memory->read<Vector2>(text_button.address + 0x200);
        for (size_t offset = 0; offset < 0x1000; offset += 0x4) {
            Vector2 value = memory->read<Vector2>(text_button.address + offset);
            if (value.y > 70.0f && value.y < 76.0f && value.x > 0.0f && value.x < 1000.0f) {
                current_text_bounds = value;
                break;
            }
        }

        std::vector<Vector2> text_bounds_values = {current_text_bounds};

        auto text_bounds_offsets = memory->find_offsets_with_snapshots<Vector2>(
            text_button.address, text_bounds_values, [&](size_t i) {}, 0x1000, 0x4, 100);

        std::optional<size_t> text_bounds_offset;
        if (!text_bounds_offsets.empty()) {
            text_bounds_offset = text_bounds_offsets[0];
            offset_registry.add("TextButton", "TextBounds", *text_bounds_offset);
        } else {
            LOG_ERR("Failed to find TextBounds offset for TextButton");
        }

        const auto text_transparency_offset =
            memory->find_verified_offset_float({text_button.address}, {0.456f}, 0x2000, 0x4);

        if (!text_transparency_offset) {
            LOG_ERR("Failed to find TextTransparency offset for TextButton");
            return false;
        }

        offset_registry.add("TextButton", "TextTransparency", *text_transparency_offset);

        constexpr float TEXT_COLOR_R = 155.0f / 255.0f;
        constexpr float TEXT_COLOR_G = 89.0f / 255.0f;
        constexpr float TEXT_COLOR_B = 182.0f / 255.0f;

        auto text_color_offset =
            memory->find_verified_offset_float({text_button.address}, {TEXT_COLOR_R}, 0x2000, 0x4);

        if (text_color_offset) {
            float g = memory->read<float>(text_button.address + *text_color_offset + 4);
            float b = memory->read<float>(text_button.address + *text_color_offset + 8);
            if (std::abs(g - TEXT_COLOR_G) < 0.01f && std::abs(b - TEXT_COLOR_B) < 0.01f) {
                offset_registry.add("TextButton", "TextColor3", *text_color_offset);
            } else {
                LOG_ERR("Failed to verify TextColor3 G/B channels for TextButton");
            }
        } else {
            LOG_ERR("Failed to find TextColor3 offset for TextButton");
        }

        constexpr float STROKE_COLOR_R = 243.0f / 255.0f;
        constexpr float STROKE_COLOR_G = 156.0f / 255.0f;
        constexpr float STROKE_COLOR_B = 18.0f / 255.0f;

        auto stroke_color_offset =
            memory->find_verified_offset_float({text_button.address}, {STROKE_COLOR_R}, 0x2000, 0x4);

        if (stroke_color_offset) {
            float g = memory->read<float>(text_button.address + *stroke_color_offset + 4);
            float b = memory->read<float>(text_button.address + *stroke_color_offset + 8);
            if (std::abs(g - STROKE_COLOR_G) < 0.01f && std::abs(b - STROKE_COLOR_B) < 0.01f) {
                offset_registry.add("TextButton", "TextStrokeColor3", *stroke_color_offset);
            } else {
                LOG_ERR("Failed to verify TextStrokeColor3 G/B channels for TextButton");
            }
        } else {
            LOG_ERR("Failed to find TextStrokeColor3 offset for TextButton");
        }

        {
            std::vector<size_t> exclude_offsets;
            if (text_bounds_offset) {
                exclude_offsets.push_back(*text_bounds_offset);
                exclude_offsets.push_back(*text_bounds_offset + 4);
            }

            auto text_size_offset = memory->find_value_offset<int32_t>(
                text_button.address, 73, 0x2000, 0x4, 0, exclude_offsets);

            if (text_size_offset) {
                offset_registry.add("TextButton", "TextSize", *text_size_offset);
            } else if (text_bounds_offset) {
                offset_registry.add("TextButton", "TextSize", *text_bounds_offset + 4);
            } else {
                LOG_ERR("Failed to find TextSize offset for TextButton");
            }
        }

        std::vector<size_t> text_offsets;
        for (size_t offset = 0; offset < 0x2000; offset += 0x8) {
            std::string read_str = memory->read_string(text_button.address + offset, 32);
            if (read_str == "jonah dumper") {
                text_offsets.push_back(offset);
            }
        }

        if (text_offsets.size() >= 2) {
            offset_registry.add("TextButton", "ContentText", text_offsets[0]);
            offset_registry.add("TextButton", "Text", text_offsets[1]);
        } else if (text_offsets.size() == 1) {
            offset_registry.add("TextButton", "ContentText", text_offsets[0]);
            LOG_ERR("Found only one string offset, expected Text and ContentText");
        } else {
            LOG_ERR("Failed to find Text/ContentText string offsets for TextButton");
        }

        return true;
    }

} // namespace scanner::phases