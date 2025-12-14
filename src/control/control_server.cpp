#include "control_server.hpp"
#include "utils/logger.hpp"
#include <iomanip>
#include <random>
#include <sstream>

namespace control {

    std::string ControlServer::get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_val = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_val), "%Y-%m-%dT%H:%M:%S");
        return ss.str();
    }

    std::string ControlServer::generate_uuid() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; i++)
            ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 4; i++)
            ss << dis(gen);
        ss << "-4";
        for (int i = 0; i < 3; i++)
            ss << dis(gen);
        ss << "-";
        ss << dis2(gen);
        for (int i = 0; i < 3; i++)
            ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 12; i++)
            ss << dis(gen);
        return ss.str();
    }

    void ControlServer::setup_routes() {
        server->Post("/execute", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                auto json_body = nlohmann::json::parse(req.body);
                std::string command_id = generate_uuid();

                CommandData cmd;
                cmd.id = command_id;
                cmd.action = json_body["action"];
                cmd.data = json_body["data"];

                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    command_queue.push_back(cmd);
                }

                {
                    std::lock_guard<std::mutex> lock(results_mutex);
                    command_results[command_id] = {"pending", nullptr, get_timestamp()};
                }

                nlohmann::json response = {{"command_id", command_id}, {"status", "queued"}};
                res.set_content(response.dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                nlohmann::json error = {{"error", e.what()}};
                res.set_content(error.dump(), "application/json");
            }
        });

        server->Get("/poll", [this](const httplib::Request&, httplib::Response& res) {
            nlohmann::json response;
            std::lock_guard<std::mutex> lock(queue_mutex);

            if (!command_queue.empty()) {
                nlohmann::json commands = nlohmann::json::array();
                for (const auto& cmd : command_queue) {
                    commands.push_back(
                        {{"id", cmd.id}, {"action", cmd.action}, {"data", cmd.data}});
                }
                command_queue.clear();
                response["commands"] = commands;
            } else {
                response["commands"] = nlohmann::json::array();
            }

            res.set_content(response.dump(), "application/json");
        });

        server->Post("/result", [this](const httplib::Request& req, httplib::Response& res) {
            try {
                auto json_body = nlohmann::json::parse(req.body);
                std::string command_id = json_body["command_id"];

                std::lock_guard<std::mutex> lock(results_mutex);
                if (command_results.find(command_id) != command_results.end()) {
                    command_results[command_id] = {json_body["status"], json_body["result"],
                                                   json_body["timestamp"]};
                    nlohmann::json response = {{"status", "ok"}};
                    res.set_content(response.dump(), "application/json");
                } else {
                    res.status = 404;
                    nlohmann::json error = {{"status", "error"},
                                            {"message", "command_id not found"}};
                    res.set_content(error.dump(), "application/json");
                }
            } catch (const std::exception& e) {
                res.status = 400;
                nlohmann::json error = {{"error", e.what()}};
                res.set_content(error.dump(), "application/json");
            }
        });

        server->Get("/status/(.*)", [this](const httplib::Request& req, httplib::Response& res) {
            std::string command_id = req.matches[1];
            std::lock_guard<std::mutex> lock(results_mutex);

            if (command_results.find(command_id) != command_results.end()) {
                auto& result = command_results[command_id];
                nlohmann::json response = {{"status", result.status},
                                           {"result", result.result},
                                           {"timestamp", result.timestamp}};
                res.set_content(response.dump(), "application/json");
            } else {
                nlohmann::json response = {{"status", "not_found"}};
                res.set_content(response.dump(), "application/json");
            }
        });

        server->Post("/clear", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(queue_mutex);
            command_queue.clear();
            nlohmann::json response = {{"status", "cleared"}};
            res.set_content(response.dump(), "application/json");
        });

        server->Get("/", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(queue_mutex);
            nlohmann::json response = {{"status", "running"}, {"queue_size", command_queue.size()}};
            res.set_content(response.dump(), "application/json");
        });
    }

    ControlServer::ControlServer(int port) : port(port) {
        server = std::make_unique<httplib::Server>();
        setup_routes();
    }

    ControlServer::~ControlServer() { stop(); }

    void ControlServer::start() {
        if (running)
            return;

        running = true;
        server_thread = std::thread([this]() {
            LOG_INFO("Lua Control server starting on http://localhost:{}", port);
            if (!server->listen("0.0.0.0", port)) {
                LOG_ERR("Failed to start server on port {}", port);
                running = false;
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void ControlServer::stop() {
        if (!running)
            return;

        running = false;
        server->stop();
        if (server_thread.joinable()) {
            server_thread.join();
        }
        LOG_INFO("Control server stopped");
    }

} // namespace control