#pragma once
#include <atomic>
#include <chrono>
#include <deque>
#include <httplib.h>
#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

namespace control {

    struct CommandData {
        std::string id;
        std::string action;
        nlohmann::json data;
    };

    struct CommandResult {
        std::string status; // "pending", "completed", "failed"
        nlohmann::json result;
        std::string timestamp;
    };

    class ControlServer {
      private:
        std::unique_ptr<httplib::Server> server;
        std::thread server_thread;
        std::atomic<bool> running{false};

        std::deque<CommandData> command_queue;
        std::map<std::string, CommandResult> command_results;
        std::mutex queue_mutex;
        std::mutex results_mutex;

        int port;

        std::string get_timestamp();
        std::string generate_uuid();

        void setup_routes();

      public:
        ControlServer(int port = 8000);
        ~ControlServer();

        void start();
        void stop();
        bool is_running() const { return running; }
    };

} // namespace control