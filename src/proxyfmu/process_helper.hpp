
#ifndef ECOS_PROXYFMU_PROCESS_HELPER_HPP
#define ECOS_PROXYFMU_PROCESS_HELPER_HPP

#include <ecos/logger/logger.hpp>

#include <spdlog/spdlog.h>
#include <subprocess/subprocess.h>

#include <filesystem>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
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
    ssize_t count = readlink("/proc/self/exe", result, sizeof(result) - 1);
    if (count != -1) {
        result[count] = '\0'; // Ensure null-termination
        return std::string(result);
    }
    return "";
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
        const auto alt_executable = std::filesystem::path(loc).parent_path() / executable;
        if (exists(alt_executable)) {
            executable = alt_executable;
        }
    }

    std::string execStr = executable.string();
#ifdef __linux__
    if (is_regular_file(executable) && !executable.is_absolute()) {
        execStr.insert(0, "./");
    }
#endif

    log::debug("[proxyfmu] Checking if executable ({}) is available..", execStr);
    std::ostringstream ss;
    ss << std::quoted(execStr) << " -v";
    const int statusCode = std::system(ss.str().c_str());
    if (statusCode != 0) {
        log::err("[proxyfmu] Unable to invoke proxyfmu!");

        bind.set_value("");
        return;
    }

    log::info("[proxyfmu] Booting FMU instance '{}'", instanceName);

    const std::string fmuPathStr = fmuPath.string();
    const std::string localStr = toString(local);
    std::vector<const char*> cmd = {
        execStr.c_str(),
        "--fmu", fmuPathStr.c_str(),
        "--instanceName", instanceName.c_str(),
        "--local", localStr.c_str(),
        nullptr};

    subprocess_s process{};
    int result = subprocess_create(cmd.data(), subprocess_option_inherit_environment | subprocess_option_search_user_path | subprocess_option_no_window, &process);

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
                        log::info("[proxyfmu] Instance '{}' instantiated using port {}", instanceName, bindVal);
                    } else {
                        log::info("[proxyfmu] Instance '{}' instantiated using handle '{}'", instanceName, bindVal);
                    }
                }
                bound = true;
            } else {
                log::debug("[proxyfmu][{}] Got: {}", instanceName, line);
            }
        }

        int status = 0;
        result = subprocess_join(&process, &status);

        if (result == 0 && status == 0 && bound) {
            log::info("[proxyfmu] External process for instance '{}' returned with status {}", instanceName, status);
            return;
        }
        log::err("[proxyfmu] External process for instance '{}' returned with status {}. Unable to bind..", instanceName, std::to_string(status));
    } else {

        log::err("[proxyfmu] Fatal: Subrocess create returned non-zero status: {}", result);
    }

    bind.set_value("");
}

} // namespace ecos::proxy

#endif // ECOS_PROXYFMU_PROCESS_HELPER_HPP
