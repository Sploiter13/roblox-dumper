#include "memory/memory.h"
#include "scanner.hpp"
#include "utils/logger.hpp"

namespace scanner::phases {

    inline bool mesh(Context& ctx) {
        if (!ctx.workspace.is_valid()) {
            LOG_ERR("Workspace not valid");
            return false;
        }

        const auto small_rock = ctx.workspace.find_first_child("SmallRock");
        if (!small_rock.is_valid()) {
            LOG_ERR("Failed to find 'SmallRock' in Workspace");
            return false;
        }

        const auto mesh_id = memory->find_roblox_string_direct(
            small_rock.address, "rbxassetid://847869656", 0x800, 0x8);

        if (!mesh_id) {
            LOG_ERR("Failed to get MeshId");
            return false;
        }

        offset_registry.add("Mesh", "MeshId", *mesh_id);

        const auto texture_id = memory->find_roblox_string_direct(
            small_rock.address, "rbxassetid://847870798", 0x800, 0x8);

        if (!texture_id) {
            LOG_ERR("Failed to get TextureId");
            return false;
        }

        offset_registry.add("Mesh", "TextureId", *texture_id);

        return true;
    }
} // namespace scanner::phases