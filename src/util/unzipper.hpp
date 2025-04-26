
#ifndef ECOS_UNZIPPER_HPP
#define ECOS_UNZIPPER_HPP

#ifdef _WIN32
#    include <process.h>
#else
#    include <fcntl.h>
#    include <sys/wait.h>
#    include <unistd.h>
#endif

#include <filesystem>
#include <string>
#include <vector>


namespace ecos
{

namespace detail
{

inline std::vector<std::string> make_args(const std::string& zip, const std::string& temp)
{

#ifdef _WIN32
    char* shell = nullptr;
    size_t len = 0;
    std::string shell_str;
    if (_dupenv_s(&shell, &len, "SHELL") == 0 && shell) {
        shell_str = shell;
        free(shell);
    }

    // Use unzip in Git Bash, otherwise tar
    const bool use_unzip = shell_str.find("bash") != std::string::npos;
#else
    constexpr bool use_unzip = true;
#endif

    return use_unzip
        ? std::vector<std::string>{"unzip", "-o", zip, "-d", temp}
        : std::vector<std::string>{"tar", "-xf", zip, "-C", temp};
}

inline bool unzip_with_system(const std::vector<char*>& argv)
{
#ifdef _WIN32
    const auto result = _spawnvp(_P_WAIT, argv[0], argv.data());
    return result == 0;
#else
    pid_t pid = fork();
    if (pid == -1) {
        return false; // Fork failed
    }
    if (pid == 0) {
        // Child process
        const int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDOUT_FILENO); // Redirect stdout to /dev/null
            dup2(dev_null, STDERR_FILENO); // Redirect stderr to /dev/null
            close(dev_null);
        }

        execvp(argv[0], argv.data());
        // If execvp returns, it failed
        std::_Exit(EXIT_FAILURE);
    }
    // Parent process : wait for child
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
#endif
}
} // namespace detail

inline bool unzip(const std::filesystem::path& zip_file, const std::filesystem::path& tmp_path)
{

    if (!exists(zip_file) || !exists(tmp_path)) {
        return false;
    }

    const auto args = detail::make_args(zip_file.string(), tmp_path.string());

    std::vector<char*> argv;
    for (auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr); // Null-terminate

    return detail::unzip_with_system(argv);
}

} // namespace ecos

#endif // ECOS_UNZIPPER_HPP
