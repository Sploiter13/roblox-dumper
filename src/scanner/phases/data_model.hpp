#pragma once
#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "settings.hpp"
#include "utils/offset_registry.hpp"

namespace scanner::phases {

    inline bool data_model(Context& ctx) {
        auto fake_dm_offset = memory->find_rtti_offset(ctx.visual_engine, "DataModel@RBX");
        if (!fake_dm_offset) {
            LOG_ERR("Failed to find FakeDataModel offset");
            return false;
        }
        offset_registry.add("VisualEngine", "ToFakeDataModel", *fake_dm_offset);

        auto fake_dm = memory->read<uintptr_t>(ctx.visual_engine + *fake_dm_offset);
        if (!fake_dm) {
            LOG_ERR("Failed to read FakeDataModel pointer");
            return false;
        }

        auto real_dm_offset = memory->find_rtti_offset(fake_dm, "DataModel@RBX");
        if (!real_dm_offset) {
            LOG_ERR("Failed to find RealDataModel offset");
            return false;
        }
        offset_registry.add("VisualEngine", "FakeToRealDataModel", *real_dm_offset);

        ctx.data_model = memory->read<uintptr_t>(fake_dm + *real_dm_offset);
        if (!ctx.data_model) {
            LOG_ERR("Failed to read DataModel pointer");
            return false;
        }

        auto workspace_offset = memory->find_rtti_offset(ctx.data_model, "Workspace@RBX");
        if (!workspace_offset) {
            LOG_ERR("Failed to find Workspace offset");
            return false;
        }
        offset_registry.add("DataModel", "Workspace", *workspace_offset);

        uintptr_t workspace_addr = memory->read<uintptr_t>(ctx.data_model + *workspace_offset);
        if (!workspace_addr) {
            LOG_ERR("Failed to read Workspace pointer");
            return false;
        }
        ctx.workspace = sdk::instance_t(workspace_addr);

        auto game_id = memory->find_value_offset<uint64_t>(ctx.data_model, settings::game_id());
        if (!game_id) {
            LOG_ERR("Failed to find GameId offset");
            return false;
        }
        offset_registry.add("DataModel", "GameId", *game_id);

        auto place_id = memory->find_value_offset<uint64_t>(ctx.data_model, settings::place_id());
        if (!place_id) {
            LOG_ERR("Failed to find PlaceId offset");
            return false;
        }
        offset_registry.add("DataModel", "PlaceId", *place_id);

        auto creator_id = memory->find_value_offset<uint64_t>(ctx.data_model, settings::creator_id());
        if (!creator_id) {
            LOG_ERR("Failed to find CreatorId offset");
            return false;
        }
        offset_registry.add("DataModel", "CreatorId", *creator_id);

        auto run_service = memory->find_rtti_offset(ctx.data_model, "RunService@RBX");
        if (!run_service) {
            LOG_ERR("Failed to find RunService offset");
            return false;
        }
        offset_registry.add("DataModel", "RunService", *run_service);

        auto user_input = memory->find_rtti_offset(ctx.data_model, "UserInputService@RBX");
        if (!user_input) {
            LOG_ERR("Failed to find UserInputService offset");
            return false;
        }
        offset_registry.add("DataModel", "UserInputService", *user_input);

        auto replicator = memory->find_rtti_offset(ctx.data_model, "ClientReplicator@Network@RBX");
        if (!replicator) {
            LOG_ERR("Failed to find ClientReplicator offset");
            return false;
        }
        offset_registry.add("DataModel", "ClientReplicator", *replicator);

        return true;
    }

} // namespace scanner::phases