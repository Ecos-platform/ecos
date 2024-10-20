
#ifndef ECOS_PROXYFMU_PROCESS_HELPER_HPP
#define ECOS_PROXYFMU_PROCESS_HELPER_HPP

#include <spdlog/spdlog.h>
#include <subprocess/subprocess.h>

#include <filesystem>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif
#ifdef __linux__
#    include <climits>
#    include <cstring>
#    include <unistd.h>
#endif

namespace ecos::proxy
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

inline std::string toString(bool value)
{
    return value ? "true" : "false";
}

inline void start_process(
    const std::filesystem::path& fmuPath,
    const std::string& instanceName,
    std::promise<std::string>& bind,
    bool local)
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

    spdlog::debug("Checking if proxyfmu is available..");
    const int statusCode = system(("\"" + execStr + "\" -v").c_str());
    if (statusCode != 0) {
        spdlog::error("Unable to invoke proxyfmu!");

        bind.set_value("-");
        return;
    }

    spdlog::info("Booting FMU instance '{}'", instanceName);

    const std::string fmuPathStr = fmuPath.string();
    const std::string localStr = toString(local);
    std::vector<const char*> cmd = {
        execStr.c_str(),
        "--fmu", fmuPathStr.c_str(),
        "--instanceName", instanceName.c_str(),
        "--local", localStr.c_str(),
        nullptr};

    subprocess_s process{};
    int result = subprocess_create(cmd.data(), subprocess_option_inherit_environment | subprocess_option_no_window, &process);

    bool bound = false;
    if (result == 0) {
        FILE* p_stdout = subprocess_stdout(&process);
        char buffer[256];
        while (fgets(buffer, 256, p_stdout)) {
            std::string line(buffer);
            if (!bound && line.substr(0, 16) == "[proxyfmu] bind=") {
                {
                    auto bindVal = line.substr(16);

                    while (bindVal.back() == '\r' || bindVal.back() == '\n') {
                        bindVal.pop_back();
                    }
                    bind.set_value(bindVal);

                    if (!local) {
                        spdlog::info("FMU proxy instance '{}' instantiated using port {}", instanceName, bindVal);
                    } else {
                        spdlog::info("FMU proxy instance '{}' instantiated using file '{}'", instanceName, bindVal);
                    }
                }
                bound = true;
            } else if (line.substr(0, 16) == "[proxyfmu] freed") {
                break;
            } else {
                std::cerr << "Got: " << line;
            }
        }

        int status = 0;
        result = subprocess_join(&process, &status);

        if (result == 0 && status == 0 && bound) {
            return;
        }
        spdlog::error("External proxy process for instance '{}' returned with status {}. Unable to bind..", instanceName, std::to_string(status));
    }

    bind.set_value("");
}

} // namespace ecos::proxy

#endif // ECOS_PROXYFMU_PROCESS_HELPER_HPP
