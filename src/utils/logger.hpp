#pragma once
#include <chrono>
#include <format>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#define NOMINMAX
#include <Windows.h>

#define COLOR_DEBUG "\033[90m"
#define COLOR_INFO "\033[94m"
#define COLOR_SUCCESS "\033[92m"
#define COLOR_WARN "\033[93m"
#define COLOR_ERR "\033[91m"
#define COLOR_RESET "\033[0m"

class Logger {
private:
  inline static std::mutex mtx{};

  static std::string timestamp() {
    using namespace std::chrono;

    auto now = system_clock::now();
    auto local_time = current_zone()->to_local(now);
    auto dp = floor<days>(local_time);
    auto time = local_time - dp;

    auto h = duration_cast<hours>(time);
    auto m = duration_cast<minutes>(time - h);
    auto s = duration_cast<seconds>(time - h - m);
    auto ms = duration_cast<milliseconds>(time - h - m - s);

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << h.count() << ':' << std::setw(2)
        << m.count() << ':' << std::setw(2) << s.count() << '.' << std::setw(3)
        << ms.count();
    return oss.str();
  }

  static void write_log(const char *color, const char *level,
                        const std::string &msg) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << color << timestamp() << " [" << level << "] " << msg
              << COLOR_RESET << std::endl;
  }

public:
  template <typename... Args>
  static void debug(std::format_string<Args...> fmt, Args &&...args) {
    write_log(COLOR_DEBUG, "DEBUG",
              std::format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void info(std::format_string<Args...> fmt, Args &&...args) {
    write_log(COLOR_INFO, "INFO",
              std::format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void success(std::format_string<Args...> fmt, Args &&...args) {
    write_log(COLOR_SUCCESS, "SUCCESS",
              std::format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void warn(std::format_string<Args...> fmt, Args &&...args) {
    write_log(COLOR_WARN, "WARN",
              std::format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void err(std::format_string<Args...> fmt, Args &&...args) {
    write_log(COLOR_ERR, "ERR", std::format(fmt, std::forward<Args>(args)...));
  }
};

struct EnableAnsi {
  EnableAnsi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
      DWORD dwMode = 0;
      if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
      }
    }
  }
};
inline static EnableAnsi enable_ansi;

#define LOG_DEBUG(...) Logger::debug(__VA_ARGS__)
#define LOG_INFO(...) Logger::info(__VA_ARGS__)
#define LOG_SUCCESS(...) Logger::success(__VA_ARGS__)
#define LOG_WARN(...) Logger::warn(__VA_ARGS__)
#define LOG_ERR(...) Logger::err(__VA_ARGS__)