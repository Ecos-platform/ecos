
#include "compile.hpp"
#include "simulate.hpp"

#include "ecos/lib_info.hpp"
#include "ecos/logger/logger.hpp"

#include <cli11/CLI11.h>
#include <iostream>

using namespace ecos;

namespace
{

int print_help(const CLI::App& desc)
{
    std::cout << desc.help() << std::endl;
    return 0;
}

std::string versionString()
{
    const auto v = library_version();
    std::stringstream ss;
    ss << "v" << v.major << "." << v.minor << "." << v.patch;
    return ss.str();
}

void create_options(CLI::App& app)
{
    app.set_version_flag("-v,--version", versionString());

    create_simulate_options(app);
    create_compile_options(app);
}

} // namespace

int main(int argc, char** argv)
{

    CLI::App app("ecos");
    create_options(app);

    if (argc == 1) {
        return print_help(app);
    }

    try {

        CLI11_PARSE(app, argc, argv)

        if (const auto* sub = app.get_subcommand("simulate")) {
            if (sub->parsed()) {
                parse_simulate_options(*sub);
            }
        }
        if (const auto* sub = app.get_subcommand("compile")) {
            if (sub->parsed()) {

                parse_compile_options(*sub);
            }
        }


    } catch (std::exception& e) {
        log::err("Unhandled Exception reached the top of main: '{}', application will now exit", e.what());
        return 1;
    }
}
