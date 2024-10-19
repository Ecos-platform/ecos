
#ifndef ECOS_PROXYFMU_PROCESS_HELPER_HPP
#define ECOS_PROXYFMU_PROCESS_HELPER_HPP

#include <subprocess/subprocess.h>

#include <exception>
#include <filesystem>
#include <future>
#include <iostream>
#include <string>

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif
#ifdef __linux__
#    include <climits>
#    include <cstring>
#    include <unistd.h>
#endif

namespace proxyfmu
{

inline std::string getLoc()
{
#ifdef __linux__
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, (count > 0) ? count : 0);
#else
    char result[MAX_PATH];
    return {result, GetModuleFileName(nullptr, result, MAX_PATH)};
#endif
}

inline void start_process(
    const std::filesystem::path& fmuPath,
    const std::string& instanceName,
    std::promise<int>& port)
{

    std::filesystem::path executable;
#ifdef __linux__
    executable = "proxyfmu";
#else
    executable = "proxyfmu.exe";
#endif

    if (!exists(executable)) {
        const std::string loc = getLoc();
        const auto alt_executable = std::filesystem::path(loc).parent_path().string() / executable;
        if (exists(alt_executable)) {
            executable = alt_executable;
        }
    }

    std::string execStr = executable.string();
#ifdef __linux__
    if (!executable.is_absolute()) {
        execStr.insert(0, "./");
    }
#endif

    std::cout << "[proxyfmu] Checking if proxyfmu is available.." << std::endl;
    const int statusCode = system(("\"" + execStr + "\" -v").c_str());
    if (statusCode != 0) {
        std::cerr << "ERROR - unable to invoke proxyfmu!" << std::endl;

        port.set_value(-1);
        return;
    }
    std::cout << std::endl;
    std::cout << "[proxyfmu] Booting FMU instance '" << instanceName << "'.." << std::endl;

    const std::string fmuPathStr = fmuPath.string();
    std::vector<const char*> cmd = {execStr.c_str(), "--fmu", fmuPathStr.c_str(), "--instanceName", instanceName.c_str(), nullptr};

    subprocess_s process{};
    int result = subprocess_create(cmd.data(), subprocess_option_inherit_environment | subprocess_option_no_window, &process);

    bool bound = false;
    if (result == 0) {
        FILE* p_stdout = subprocess_stdout(&process);
        char buffer[256];
        while (fgets(buffer, 256, p_stdout)) {
            std::string line(buffer);
            if (!bound && line.substr(0, 16) == "[proxyfmu] port=") {
                {
                    const auto parsedPort = std::stoi(line.substr(16));
                    port.set_value(parsedPort);
                    std::cout << "[proxyfmu] FMU instance '" << instanceName << "' instantiated using port " << parsedPort << std::endl;
                }
                bound = true;
            } else if (line.substr(0, 16) == "[proxyfmu] freed") {
                break;
            } else {
                std::cerr << line;
            }
        }

        int status = 0;
        result = subprocess_join(&process, &status);

        if (result == 0 && status == 0 && bound) {
            return;
        }
        std::cerr << "[proxyfmu] External proxy process for instance '"
                  << instanceName << "' returned with status "
                  << std::to_string(status) << ". Unable to bind.." << std::endl;
    }
    port.set_value(-1);
}

} // namespace proxyfmu

#endif // ECOS_PROXYFMU_PROCESS_HELPER_HPP
