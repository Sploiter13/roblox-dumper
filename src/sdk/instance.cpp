#include "instance.hpp"
#include "memory/memory.h"
#include "offsets.hpp"
#include "utils/logger.hpp"

namespace sdk {
    std::string instance_t::get_name() const {
        if (!is_valid())
            return "";
        auto name_ptr = memory->read<uintptr_t>(address + offsets::Instance::Name);
        if (!name_ptr)
            return "";
        return memory->read_roblox_string(name_ptr);
    }

    std::string instance_t::get_class_name() const {
        if (!is_valid())
            return "";

        uint64_t desc = memory->read<uint64_t>(address + offsets::Instance::ClassDescriptor);
        uint64_t name_ptr = memory->read<uint64_t>(desc + offsets::Instance::ClassName);
        if (!name_ptr)
            return "";

        return memory->read_roblox_string(name_ptr);
    }

    std::vector<instance_t> instance_t::get_children() const {
        std::vector<instance_t> children;
        if (!address)
            return children;

        const auto start = memory->read<uintptr_t>(address + offsets::Instance::ChildrenStart);
        if (!start)
            return children;

        const auto end = memory->read<uintptr_t>(start + offsets::Instance::ChildrenEnd);
        for (auto node = memory->read<uintptr_t>(start); node != end; node += 0x10) {
            if (const auto child_addr = memory->read<uintptr_t>(node))
                children.emplace_back(child_addr);
        }
        return children;
    }

    instance_t instance_t::find_first_child(const std::string& name) const {
        for (const auto& child : get_children()) {
            auto child_name = child.get_name();
            child_name.erase(0, child_name.find_first_not_of(' '));
            child_name.erase(child_name.find_last_not_of(' ') + 1);

            if (child_name == name)
                return child;
        }
        return {};
    }

    instance_t instance_t::find_first_child_of_class(const std::string& name) const {
        for (const auto& child : get_children()) {
            if (child.get_class_name() == name)
                return child;
        }
        return {};
    }

    instance_t instance_t::get_parent() const {
        if (!is_valid())
            return instance_t(0);
        const std::uint64_t parent_addr =
            memory->read<uintptr_t>(address + offsets::Instance::Parent);
        return instance_t(parent_addr);
    }

    std::size_t instance_t::get_children_count() const {
        if (!is_valid())
            return 0;

        const auto start = memory->read<uintptr_t>(address + offsets::Instance::ChildrenStart);
        if (!start)
            return 0;

        const auto end = memory->read<uintptr_t>(start + offsets::Instance::ChildrenEnd);

        std::size_t count = 0;
        for (auto node = memory->read<uintptr_t>(start); node != end; node += 0x10) {
            if (memory->read<uintptr_t>(node))
                count++;
        }

        return count;
    }
} // namespace sdk