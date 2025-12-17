#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/logger.hpp"

namespace scanner::phases {
    inline bool team(Context& ctx) {
        if (!ctx.data_model) {
            LOG_ERR("DataModel not valid");
            return false;
        }

        const auto teams = sdk::instance_t(ctx.data_model).find_first_child_of_class("Teams");

        if (!teams.is_valid()) {
            LOG_ERR("Could not find 'Teams' inside data model.");
            return false;
        }

        const auto blue_team = teams.find_first_child("Blue Team");

        const auto team_color_offset =
            memory->find_value_offset<uint32_t>(blue_team.address, 23, 0x200, 0x4);

        if (!team_color_offset) {
            LOG_ERR("Failed to get Team Color offset");
            return false;
        }

        offset_registry.add("Team", "Color", *team_color_offset);

        return true;
    }

} // namespace scanner::phases