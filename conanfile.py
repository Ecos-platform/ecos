from conans import ConanFile, CMake, tools
from os import path


class EcosConan(ConanFile):
    name = "libecos"
    author = "Lars Ivar Hatledal"
    license = "MIT"
    exports = "version.txt"
    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
    }
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    requires = (
        "libzip/1.8.0",
        "spdlog/1.10.0",
        "proxyfmu/0.3.1@ais/testing-fix_linking_test",
        "simple-ssp-parser/0.2.9@ais/testing",
        # conflict resolution
        "openssl/1.1.1n"
    )

    def set_version(self):
        self.version = tools.load(path.join(self.recipe_folder, "version.txt")).strip()

    def imports(self):
        self.copy("proxyfmu*", dst="bin", src="bin", keep_path=False)

    def configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["ECOS_BUILD_TESTS"] = "OFF"
        cmake.definitions["ECOS_BUILD_EXAMPLES"] = "OFF"
        cmake.configure()
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["libecos"]
