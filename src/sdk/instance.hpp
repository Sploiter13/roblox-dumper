#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace sdk {
    class instance_t {
      public:
        std::uint64_t address = 0;

        instance_t() = default;
        explicit instance_t(std::uint64_t addr) : address(addr) {}

        std::string get_name() const;
        std::string get_class_name() const;
        std::vector<instance_t> get_children() const;
        instance_t find_first_child(const std::string& name) const;
        instance_t find_first_child_of_class(const std::string& class_name) const;
        instance_t get_parent() const;
        std::size_t get_children_count() const;

        bool is_valid() const { return address != 0; }
    };
} // namespace sdk