#include "memory/memory.h"
#include "scanner.hpp"
#include "utils/logger.hpp"

namespace scanner::phases {

    inline bool special_mesh(Context& ctx) {
        if (!ctx.workspace.is_valid()) {
            LOG_ERR("Workspace not valid");
            return false;
        }

        const auto special_mesh_1 =
            ctx.workspace.find_first_child("specialmesh1").find_first_child("Mesh");
        if (!special_mesh_1.is_valid()) {
            LOG_ERR("Failed to find 'specialmesh1' in Workspace");
            return false;
        }

        const auto special_mesh_2 =
            ctx.workspace.find_first_child("specialmesh2").find_first_child("Mesh");
        if (!special_mesh_2.is_valid()) {
            LOG_ERR("Failed to find 'specialmesh2' in Workspace");
            return false;
        }

        const auto mesh_id = memory->find_roblox_string_direct(
            special_mesh_1.address, "http://www.roblox.com/Asset/?id=9982590", 0x800, 0x8);

        if (!mesh_id) {
            LOG_ERR("Failed to get MeshId for SpecialMesh");
            return false;
        }

        offset_registry.add("SpecialMesh", "MeshId", *mesh_id);

        constexpr float SPECIALMESH1_SCALE_X = 11.2f;
        constexpr float SPECIALMESH1_SCALE_Y = 14.6f;
        constexpr float SPECIALMESH1_SCALE_Z = 4.5f;

        constexpr float SPECIALMESH2_SCALE_X = 14.5f;
        constexpr float SPECIALMESH2_SCALE_Y = 16.6f;
        constexpr float SPECIALMESH2_SCALE_Z = 3.2f;

        const auto scale_offset = memory->find_vector3_offset(
            {special_mesh_1.address, special_mesh_2.address},
            {{SPECIALMESH1_SCALE_X, SPECIALMESH1_SCALE_Y, SPECIALMESH1_SCALE_Z},
             {SPECIALMESH2_SCALE_X, SPECIALMESH2_SCALE_Y, SPECIALMESH2_SCALE_Z}},
            0x1000, 0.5f);

        if (!scale_offset) {
            LOG_ERR("Failed to get SpecialMesh Scale");
            return false;
        }

        offset_registry.add("SpecialMesh", "Scale", *scale_offset);

        return true;
    }
} // namespace scanner::phases