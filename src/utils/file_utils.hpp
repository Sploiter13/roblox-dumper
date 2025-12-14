#include <Windows.h>
#include <filesystem>
#include <string>

namespace file_utils {
    inline std::string get_exe_directory() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::string path(buffer);
        return path.substr(0, path.find_last_of("\\/"));
    }
} // namespace file_utils
