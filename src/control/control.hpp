#pragma once
#include <chrono>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <thread>

namespace control {
    struct CommandResult {
        std::string status;
        nlohmann::json result;
    };

    class Controller {
      public:
        Controller(const std::string& api_url = "http://localhost:8000");

        std::string send_command(const std::string& action, const nlohmann::json& data);
        std::optional<CommandResult> get_status(const std::string& command_id);
        std::optional<CommandResult> wait_for_completion(const std::string& command_id,
                                                         int timeout_ms = 5000,
                                                         int poll_interval_ms = 50);

        bool set_gravity(float value);
        bool set_ambient(float r, float g, float b);
        bool set_outdoor_ambient(float r, float g, float b);
        bool set_color_shift_top(float r, float g, float b);
        bool set_color_shift_bottom(float r, float g, float b);
        bool set_skybox_orientation(float x, float y, float z);
        bool set_npc_move_to(bool enabled);
        bool set_camera_fov(float value);
        bool set_camera_position(float x, float y, float z);
        bool set_camera_rotation(float x, float y, float z);
        bool set_team(const std::string& team);
        bool set_atmosphere_color(float r, float g, float b);
        bool set_atmosphere_decay(float r, float g, float b);
        bool set_color_correction_tint(float r, float g, float b);
        bool set_ui_gradient_color(float r, float g, float b);

        void set_api_url(const std::string& url);
        bool clear_queue();

      private:
        std::string api_url;
        bool execute_command(const std::string& action, const nlohmann::json& data);
    };
} // namespace control