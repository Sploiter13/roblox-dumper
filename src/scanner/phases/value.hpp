#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/logger.hpp"

namespace scanner::phases {
    inline bool value(Context& ctx) {
        if (!ctx.data_model) {
            LOG_ERR("DataModel not valid");
            return false;
        }

        const auto replicated_storage =
            sdk::instance_t(ctx.data_model).find_first_child_of_class("ReplicatedStorage");

        if (!replicated_storage.is_valid()) {
            LOG_ERR("Could not find 'ReplicatedStorage' inside data model.");
            return false;
        }

        const auto string_value = replicated_storage.find_first_child("Value");

        const auto value_offset =
            memory->find_roblox_string_direct(string_value.address, "hello world", 0x800, 0x8);

        if (!value_offset) {
            LOG_ERR("Failed to get Value offset");
            return false;
        }

        offset_registry.add("Value", "Value", *value_offset);

        return true;
    }

} // namespace scanner::phases