
#ifndef LIBECOS_COMPILE_HPP
#define LIBECOS_COMPILE_HPP

#include "pugixml.hpp"
#include "util/temp_dir.hpp"
#include "util/unzipper.hpp"

#include "subprocess/subprocess.h"

#include <cli11/CLI11.h>
#include <utility>

namespace ecos
{

const char* fmi3Functions();
const char* fmi3PlatformTypes();
const char* fmi3FunctionTypes();

inline std::string get_target_platform()
{
#ifdef _WIN32
    return "x86_64-windows";
#else
    return "x86_64-linux";
#endif
    throw std::runtime_error("unsupported platform");
}

inline void create_compile_options(CLI::App& app)
{
    auto compile = app.add_subcommand("compile");

    compile->add_option("--fmu", "Location of the fmu to compile.")->required();
    compile->add_option("--dest", "Save location of the compiled fmu.");
    compile->add_flag("--force", "Overwrite any existing binary for current platform.");
}

struct BuildInfo
{
    std::string modelIdentifier;
    // there may be multiple SourceFileSet entries; store entries per set
    struct SourceFileSetInfo
    {
        std::string language;
        std::string compilerOptions;
        std::vector<std::string> sources;
        std::map<std::string, std::string> preprocessorDefinitions;
        std::vector<std::string> includeDirectories;
    };
    std::vector<SourceFileSetInfo> sourceFileSets;
};

inline BuildInfo parse_build_description(const std::filesystem::path& xml_path)
{
    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load_file(xml_path.string().c_str());
    if (!res) {
        throw std::runtime_error("Failed to parse " + xml_path.string() + ": " + std::string(res.description()));
    }

    pugi::xml_node root = doc.child("fmiBuildDescription");
    if (!root) throw std::runtime_error("Missing root element 'fmiBuildDescription'");

    pugi::xml_node buildConf = root.child("BuildConfiguration");
    if (!buildConf) throw std::runtime_error("No 'BuildConfiguration' element found");

    BuildInfo info;
    info.modelIdentifier = buildConf.attribute("modelIdentifier").as_string();

    for (pugi::xml_node sfs : buildConf.children("SourceFileSet")) {
        BuildInfo::SourceFileSetInfo setInfo;
        setInfo.language = sfs.attribute("language").as_string();
        setInfo.compilerOptions = sfs.attribute("compilerOptions").as_string();

        for (pugi::xml_node sf : sfs.children("SourceFile")) {
            const char* name = sf.attribute("name").as_string();
            if (name && *name) setInfo.sources.emplace_back(name);
        }

        for (pugi::xml_node pd : sfs.children("PreprocessorDefinition")) {
            const char* name = pd.attribute("name").as_string();
            const char* value = pd.attribute("value").as_string();
            if (name && *name) setInfo.preprocessorDefinitions.emplace(std::string(name),
                std::string(value ? value : ""));
        }

        for (pugi::xml_node inc : sfs.children("IncludeDirectory")) {
            const char* name = inc.attribute("name").as_string();
            if (name && *name) setInfo.includeDirectories.emplace_back(name);
        }

        info.sourceFileSets.emplace_back(std::move(setInfo));
    }

    return info;
}

class ScopedFolder
{
public:
    explicit ScopedFolder(std::filesystem::path path)
        : folderPath_(std::move(path))
    {
        std::filesystem::create_directories(folderPath_);
    }

    ~ScopedFolder() noexcept
    {
        std::cout << "Removing temporary folder: " << folderPath_.string() << std::endl;
        try {
            if (std::filesystem::exists(folderPath_)) {
                std::filesystem::remove_all(folderPath_);
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to remove folder '" << folderPath_ << "': " << e.what() << '\n';
        }
    }

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return folderPath_; }

private:
    std::filesystem::path folderPath_;
};


inline void parse_compile_options(const CLI::App& app)
{
    const std::filesystem::path path = app["--fmu"]->as<std::string>();
    std::filesystem::path destFolder;
    if (app.count("--dest")) {
        destFolder = app["--dest"]->as<std::string>();
        if (destFolder.string() == ".") {
            destFolder = std::filesystem::current_path();
        }
    } else {
        destFolder = path.parent_path();
    }

    const bool force = app.count("--force") ? true : false;

    if (!exists(path)) {
        throw std::runtime_error("No such file: " + absolute(path).string());
    }

    if (path.extension() != ".fmu") {
        throw std::runtime_error("File is not an fmu: " + absolute(path).string());
    }

    const auto tmp = temp_dir("fmu_compile");
    const auto extractDir = tmp.path() / "source_fmu";
    auto sourcesDir = extractDir / "sources";
    const auto buildFolder = tmp.path() / "build";
    const auto binaryDir = extractDir / "binaries" / get_target_platform();
    const auto binaryDirAbsolute = std::filesystem::absolute(binaryDir);


    std::cout << "Extracting "<< path.string() << " to: " << extractDir.string() << std::endl;

    std::filesystem::create_directories(extractDir);
    // extract fmu into tmp
    unzip(path, extractDir);

    if (!std::filesystem::exists(sourcesDir)) {
        throw std::runtime_error("FMU does not contain sources: " + extractDir.string());
    }

    std::filesystem::path workingSources = tmp.path() / "sources";
    std::filesystem::copy(sourcesDir, workingSources, std::filesystem::copy_options::recursive);
    sourcesDir = workingSources;

    if (!std::filesystem::exists(sourcesDir / "buildDescription.xml")) {
        throw std::runtime_error("FMU does not contain buildDescription.xml");
    }

    if (!force && std::filesystem::exists(binaryDir)) {
        std::cout << "Binary for current platform already exists at: " << binaryDir.string() << ". To overwrite use --force" << std::endl;
        return; // binary already exists
    }

    const auto desc = parse_build_description(sourcesDir / "buildDescription.xml");
    if (desc.sourceFileSets.empty()) throw std::runtime_error("No SourceFileSet found in buildDescription.xml");

    const auto& modelIdentifier = desc.modelIdentifier;
    const auto& sourceSet = desc.sourceFileSets.front();

    std::map<std::string, std::string> langMap{
        {"C99", "CMAKE_C_STANDARD 99"},
        {"C11", "CMAKE_C_STANDARD 11"},
        {"C17", "CMAKE_C_STANDARD 17"},
        {"C++11", "CMAKE_CXX_STANDARD 11"},
        {"C++14", "CMAKE_CXX_STANDARD 14"},
        {"C++17", "CMAKE_CXX_STANDARD 17"},
        {"C++20", "CMAKE_CXX_STANDARD 20"},
        {"C++23", "CMAKE_CXX_STANDARD 23"},
    };

    if (!langMap.contains(sourceSet.language)) {
        throw std::runtime_error("Unknown language: " + sourceSet.language);
    }

    {
        const std::filesystem::path destinationFile = destFolder / (path.stem().string() +  ".fmu");

        auto sanitize = [](std::string path) {
            std::ranges::replace(path, '\\', '/');
            return path;
        };

        std::ostringstream cmakeLists;
        cmakeLists << "cmake_minimum_required(VERSION 3.15)\n";
        cmakeLists << "project(CompileFMU)\n\n";
        cmakeLists << "set(CMAKE_MSVC_RUNTIME_LIBRARY \"MultiThreaded$<$<CONFIG:Debug>:Debug>\")\n";
        cmakeLists << "set(" << langMap[sourceSet.language] << ")\n\n";

        cmakeLists << "add_library(" << modelIdentifier << " SHARED\n";
        for (const auto& file : sourceSet.sources) {
            cmakeLists << "\t" << file << "\n";
        }
        cmakeLists << ")\n";

        cmakeLists << "target_include_directories(" << modelIdentifier << " PRIVATE \"${CMAKE_CURRENT_SOURCE_DIR}\")\n";
        if (!sourceSet.includeDirectories.empty()) {
            cmakeLists << "target_include_directories(" << modelIdentifier << " PRIVATE\n";
            for (const auto& includeDir : sourceSet.includeDirectories) {
                cmakeLists << "\t\"" << includeDir << "\"" << "\n";
            }
            cmakeLists << ")\n";
        }
        if (!sourceSet.preprocessorDefinitions.empty()) {
            cmakeLists << "target_compile_definitions(" << modelIdentifier << " PRIVATE\n";
            for (const auto& [key, value] : sourceSet.preprocessorDefinitions) {
                if (value.empty()) {
                    cmakeLists << "\t" << key << "\n";
                } else {
                    cmakeLists << "\t" << key << "=" << value << "\n";
                }
            }
            cmakeLists << ")\n";
        }

        cmakeLists << "if (WIN32)\n";
        cmakeLists << "\ttarget_compile_definitions(" << modelIdentifier << " PUBLIC\n";
        cmakeLists << "\t\t\"FMI3_Export=__declspec(dllexport)\"\n";
        cmakeLists << "\t\t\"FMI3_FUNCTION_PREFIX=\"\n";
        cmakeLists << "\t)\n";
        cmakeLists << "endif()\n\n";

        const auto sanitizedBinaryDir = sanitize(binaryDirAbsolute.string());
        cmakeLists << "if (CMAKE_CONFIGURATION_TYPES)\n";
        cmakeLists << "\tforeach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})\n";
        cmakeLists << "\t\tstring(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)\n";
        cmakeLists << "\t\tset_target_properties(" << modelIdentifier << " PROPERTIES\n";
        cmakeLists << "\t\t\tRUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} \"" << sanitizedBinaryDir << "\"\n";
        cmakeLists << "\t\t\tLIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} \"" << sanitizedBinaryDir << "\"\n";
        cmakeLists << "\t\t\tARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} \"" << sanitizedBinaryDir << "\"\n";
        cmakeLists << "\t\t)\n";
        cmakeLists << "\tendforeach()\n";
        cmakeLists << "else()\n";
        cmakeLists << "\tset_target_properties(" << modelIdentifier << " PROPERTIES\n";
        cmakeLists << "\t\tRUNTIME_OUTPUT_DIRECTORY \"" << sanitizedBinaryDir << "\"\n";
        cmakeLists << "\t\tLIBRARY_OUTPUT_DIRECTORY \"" << sanitizedBinaryDir << "\"\n";
        cmakeLists << "\t\tARCHIVE_OUTPUT_DIRECTORY \"" << sanitizedBinaryDir << "\"\n";
        cmakeLists << "\t)\n";
        cmakeLists << "endif()\n";


        cmakeLists << "\tset_target_properties(" << modelIdentifier << "\n";
        cmakeLists << "\t\tPROPERTIES\n";
        cmakeLists << "\t\tPREFIX \"\" \n";
        cmakeLists << "\t)\n";

        cmakeLists << "add_custom_command(TARGET " << modelIdentifier << " POST_BUILD\n";
        cmakeLists << "\tWORKING_DIRECTORY \"" << sanitize(extractDir.string()) << "\"\n";
        cmakeLists << "\tCOMMAND ${CMAKE_COMMAND} -E tar cf \"" << sanitize(destinationFile.string()) << "\" --format=zip ." << "\n";
        cmakeLists << ")\n";

        std::cout << "Creating FMU at " << destinationFile << std::endl;

        {
            std::ofstream outFile;
            outFile.open((sourcesDir / "CMakeLists.txt"));
            outFile << cmakeLists.str();
        }

        {
            std::ofstream outFile;
            outFile.open(sourcesDir / "fmi3Functions.h");
            outFile << fmi3Functions();
        }

        {
            std::ofstream outFile;
            outFile.open(sourcesDir / "fmi3PlatformTypes.h");
            outFile << fmi3PlatformTypes();
        }

        {
            std::ofstream outFile;
            outFile.open(sourcesDir / "fmi3FunctionTypes.h");
            outFile << fmi3FunctionTypes();
        }

        auto sourceDirStr = sourcesDir.string();
        auto buildFolderStr = buildFolder.string();

        std::vector<const char*> configureCommand{
            "cmake",
            sourceDirStr.c_str(),
            "-B",
            buildFolderStr.c_str(),
            "-DCMAKE_BUILD_TYPE=Release",
            nullptr};

        std::vector<const char*> compileCommand{
            "cmake",
            "--build",
            buildFolderStr.c_str(),
            nullptr};

#ifdef _WIN32
        configureCommand.insert(configureCommand.end() - 1, "-A");
        configureCommand.insert(configureCommand.end() - 1, "x64");

        compileCommand.insert(compileCommand.end() - 1, "--config");
        compileCommand.insert(compileCommand.end() - 1, "Release");
#endif


        subprocess_s proc{};
        int res = subprocess_create(configureCommand.data(), subprocess_option_inherit_environment | subprocess_option_no_window, &proc);
        if (res != 0) {
            throw std::runtime_error("Failed to run cmake configure");
        }

        FILE* p_stdout = subprocess_stdout(&proc);
        char buffer[256];
        while (fgets(buffer, 256, p_stdout)) {
            std::string line(buffer);
            std::cout << line;
        }

        int status;
        res = subprocess_join(&proc, &status);
        if (res != 0 || status != 0) {
            throw std::runtime_error("CMake configure failed");
        }

        res = subprocess_create(compileCommand.data(), subprocess_option_inherit_environment | subprocess_option_no_window, &proc);
        if (res != 0) {
            throw std::runtime_error("Failed to run cmake build");
        }

        p_stdout = subprocess_stdout(&proc);
        while (fgets(buffer, 256, p_stdout)) {
            std::string line(buffer);
            std::cout << line;
        }

        res = subprocess_join(&proc, &status);
        if (res != 0 || status != 0) {
            throw std::runtime_error("CMake build failed");
        }
    }
}

} // namespace ecos

inline const char* ecos::fmi3Functions()
{

    static std::string s(

        R"(
#ifndef fmi3Functions_h
#define fmi3Functions_h

/*
This header file declares the functions of FMI 3.0.
It must be used when compiling an FMU.

In order to have unique function names even if several FMUs
are compiled together (e.g. for embedded systems), every "real" function name
is constructed by prepending the function name by "FMI3_FUNCTION_PREFIX".
Therefore, the typical usage is:

  #define FMI3_FUNCTION_PREFIX MyModel_
  #include "fmi3Functions.h"

As a result, a function that is defined as "fmi3GetContinuousStateDerivatives" in this header file,
is actually getting the name "MyModel_fmi3GetContinuousStateDerivatives".

This only holds if the FMU is shipped in C source code, or is compiled in a
static link library. For FMUs compiled in a DLL/sharedObject, the "actual" function
names are used and "FMI3_FUNCTION_PREFIX" must not be defined.

Copyright (C) 2008-2011 MODELISAR consortium,
              2012-2024 Modelica Association Project "FMI"
              All rights reserved.

This file is licensed by the copyright holders under the 2-Clause BSD License
(https://opensource.org/licenses/BSD-2-Clause):

----------------------------------------------------------------------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "fmi3PlatformTypes.h"
#include "fmi3FunctionTypes.h"
#include <stdlib.h>

/*
Allow override of FMI3_FUNCTION_PREFIX: If FMI3_OVERRIDE_FUNCTION_PREFIX
is defined, then FMI3_ACTUAL_FUNCTION_PREFIX will be used, if defined,
or no prefix if undefined. Otherwise FMI3_FUNCTION_PREFIX will be used,
if defined.
*/
#if !defined(FMI3_OVERRIDE_FUNCTION_PREFIX) && defined(FMI3_FUNCTION_PREFIX)
  #define FMI3_ACTUAL_FUNCTION_PREFIX FMI3_FUNCTION_PREFIX
#endif

/*
Export FMI3 API functions on Windows and under GCC.
If custom linking is desired then the FMI3_Export must be
defined before including this file. For instance,
it may be set to __declspec(dllimport).
*/
#if !defined(FMI3_Export)
  #if !defined(FMI3_ACTUAL_FUNCTION_PREFIX)
    #if defined _WIN32 || defined __CYGWIN__
     /* Note: both gcc & MSVC on Windows support this syntax. */
        #define FMI3_Export __declspec(dllexport)
    #else
      #if __GNUC__ >= 4
        #define FMI3_Export __attribute__ ((visibility ("default")))
      #else
        #define FMI3_Export
      #endif
    #endif
  #else
    #define FMI3_Export
  #endif
#endif

/* Macros to construct the real function name (prepend function name by FMI3_FUNCTION_PREFIX) */
#if defined(FMI3_ACTUAL_FUNCTION_PREFIX)
  #define fmi3Paste(a,b)     a ## b
  #define fmi3PasteB(a,b)    fmi3Paste(a,b)
  #define fmi3FullName(name) fmi3PasteB(FMI3_ACTUAL_FUNCTION_PREFIX, name)
#else
  #define fmi3FullName(name) name
#endif

/* FMI version */
#define fmi3Version "3.0"
)"
        R"(
/***************************************************
Common Functions
****************************************************/

/* Inquire version numbers and set debug logging */
#define fmi3GetVersion               fmi3FullName(fmi3GetVersion)
#define fmi3SetDebugLogging          fmi3FullName(fmi3SetDebugLogging)

/* Creation and destruction of FMU instances */
#define fmi3InstantiateModelExchange         fmi3FullName(fmi3InstantiateModelExchange)
#define fmi3InstantiateCoSimulation          fmi3FullName(fmi3InstantiateCoSimulation)
#define fmi3InstantiateScheduledExecution    fmi3FullName(fmi3InstantiateScheduledExecution)
#define fmi3FreeInstance                     fmi3FullName(fmi3FreeInstance)

/* Enter and exit initialization mode, terminate and reset */
#define fmi3EnterInitializationMode  fmi3FullName(fmi3EnterInitializationMode)
#define fmi3ExitInitializationMode   fmi3FullName(fmi3ExitInitializationMode)
#define fmi3EnterEventMode           fmi3FullName(fmi3EnterEventMode)
#define fmi3Terminate                fmi3FullName(fmi3Terminate)
#define fmi3Reset                    fmi3FullName(fmi3Reset)

/* Getting and setting variable values */
#define fmi3GetFloat32               fmi3FullName(fmi3GetFloat32)
#define fmi3GetFloat64               fmi3FullName(fmi3GetFloat64)
#define fmi3GetInt8                  fmi3FullName(fmi3GetInt8)
#define fmi3GetUInt8                 fmi3FullName(fmi3GetUInt8)
#define fmi3GetInt16                 fmi3FullName(fmi3GetInt16)
#define fmi3GetUInt16                fmi3FullName(fmi3GetUInt16)
#define fmi3GetInt32                 fmi3FullName(fmi3GetInt32)
#define fmi3GetUInt32                fmi3FullName(fmi3GetUInt32)
#define fmi3GetInt64                 fmi3FullName(fmi3GetInt64)
#define fmi3GetUInt64                fmi3FullName(fmi3GetUInt64)
#define fmi3GetBoolean               fmi3FullName(fmi3GetBoolean)
#define fmi3GetString                fmi3FullName(fmi3GetString)
#define fmi3GetBinary                fmi3FullName(fmi3GetBinary)
#define fmi3GetClock                 fmi3FullName(fmi3GetClock)
#define fmi3SetFloat32               fmi3FullName(fmi3SetFloat32)
#define fmi3SetFloat64               fmi3FullName(fmi3SetFloat64)
#define fmi3SetInt8                  fmi3FullName(fmi3SetInt8)
#define fmi3SetUInt8                 fmi3FullName(fmi3SetUInt8)
#define fmi3SetInt16                 fmi3FullName(fmi3SetInt16)
#define fmi3SetUInt16                fmi3FullName(fmi3SetUInt16)
#define fmi3SetInt32                 fmi3FullName(fmi3SetInt32)
#define fmi3SetUInt32                fmi3FullName(fmi3SetUInt32)
#define fmi3SetInt64                 fmi3FullName(fmi3SetInt64)
#define fmi3SetUInt64                fmi3FullName(fmi3SetUInt64)
#define fmi3SetBoolean               fmi3FullName(fmi3SetBoolean)
#define fmi3SetString                fmi3FullName(fmi3SetString)
#define fmi3SetBinary                fmi3FullName(fmi3SetBinary)
#define fmi3SetClock                 fmi3FullName(fmi3SetClock)

/* Getting Variable Dependency Information */
#define fmi3GetNumberOfVariableDependencies fmi3FullName(fmi3GetNumberOfVariableDependencies)
#define fmi3GetVariableDependencies         fmi3FullName(fmi3GetVariableDependencies)

/* Getting and setting the internal FMU state */
#define fmi3GetFMUState              fmi3FullName(fmi3GetFMUState)
#define fmi3SetFMUState              fmi3FullName(fmi3SetFMUState)
#define fmi3FreeFMUState             fmi3FullName(fmi3FreeFMUState)
#define fmi3SerializedFMUStateSize   fmi3FullName(fmi3SerializedFMUStateSize)
#define fmi3SerializeFMUState        fmi3FullName(fmi3SerializeFMUState)
#define fmi3DeserializeFMUState      fmi3FullName(fmi3DeserializeFMUState)

/* Getting partial derivatives */
#define fmi3GetDirectionalDerivative fmi3FullName(fmi3GetDirectionalDerivative)
#define fmi3GetAdjointDerivative     fmi3FullName(fmi3GetAdjointDerivative)

/* Entering and exiting the Configuration or Reconfiguration Mode */
#define fmi3EnterConfigurationMode   fmi3FullName(fmi3EnterConfigurationMode)
#define fmi3ExitConfigurationMode    fmi3FullName(fmi3ExitConfigurationMode)

/* Clock related functions */
#define fmi3GetIntervalDecimal       fmi3FullName(fmi3GetIntervalDecimal)
#define fmi3GetIntervalFraction      fmi3FullName(fmi3GetIntervalFraction)
#define fmi3GetShiftDecimal          fmi3FullName(fmi3GetShiftDecimal)
#define fmi3GetShiftFraction         fmi3FullName(fmi3GetShiftFraction)
#define fmi3SetIntervalDecimal       fmi3FullName(fmi3SetIntervalDecimal)
#define fmi3SetIntervalFraction      fmi3FullName(fmi3SetIntervalFraction)
#define fmi3SetShiftDecimal          fmi3FullName(fmi3SetShiftDecimal)
#define fmi3SetShiftFraction         fmi3FullName(fmi3SetShiftFraction)
#define fmi3EvaluateDiscreteStates   fmi3FullName(fmi3EvaluateDiscreteStates)
#define fmi3UpdateDiscreteStates     fmi3FullName(fmi3UpdateDiscreteStates)

/***************************************************
Functions for Model Exchange
****************************************************/

#define fmi3EnterContinuousTimeMode       fmi3FullName(fmi3EnterContinuousTimeMode)
#define fmi3CompletedIntegratorStep       fmi3FullName(fmi3CompletedIntegratorStep)

/* Providing independent variables and re-initialization of caching */
#define fmi3SetTime                       fmi3FullName(fmi3SetTime)
#define fmi3SetContinuousStates           fmi3FullName(fmi3SetContinuousStates)

/* Evaluation of the model equations */
#define fmi3GetContinuousStateDerivatives fmi3FullName(fmi3GetContinuousStateDerivatives)
#define fmi3GetEventIndicators            fmi3FullName(fmi3GetEventIndicators)
#define fmi3GetContinuousStates           fmi3FullName(fmi3GetContinuousStates)
#define fmi3GetNominalsOfContinuousStates fmi3FullName(fmi3GetNominalsOfContinuousStates)
#define fmi3GetNumberOfEventIndicators    fmi3FullName(fmi3GetNumberOfEventIndicators)
#define fmi3GetNumberOfContinuousStates   fmi3FullName(fmi3GetNumberOfContinuousStates)

/***************************************************
Functions for Co-Simulation
****************************************************/

/* Simulating the FMU */
#define fmi3EnterStepMode            fmi3FullName(fmi3EnterStepMode)
#define fmi3GetOutputDerivatives     fmi3FullName(fmi3GetOutputDerivatives)
#define fmi3DoStep                   fmi3FullName(fmi3DoStep)
#define fmi3ActivateModelPartition   fmi3FullName(fmi3ActivateModelPartition)

/***************************************************
Common Functions
****************************************************/

/* Inquire version numbers and set debug logging */
FMI3_Export fmi3GetVersionTYPE      fmi3GetVersion;
FMI3_Export fmi3SetDebugLoggingTYPE fmi3SetDebugLogging;

/* Creation and destruction of FMU instances */
FMI3_Export fmi3InstantiateModelExchangeTYPE         fmi3InstantiateModelExchange;
FMI3_Export fmi3InstantiateCoSimulationTYPE          fmi3InstantiateCoSimulation;
FMI3_Export fmi3InstantiateScheduledExecutionTYPE    fmi3InstantiateScheduledExecution;
FMI3_Export fmi3FreeInstanceTYPE                     fmi3FreeInstance;

/* Enter and exit initialization mode, terminate and reset */
FMI3_Export fmi3EnterInitializationModeTYPE fmi3EnterInitializationMode;
FMI3_Export fmi3ExitInitializationModeTYPE  fmi3ExitInitializationMode;
FMI3_Export fmi3EnterEventModeTYPE          fmi3EnterEventMode;
FMI3_Export fmi3TerminateTYPE               fmi3Terminate;
FMI3_Export fmi3ResetTYPE                   fmi3Reset;

/* Getting and setting variables values */
FMI3_Export fmi3GetFloat32TYPE fmi3GetFloat32;
FMI3_Export fmi3GetFloat64TYPE fmi3GetFloat64;
FMI3_Export fmi3GetInt8TYPE    fmi3GetInt8;
FMI3_Export fmi3GetUInt8TYPE   fmi3GetUInt8;
FMI3_Export fmi3GetInt16TYPE   fmi3GetInt16;
FMI3_Export fmi3GetUInt16TYPE  fmi3GetUInt16;
FMI3_Export fmi3GetInt32TYPE   fmi3GetInt32;
FMI3_Export fmi3GetUInt32TYPE  fmi3GetUInt32;
FMI3_Export fmi3GetInt64TYPE   fmi3GetInt64;
FMI3_Export fmi3GetUInt64TYPE  fmi3GetUInt64;
FMI3_Export fmi3GetBooleanTYPE fmi3GetBoolean;
FMI3_Export fmi3GetStringTYPE  fmi3GetString;
FMI3_Export fmi3GetBinaryTYPE  fmi3GetBinary;
FMI3_Export fmi3GetClockTYPE   fmi3GetClock;
FMI3_Export fmi3SetFloat32TYPE fmi3SetFloat32;
FMI3_Export fmi3SetFloat64TYPE fmi3SetFloat64;
FMI3_Export fmi3SetInt8TYPE    fmi3SetInt8;
FMI3_Export fmi3SetUInt8TYPE   fmi3SetUInt8;
FMI3_Export fmi3SetInt16TYPE   fmi3SetInt16;
FMI3_Export fmi3SetUInt16TYPE  fmi3SetUInt16;
FMI3_Export fmi3SetInt32TYPE   fmi3SetInt32;
FMI3_Export fmi3SetUInt32TYPE  fmi3SetUInt32;
FMI3_Export fmi3SetInt64TYPE   fmi3SetInt64;
FMI3_Export fmi3SetUInt64TYPE  fmi3SetUInt64;
FMI3_Export fmi3SetBooleanTYPE fmi3SetBoolean;
FMI3_Export fmi3SetStringTYPE  fmi3SetString;
FMI3_Export fmi3SetBinaryTYPE  fmi3SetBinary;
FMI3_Export fmi3SetClockTYPE   fmi3SetClock;

/* Getting Variable Dependency Information */
FMI3_Export fmi3GetNumberOfVariableDependenciesTYPE fmi3GetNumberOfVariableDependencies;
FMI3_Export fmi3GetVariableDependenciesTYPE         fmi3GetVariableDependencies;

/* Getting and setting the internal FMU state */
FMI3_Export fmi3GetFMUStateTYPE            fmi3GetFMUState;
FMI3_Export fmi3SetFMUStateTYPE            fmi3SetFMUState;
FMI3_Export fmi3FreeFMUStateTYPE           fmi3FreeFMUState;
FMI3_Export fmi3SerializedFMUStateSizeTYPE fmi3SerializedFMUStateSize;
FMI3_Export fmi3SerializeFMUStateTYPE      fmi3SerializeFMUState;
FMI3_Export fmi3DeserializeFMUStateTYPE    fmi3DeserializeFMUState;

/* Getting partial derivatives */
FMI3_Export fmi3GetDirectionalDerivativeTYPE fmi3GetDirectionalDerivative;
FMI3_Export fmi3GetAdjointDerivativeTYPE     fmi3GetAdjointDerivative;

/* Entering and exiting the Configuration or Reconfiguration Mode */
FMI3_Export fmi3EnterConfigurationModeTYPE fmi3EnterConfigurationMode;
FMI3_Export fmi3ExitConfigurationModeTYPE  fmi3ExitConfigurationMode;

/* Clock related functions */
FMI3_Export fmi3GetIntervalDecimalTYPE     fmi3GetIntervalDecimal;
FMI3_Export fmi3GetIntervalFractionTYPE    fmi3GetIntervalFraction;
FMI3_Export fmi3GetShiftDecimalTYPE        fmi3GetShiftDecimal;
FMI3_Export fmi3GetShiftFractionTYPE       fmi3GetShiftFraction;
FMI3_Export fmi3SetIntervalDecimalTYPE     fmi3SetIntervalDecimal;
FMI3_Export fmi3SetIntervalFractionTYPE    fmi3SetIntervalFraction;
FMI3_Export fmi3SetShiftDecimalTYPE        fmi3SetShiftDecimal;
FMI3_Export fmi3SetShiftFractionTYPE       fmi3SetShiftFraction;
FMI3_Export fmi3EvaluateDiscreteStatesTYPE fmi3EvaluateDiscreteStates;
FMI3_Export fmi3UpdateDiscreteStatesTYPE   fmi3UpdateDiscreteStates;
)"
        R"(
/***************************************************
Functions for Model Exchange
****************************************************/

FMI3_Export fmi3EnterContinuousTimeModeTYPE fmi3EnterContinuousTimeMode;
FMI3_Export fmi3CompletedIntegratorStepTYPE fmi3CompletedIntegratorStep;

/* Providing independent variables and re-initialization of caching */
/* tag::SetTimeTYPE[] */
FMI3_Export fmi3SetTimeTYPE             fmi3SetTime;
/* end::SetTimeTYPE[] */
FMI3_Export fmi3SetContinuousStatesTYPE fmi3SetContinuousStates;

/* Evaluation of the model equations */
FMI3_Export fmi3GetContinuousStateDerivativesTYPE fmi3GetContinuousStateDerivatives;
FMI3_Export fmi3GetEventIndicatorsTYPE            fmi3GetEventIndicators;
FMI3_Export fmi3GetContinuousStatesTYPE           fmi3GetContinuousStates;
FMI3_Export fmi3GetNominalsOfContinuousStatesTYPE fmi3GetNominalsOfContinuousStates;
FMI3_Export fmi3GetNumberOfEventIndicatorsTYPE    fmi3GetNumberOfEventIndicators;
FMI3_Export fmi3GetNumberOfContinuousStatesTYPE   fmi3GetNumberOfContinuousStates;

/***************************************************
Functions for Co-Simulation
****************************************************/

/* Simulating the FMU */
FMI3_Export fmi3EnterStepModeTYPE        fmi3EnterStepMode;
FMI3_Export fmi3GetOutputDerivativesTYPE fmi3GetOutputDerivatives;
FMI3_Export fmi3DoStepTYPE               fmi3DoStep;

/***************************************************
Functions for Scheduled Execution
****************************************************/

FMI3_Export fmi3ActivateModelPartitionTYPE fmi3ActivateModelPartition;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* fmi3Functions_h */
)");

    return s.c_str();
}

inline const char* ecos::fmi3FunctionTypes()
{
    static std::string s(

        R"(
#ifndef fmi3FunctionTypes_h
#define fmi3FunctionTypes_h

#include "fmi3PlatformTypes.h"

/*
This header file defines the data and function types of FMI 3.0.
It must be used when compiling an FMU or an FMI importer.

Copyright (C) 2011 MODELISAR consortium,
              2012-2024 Modelica Association Project "FMI"
              All rights reserved.

This file is licensed by the copyright holders under the 2-Clause BSD License
(https://opensource.org/licenses/BSD-2-Clause):

----------------------------------------------------------------------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Include stddef.h, in order that size_t etc. is defined */
#include <stddef.h>


/* Type definitions */

/* tag::Status[] */
typedef enum {
    fmi3OK,
    fmi3Warning,
    fmi3Discard,
    fmi3Error,
    fmi3Fatal,
} fmi3Status;
/* end::Status[] */

/* tag::DependencyKind[] */
typedef enum {
    fmi3Independent,
    fmi3Constant,
    fmi3Fixed,
    fmi3Tunable,
    fmi3Discrete,
    fmi3Dependent
} fmi3DependencyKind;
/* end::DependencyKind[] */

/* tag::IntervalQualifier[] */
typedef enum {
    fmi3IntervalNotYetKnown,
    fmi3IntervalUnchanged,
    fmi3IntervalChanged
} fmi3IntervalQualifier;
/* end::IntervalQualifier[] */

/* tag::CallbackLogMessage[] */
typedef void  (*fmi3LogMessageCallback) (fmi3InstanceEnvironment instanceEnvironment,
                                         fmi3Status status,
                                         fmi3String category,
                                         fmi3String message);
/* end::CallbackLogMessage[] */

/* tag::CallbackClockUpdate[] */
typedef void (*fmi3ClockUpdateCallback) (
    fmi3InstanceEnvironment  instanceEnvironment);
/* end::CallbackClockUpdate[] */

/* tag::CallbackIntermediateUpdate[] */
typedef void (*fmi3IntermediateUpdateCallback) (
    fmi3InstanceEnvironment instanceEnvironment,
    fmi3Float64  intermediateUpdateTime,
    fmi3Boolean  intermediateVariableSetRequested,
    fmi3Boolean  intermediateVariableGetAllowed,
    fmi3Boolean  intermediateStepFinished,
    fmi3Boolean  canReturnEarly,
    fmi3Boolean* earlyReturnRequested,
    fmi3Float64* earlyReturnTime);
/* end::CallbackIntermediateUpdate[] */

/* tag::CallbackPreemptionLock[] */
typedef void (*fmi3LockPreemptionCallback)   (void);
typedef void (*fmi3UnlockPreemptionCallback) (void);
/* end::CallbackPreemptionLock[] */

/* Define fmi3 function pointer types to simplify dynamic loading */
)"
        R"(
/***************************************************
Types for Common Functions
****************************************************/

/* Inquire version numbers and setting logging status */
/* tag::GetVersion[] */
typedef const char* fmi3GetVersionTYPE(void);
/* end::GetVersion[] */

/* tag::SetDebugLogging[] */
typedef fmi3Status fmi3SetDebugLoggingTYPE(fmi3Instance instance,
                                           fmi3Boolean loggingOn,
                                           size_t nCategories,
                                           const fmi3String categories[]);
/* end::SetDebugLogging[] */

/* Creation and destruction of FMU instances and setting debug status */
/* tag::Instantiate[] */
typedef fmi3Instance fmi3InstantiateModelExchangeTYPE(
    fmi3String                 instanceName,
    fmi3String                 instantiationToken,
    fmi3String                 resourcePath,
    fmi3Boolean                visible,
    fmi3Boolean                loggingOn,
    fmi3InstanceEnvironment    instanceEnvironment,
    fmi3LogMessageCallback     logMessage);

typedef fmi3Instance fmi3InstantiateCoSimulationTYPE(
    fmi3String                     instanceName,
    fmi3String                     instantiationToken,
    fmi3String                     resourcePath,
    fmi3Boolean                    visible,
    fmi3Boolean                    loggingOn,
    fmi3Boolean                    eventModeUsed,
    fmi3Boolean                    earlyReturnAllowed,
    const fmi3ValueReference       requiredIntermediateVariables[],
    size_t                         nRequiredIntermediateVariables,
    fmi3InstanceEnvironment        instanceEnvironment,
    fmi3LogMessageCallback         logMessage,
    fmi3IntermediateUpdateCallback intermediateUpdate);

typedef fmi3Instance fmi3InstantiateScheduledExecutionTYPE(
    fmi3String                     instanceName,
    fmi3String                     instantiationToken,
    fmi3String                     resourcePath,
    fmi3Boolean                    visible,
    fmi3Boolean                    loggingOn,
    fmi3InstanceEnvironment        instanceEnvironment,
    fmi3LogMessageCallback         logMessage,
    fmi3ClockUpdateCallback        clockUpdate,
    fmi3LockPreemptionCallback     lockPreemption,
    fmi3UnlockPreemptionCallback   unlockPreemption);
/* end::Instantiate[] */

/* tag::FreeInstance[] */
typedef void fmi3FreeInstanceTYPE(fmi3Instance instance);
/* end::FreeInstance[] */

/* Enter and exit initialization mode, enter event mode, terminate and reset */
/* tag::EnterInitializationMode[] */
typedef fmi3Status fmi3EnterInitializationModeTYPE(fmi3Instance instance,
                                                   fmi3Boolean toleranceDefined,
                                                   fmi3Float64 tolerance,
                                                   fmi3Float64 startTime,
                                                   fmi3Boolean stopTimeDefined,
                                                   fmi3Float64 stopTime);
/* end::EnterInitializationMode[] */

/* tag::ExitInitializationMode[] */
typedef fmi3Status fmi3ExitInitializationModeTYPE(fmi3Instance instance);
/* end::ExitInitializationMode[] */

/* tag::EnterEventMode[] */
typedef fmi3Status fmi3EnterEventModeTYPE(fmi3Instance instance);
/* end::EnterEventMode[] */

/* tag::Terminate[] */
typedef fmi3Status fmi3TerminateTYPE(fmi3Instance instance);
/* end::Terminate[] */

/* tag::Reset[] */
typedef fmi3Status fmi3ResetTYPE(fmi3Instance instance);
/* end::Reset[] */

/* Getting and setting variable values */
/* tag::Getters[] */
typedef fmi3Status fmi3GetFloat32TYPE(fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Float32 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetFloat64TYPE(fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Float64 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetInt8TYPE   (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Int8 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetUInt8TYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3UInt8 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetInt16TYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Int16 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetUInt16TYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3UInt16 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetInt32TYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Int32 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetUInt32TYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3UInt32 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetInt64TYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Int64 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetUInt64TYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3UInt64 values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetBooleanTYPE(fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Boolean values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetStringTYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3String values[],
                                      size_t nValues);

typedef fmi3Status fmi3GetBinaryTYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      size_t valueSizes[],
                                      fmi3Binary values[],
                                      size_t nValues);
/* end::Getters[] */
)"
        R"(
/* tag::GetClock[] */
typedef fmi3Status fmi3GetClockTYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Clock values[]);
/* end::GetClock[] */

/* tag::Setters[] */
typedef fmi3Status fmi3SetFloat32TYPE(fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Float32 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetFloat64TYPE(fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Float64 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetInt8TYPE   (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Int8 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetUInt8TYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3UInt8 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetInt16TYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Int16 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetUInt16TYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3UInt16 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetInt32TYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Int32 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetUInt32TYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3UInt32 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetInt64TYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Int64 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetUInt64TYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3UInt64 values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetBooleanTYPE(fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Boolean values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetStringTYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3String values[],
                                      size_t nValues);

typedef fmi3Status fmi3SetBinaryTYPE (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const size_t valueSizes[],
                                      const fmi3Binary values[],
                                      size_t nValues);
/* end::Setters[] */
/* tag::SetClock[] */
typedef fmi3Status fmi3SetClockTYPE  (fmi3Instance instance,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Clock values[]);
/* end::SetClock[] */

/* Getting Variable Dependency Information */
/* tag::GetNumberOfVariableDependencies[] */
typedef fmi3Status fmi3GetNumberOfVariableDependenciesTYPE(fmi3Instance instance,
                                                           fmi3ValueReference valueReference,
                                                           size_t* nDependencies);
/* end::GetNumberOfVariableDependencies[] */

/* tag::GetVariableDependencies[] */
typedef fmi3Status fmi3GetVariableDependenciesTYPE(fmi3Instance instance,
                                                   fmi3ValueReference dependent,
                                                   size_t elementIndicesOfDependent[],
                                                   fmi3ValueReference independents[],
                                                   size_t elementIndicesOfIndependents[],
                                                   fmi3DependencyKind dependencyKinds[],
                                                   size_t nDependencies);
/* end::GetVariableDependencies[] */

/* Getting and setting the internal FMU state */
/* tag::GetFMUState[] */
typedef fmi3Status fmi3GetFMUStateTYPE (fmi3Instance instance, fmi3FMUState* FMUState);
/* end::GetFMUState[] */

/* tag::SetFMUState[] */
typedef fmi3Status fmi3SetFMUStateTYPE (fmi3Instance instance, fmi3FMUState  FMUState);
/* end::SetFMUState[] */

/* tag::FreeFMUState[] */
typedef fmi3Status fmi3FreeFMUStateTYPE(fmi3Instance instance, fmi3FMUState* FMUState);
/* end::FreeFMUState[] */

/* tag::SerializedFMUStateSize[] */
typedef fmi3Status fmi3SerializedFMUStateSizeTYPE(fmi3Instance instance,
                                                  fmi3FMUState FMUState,
                                                  size_t* size);
/* end::SerializedFMUStateSize[] */

/* tag::SerializeFMUState[] */
typedef fmi3Status fmi3SerializeFMUStateTYPE     (fmi3Instance instance,
                                                  fmi3FMUState FMUState,
                                                  fmi3Byte serializedState[],
                                                  size_t size);
/* end::SerializeFMUState[] */

/* tag::DeserializeFMUState[] */
typedef fmi3Status fmi3DeserializeFMUStateTYPE   (fmi3Instance instance,
                                                  const fmi3Byte serializedState[],
                                                  size_t size,
                                                  fmi3FMUState* FMUState);
/* end::DeserializeFMUState[] */
)"
        R"(
/* Getting partial derivatives */
/* tag::GetDirectionalDerivative[] */
typedef fmi3Status fmi3GetDirectionalDerivativeTYPE(fmi3Instance instance,
                                                    const fmi3ValueReference unknowns[],
                                                    size_t nUnknowns,
                                                    const fmi3ValueReference knowns[],
                                                    size_t nKnowns,
                                                    const fmi3Float64 seed[],
                                                    size_t nSeed,
                                                    fmi3Float64 sensitivity[],
                                                    size_t nSensitivity);
/* end::GetDirectionalDerivative[] */

/* tag::GetAdjointDerivative[] */
typedef fmi3Status fmi3GetAdjointDerivativeTYPE(fmi3Instance instance,
                                                const fmi3ValueReference unknowns[],
                                                size_t nUnknowns,
                                                const fmi3ValueReference knowns[],
                                                size_t nKnowns,
                                                const fmi3Float64 seed[],
                                                size_t nSeed,
                                                fmi3Float64 sensitivity[],
                                                size_t nSensitivity);
/* end::GetAdjointDerivative[] */

/* Entering and exiting the Configuration or Reconfiguration Mode */

/* tag::EnterConfigurationMode[] */
typedef fmi3Status fmi3EnterConfigurationModeTYPE(fmi3Instance instance);
/* end::EnterConfigurationMode[] */

/* tag::ExitConfigurationMode[] */
typedef fmi3Status fmi3ExitConfigurationModeTYPE(fmi3Instance instance);
/* end::ExitConfigurationMode[] */

/* tag::GetIntervalDecimal[] */
typedef fmi3Status fmi3GetIntervalDecimalTYPE(fmi3Instance instance,
                                              const fmi3ValueReference valueReferences[],
                                              size_t nValueReferences,
                                              fmi3Float64 intervals[],
                                              fmi3IntervalQualifier qualifiers[]);
/* end::GetIntervalDecimal[] */

/* tag::GetIntervalFraction[] */
typedef fmi3Status fmi3GetIntervalFractionTYPE(fmi3Instance instance,
                                               const fmi3ValueReference valueReferences[],
                                               size_t nValueReferences,
                                               fmi3UInt64 counters[],
                                               fmi3UInt64 resolutions[],
                                               fmi3IntervalQualifier qualifiers[]);
/* end::GetIntervalFraction[] */

/* tag::GetShiftDecimal[] */
typedef fmi3Status fmi3GetShiftDecimalTYPE(fmi3Instance instance,
                                           const fmi3ValueReference valueReferences[],
                                           size_t nValueReferences,
                                           fmi3Float64 shifts[]);
/* end::GetShiftDecimal[] */

/* tag::GetShiftFraction[] */
typedef fmi3Status fmi3GetShiftFractionTYPE(fmi3Instance instance,
                                            const fmi3ValueReference valueReferences[],
                                            size_t nValueReferences,
                                            fmi3UInt64 counters[],
                                            fmi3UInt64 resolutions[]);
/* end::GetShiftFraction[] */

/* tag::SetIntervalDecimal[] */
typedef fmi3Status fmi3SetIntervalDecimalTYPE(fmi3Instance instance,
                                              const fmi3ValueReference valueReferences[],
                                              size_t nValueReferences,
                                              const fmi3Float64 intervals[]);
/* end::SetIntervalDecimal[] */

/* tag::SetIntervalFraction[] */
typedef fmi3Status fmi3SetIntervalFractionTYPE(fmi3Instance instance,
                                               const fmi3ValueReference valueReferences[],
                                               size_t nValueReferences,
                                               const fmi3UInt64 counters[],
                                               const fmi3UInt64 resolutions[]);
/* end::SetIntervalFraction[] */

/* tag::SetShiftDecimal[] */
typedef fmi3Status fmi3SetShiftDecimalTYPE(fmi3Instance instance,
                                           const fmi3ValueReference valueReferences[],
                                           size_t nValueReferences,
                                           const fmi3Float64 shifts[]);
/* end::SetShiftDecimal[] */

/* tag::SetShiftFraction[] */
typedef fmi3Status fmi3SetShiftFractionTYPE(fmi3Instance instance,
                                            const fmi3ValueReference valueReferences[],
                                            size_t nValueReferences,
                                            const fmi3UInt64 counters[],
                                            const fmi3UInt64 resolutions[]);
/* end::SetShiftFraction[] */

/* tag::EvaluateDiscreteStates[] */
typedef fmi3Status fmi3EvaluateDiscreteStatesTYPE(fmi3Instance instance);
/* end::EvaluateDiscreteStates[] */

/* tag::UpdateDiscreteStates[] */
typedef fmi3Status fmi3UpdateDiscreteStatesTYPE(fmi3Instance instance,
                                                fmi3Boolean* discreteStatesNeedUpdate,
                                                fmi3Boolean* terminateSimulation,
                                                fmi3Boolean* nominalsOfContinuousStatesChanged,
                                                fmi3Boolean* valuesOfContinuousStatesChanged,
                                                fmi3Boolean* nextEventTimeDefined,
                                                fmi3Float64* nextEventTime);
/* end::UpdateDiscreteStates[] */
)"
        R"(
/***************************************************
Types for Functions for Model Exchange
****************************************************/

/* tag::EnterContinuousTimeMode[] */
typedef fmi3Status fmi3EnterContinuousTimeModeTYPE(fmi3Instance instance);
/* end::EnterContinuousTimeMode[] */

/* tag::CompletedIntegratorStep[] */
typedef fmi3Status fmi3CompletedIntegratorStepTYPE(fmi3Instance instance,
                                                   fmi3Boolean  noSetFMUStatePriorToCurrentPoint,
                                                   fmi3Boolean* enterEventMode,
                                                   fmi3Boolean* terminateSimulation);
/* end::CompletedIntegratorStep[] */

/* Providing independent variables and re-initialization of caching */
/* tag::SetTime[] */
typedef fmi3Status fmi3SetTimeTYPE(fmi3Instance instance, fmi3Float64 time);
/* end::SetTime[] */

/* tag::SetContinuousStates[] */
typedef fmi3Status fmi3SetContinuousStatesTYPE(fmi3Instance instance,
                                               const fmi3Float64 continuousStates[],
                                               size_t nContinuousStates);
/* end::SetContinuousStates[] */

/* Evaluation of the model equations */
/* tag::GetDerivatives[] */
typedef fmi3Status fmi3GetContinuousStateDerivativesTYPE(fmi3Instance instance,
                                                         fmi3Float64 derivatives[],
                                                         size_t nContinuousStates);
/* end::GetDerivatives[] */

/* tag::GetEventIndicators[] */
typedef fmi3Status fmi3GetEventIndicatorsTYPE(fmi3Instance instance,
                                              fmi3Float64 eventIndicators[],
                                              size_t nEventIndicators);
/* end::GetEventIndicators[] */

/* tag::GetContinuousStates[] */
typedef fmi3Status fmi3GetContinuousStatesTYPE(fmi3Instance instance,
                                               fmi3Float64 continuousStates[],
                                               size_t nContinuousStates);
/* end::GetContinuousStates[] */

/* tag::GetNominalsOfContinuousStates[] */
typedef fmi3Status fmi3GetNominalsOfContinuousStatesTYPE(fmi3Instance instance,
                                                         fmi3Float64 nominals[],
                                                         size_t nContinuousStates);
/* end::GetNominalsOfContinuousStates[] */

/* tag::GetNumberOfEventIndicators[] */
typedef fmi3Status fmi3GetNumberOfEventIndicatorsTYPE(fmi3Instance instance,
                                                      size_t* nEventIndicators);
/* end::GetNumberOfEventIndicators[] */

/* tag::GetNumberOfContinuousStates[] */
typedef fmi3Status fmi3GetNumberOfContinuousStatesTYPE(fmi3Instance instance,
                                                       size_t* nContinuousStates);
/* end::GetNumberOfContinuousStates[] */
)"
        R"(
/***************************************************
Types for Functions for Co-Simulation
****************************************************/

/* Simulating the FMU */

/* tag::EnterStepMode[] */
typedef fmi3Status fmi3EnterStepModeTYPE(fmi3Instance instance);
/* end::EnterStepMode[] */

/* tag::GetOutputDerivatives[] */
typedef fmi3Status fmi3GetOutputDerivativesTYPE(fmi3Instance instance,
                                                const fmi3ValueReference valueReferences[],
                                                size_t nValueReferences,
                                                const fmi3Int32 orders[],
                                                fmi3Float64 values[],
                                                size_t nValues);
/* end::GetOutputDerivatives[] */

/* tag::DoStep[] */
typedef fmi3Status fmi3DoStepTYPE(fmi3Instance instance,
                                  fmi3Float64 currentCommunicationPoint,
                                  fmi3Float64 communicationStepSize,
                                  fmi3Boolean noSetFMUStatePriorToCurrentPoint,
                                  fmi3Boolean* eventHandlingNeeded,
                                  fmi3Boolean* terminateSimulation,
                                  fmi3Boolean* earlyReturn,
                                  fmi3Float64* lastSuccessfulTime);
/* end::DoStep[] */

/***************************************************
Types for Functions for Scheduled Execution
****************************************************/

/* tag::ActivateModelPartition[] */
typedef fmi3Status fmi3ActivateModelPartitionTYPE(fmi3Instance instance,
                                                  fmi3ValueReference clockReference,
                                                  fmi3Float64 activationTime);
/* end::ActivateModelPartition[] */

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* fmi3FunctionTypes_h */
)");

    return s.c_str();
}

inline const char* ecos::fmi3PlatformTypes()
{
    static std::string s = R"(
#ifndef fmi3PlatformTypes_h
#define fmi3PlatformTypes_h

/*
This header file defines the data types of FMI 3.0.
It must be used by both FMU and FMI master.

Copyright (C) 2008-2011 MODELISAR consortium,
              2012-2024 Modelica Association Project "FMI"
              All rights reserved.

This file is licensed by the copyright holders under the 2-Clause BSD License
(https://opensource.org/licenses/BSD-2-Clause):

----------------------------------------------------------------------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
----------------------------------------------------------------------------
*/

/* Include the integer and boolean type definitions */
#include <stdint.h>
#include <stdbool.h>


/* tag::Component[] */
typedef           void* fmi3Instance;             /* Pointer to the FMU instance */
/* end::Component[] */

/* tag::ComponentEnvironment[] */
typedef           void* fmi3InstanceEnvironment;  /* Pointer to the FMU environment */
/* end::ComponentEnvironment[] */

/* tag::FMUState[] */
typedef           void* fmi3FMUState;             /* Pointer to the internal FMU state */
/* end::FMUState[] */

/* tag::ValueReference[] */
typedef        uint32_t fmi3ValueReference;       /* Handle to the value of a variable */
/* end::ValueReference[] */

/* tag::VariableTypes[] */
typedef           float fmi3Float32;  /* Single precision floating point (32-bit) */
/* tag::fmi3Float64[] */
typedef          double fmi3Float64;  /* Double precision floating point (64-bit) */
/* end::fmi3Float64[] */
typedef          int8_t fmi3Int8;     /* 8-bit signed integer */
typedef         uint8_t fmi3UInt8;    /* 8-bit unsigned integer */
typedef         int16_t fmi3Int16;    /* 16-bit signed integer */
typedef        uint16_t fmi3UInt16;   /* 16-bit unsigned integer */
typedef         int32_t fmi3Int32;    /* 32-bit signed integer */
typedef        uint32_t fmi3UInt32;   /* 32-bit unsigned integer */
typedef         int64_t fmi3Int64;    /* 64-bit signed integer */
typedef        uint64_t fmi3UInt64;   /* 64-bit unsigned integer */
typedef            bool fmi3Boolean;  /* Data type to be used with fmi3True and fmi3False */
typedef            char fmi3Char;     /* Data type for one character */
typedef const fmi3Char* fmi3String;   /* Data type for character strings
                                         ('\0' terminated, UTF-8 encoded) */
typedef         uint8_t fmi3Byte;     /* Smallest addressable unit of the machine
                                         (typically one byte) */
typedef const fmi3Byte* fmi3Binary;   /* Data type for binary data
                                         (out-of-band length terminated) */
typedef            bool fmi3Clock;    /* Data type to be used with fmi3ClockActive and
                                         fmi3ClockInactive */

/* Values for fmi3Boolean */
#define fmi3True  true
#define fmi3False false

/* Values for fmi3Clock */
#define fmi3ClockActive   true
#define fmi3ClockInactive false
/* end::VariableTypes[] */

#endif /* fmi3PlatformTypes_h */

)";

    return s.c_str();
}


#endif // LIBECOS_COMPILE_HPP
