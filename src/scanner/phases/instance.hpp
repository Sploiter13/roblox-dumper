#pragma once
#include "memory/memory.h"
#include "scanner.hpp"
#include "sdk/instance.hpp"
#include "utils/offset_registry.hpp"

namespace scanner::phases {

    std::optional<std::pair<size_t, size_t>> find_children_offsets(uintptr_t instance,
                                                                   size_t expected_count) {
        uintptr_t parent_offset = sdk::offsets::Instance::Parent;

        for (size_t start_off = 0; start_off < 0x200; start_off += 0x8) {
            if (start_off == parent_offset)
                continue;

            uintptr_t start_ptr = memory->read<uintptr_t>(instance + start_off);
            if (!start_ptr || start_ptr < 0x10000)
                continue;

            for (size_t end_off = 0; end_off < 0x20; end_off += 0x8) {
                uintptr_t end_ptr = memory->read<uintptr_t>(start_ptr + end_off);
                if (!end_ptr || end_ptr < 0x10000)
                    continue;

                size_t count = 0;
                uintptr_t node = memory->read<uintptr_t>(start_ptr);
                for (int i = 0; i < 1000 && node != end_ptr; i++, node += 0x10) {
                    uintptr_t child = memory->read<uintptr_t>(node);
                    if (!child || child < 0x10000)
                        break;
                    uintptr_t vtable = memory->read<uintptr_t>(child);
                    if (!vtable || vtable < 0x10000)
                        break;
                    count++;
                }

                if (count == expected_count) {
                    return std::make_pair(start_off, end_off);
                }
            }
        }
        return std::nullopt;
    }

    std::optional<std::tuple<size_t, size_t, size_t, size_t>>
    find_attribute_offsets(uintptr_t instance) {
        size_t iterations = 0;
        constexpr size_t MAX_ITERATIONS = 50000;

        for (size_t container_off = 0; container_off < 0x200; container_off += 0x8) {
            uintptr_t container = memory->read<uintptr_t>(instance + container_off);
            if (!container || container < 0x10000)
                continue;

            for (size_t list_off = 0; list_off < 0x100; list_off += 0x8) {
                uintptr_t first_attr = memory->read<uintptr_t>(container + list_off);
                if (!first_attr || first_attr < 0x10000)
                    continue;

                size_t value_off = 0;
                bool found_value = false;
                for (size_t off = 0; off < 0x100; off += 0x8) {
                    if (++iterations > MAX_ITERATIONS) {
                        LOG_ERR("Attribute scan exceeded max iterations");
                        return std::nullopt;
                    }

                    std::string str = memory->read_roblox_string(first_attr + off);
                    if (str == "hellovalue") {
                        value_off = off;
                        found_value = true;
                        break;
                    }
                }
                if (!found_value)
                    continue;

                uintptr_t second_attr = 0;
                size_t stride = 0;
                constexpr size_t MAX_STRIDE_SEARCH = 0x200;

                for (size_t offset = 0x8; offset < MAX_STRIDE_SEARCH; offset += 0x8) {
                    if (++iterations > MAX_ITERATIONS) {
                        LOG_ERR("Attribute scan exceeded max iterations");
                        return std::nullopt;
                    }

                    uintptr_t test_addr = first_attr + offset + value_off;
                    if (test_addr < 0x10000)
                        break;

                    std::string forward = memory->read_roblox_string(test_addr);
                    if (forward == "hello2value") {
                        second_attr = first_attr + offset;
                        stride = offset;
                        break;
                    }
                }

                if (!second_attr)
                    continue;

                for (size_t off = 0; off < 0x100; off += 0x8) {
                    if (++iterations > MAX_ITERATIONS) {
                        LOG_ERR("Attribute scan exceeded max iterations");
                        return std::nullopt;
                    }

                    if (off == value_off)
                        continue;
                    uintptr_t ptr = memory->read<uintptr_t>(first_attr + off);
                    if (ptr && ptr > 0x10000) {
                        std::string str = memory->read_roblox_string(ptr);
                        if (str == "hello") {
                            return std::make_tuple(container_off, list_off, stride, value_off);
                        }
                    }
                }
            }
        }

        return std::nullopt;
    }

    inline bool instance(Context& ctx) {
        if (!ctx.workspace.is_valid()) {
            LOG_ERR("Workspace not valid");
            return false;
        }

        auto class_desc =
            memory->find_rtti_offset(ctx.workspace.address, "ClassDescriptor@Reflection@RBX");
        if (!class_desc) {
            LOG_ERR("Failed to find ClassDescriptor offset");
            return false;
        }
        offset_registry.add("Instance", "ClassDescriptor", *class_desc);

        auto workspace_class = memory->read<uintptr_t>(ctx.workspace.address + *class_desc);
        if (!workspace_class) {
            LOG_ERR("Failed to read ClassDescriptor pointer");
            return false;
        }

        auto class_name = memory->scan_roblox_string(workspace_class, "Workspace");
        if (!class_name) {
            LOG_ERR("Failed to find ClassName offset");
            return false;
        }
        offset_registry.add("Instance", "ClassName", class_name->pointer_offset);

        auto parent = memory->find_rtti_offset(ctx.workspace.address, "DataModel@RBX");
        if (!parent) {
            LOG_ERR("Failed to find Parent offset");
            return false;
        }
        offset_registry.add("Instance", "Parent", *parent);

        auto name = memory->scan_roblox_string(ctx.workspace.address, "Workspace");
        if (!name) {
            LOG_ERR("Failed to find Name offset");
            return false;
        }
        offset_registry.add("Instance", "Name", name->pointer_offset);

        auto children = find_children_offsets(ctx.workspace.address, 24);
        if (!children) {
            LOG_ERR("Failed to find Children offsets");
            return false;
        }
        offset_registry.add("Instance", "ChildrenStart", children->first);
        offset_registry.add("Instance", "ChildrenEnd", children->second);

        auto attributes_part = ctx.workspace.find_first_child("attributes");
        if (!attributes_part.is_valid()) {
            LOG_ERR("Failed to find 'attributes' Part in Workspace");
            return false;
        }

        auto attr = find_attribute_offsets(attributes_part.address);
        if (!attr) {
            LOG_ERR("Failed to find Attribute offsets");
            return false;
        }

        offset_registry.add("Instance", "AttributeContainer", std::get<0>(*attr));
        offset_registry.add("Instance", "AttributeList", std::get<1>(*attr));
        offset_registry.add("Instance", "AttributeToNext", std::get<2>(*attr));
        offset_registry.add("Instance", "AttributeToValue", std::get<3>(*attr));

        return true;
    }

} // namespace scanner::phases