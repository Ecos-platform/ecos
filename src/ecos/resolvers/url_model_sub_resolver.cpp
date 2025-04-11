
#include "url_model_sub_resolver.hpp"

#include "util/temp_dir.hpp"

#include "ecos/fmi/fmi_model.hpp"
#include "ecos/logger/logger.hpp"

#include <fstream>
#include <regex>

using namespace ecos;

namespace
{
bool isValidURL(const std::string& url)
{
    // Regular expression for validating a URL
    const std::regex pattern(R"((http|https)://([a-zA-Z0-9.-]+)(:[0-9]+)?(/.*)?)");
    return std::regex_match(url, pattern);
}

std::string getFilenameFromURL(const std::string& url)
{
    // Find the last '/' in the URL
    const size_t lastSlash = url.find_last_of('/');
    if (lastSlash != std::string::npos) {
        // Return the substring after the last '/'
        return url.substr(lastSlash + 1);
    }

    return ""; // Return an empty string if no '/' is found
}

bool isFileEmpty(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    return file.tellg() == 0;
}

std::optional<std::filesystem::path> downloadFile(const std::string& url, const std::filesystem::path& outputDir)
{

    const auto filename = getFilenameFromURL(url);
    if (filename.empty()) return std::nullopt;

    std::filesystem::path outputPath = outputDir / filename;
#ifdef _WIN32
    std::string command = "curl -o " + outputPath.string() + " " + url;
#else
    std::string command = "wget -O " + outputPath.string() + " " + url;
#endif

    if (system(command.c_str()) == 0) {
        log::debug("File downloaded successfully: {}", outputPath.string());
        return outputPath;
    }

    log::err("Error downloading file.");
    return std::nullopt;
}

} // namespace

url_model_sub_resolver::url_model_sub_resolver()
    : temp_dir_("url_sub_resolver")
{ }

std::unique_ptr<model> url_model_sub_resolver::resolve(const std::filesystem::path& base, const std::string& uri)
{

    if (!isValidURL(uri) || !uri.ends_with(".fmu")) {
        return nullptr;
    }

    log::debug("Resolving url {}", uri);

    if (const auto fmuFile = downloadFile(uri, temp_dir_.path().string())) {
        if (isFileEmpty(fmuFile->string())) {
            log::err("Downloaded file is empty!", fmuFile->string());
            return nullptr;
        }

        return std::make_unique<fmi_model>(*fmuFile);
    }

    return nullptr;
}
