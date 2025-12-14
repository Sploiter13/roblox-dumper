#pragma once
#include "memory/memory.h"
#include "sdk/instance.hpp"
#include "utils/logger.hpp"
#include "utils/offset_registry.hpp"
#include <functional>
#include <string>
#include <vector>

namespace scanner {

    struct Context {
        uintptr_t visual_engine = 0;
        uintptr_t data_model = 0;
        sdk::instance_t workspace;
    };

    using ScanPhase = std::function<bool(Context&)>;

    class PhaseRegistry {
      private:
        std::vector<std::pair<std::string, ScanPhase>> phases;

      public:
        void register_phase(const std::string& name, ScanPhase phase) {
            phases.push_back({name, phase});
        }

        bool run_all(Context& ctx) {
            for (const auto& [name, phase] : phases) {
                LOG_INFO("Phase: {}", name);
                if (!phase(ctx)) {
                    LOG_ERR("Phase {} failed", name);
                    return false;
                }
            }
            return true;
        }
    };

    bool run_all_phases();
} // namespace scanner