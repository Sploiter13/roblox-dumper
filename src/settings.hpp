#pragma once
#include "utils/config.hpp"

namespace settings {
    inline uint64_t game_id() { return config::get().game_id; }
    inline uint64_t place_id() { return config::get().place_id; }
    inline uint64_t creator_id() { return config::get().creator_id; }
    inline uint64_t user_id() { return config::get().user_id; }
    inline const std::string& display_name() { return config::get().display_name; }
} // namespace settings