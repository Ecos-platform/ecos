from conans import ConanFile, CMake, tools
from os import path


class VicoConan(ConanFile):
    name = "vico"
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
        "fmilibcpp/0.1.0@ais/testing",
        "proxyfmu/0.2.3@osp/stable"
    )

    def set_version(self):
        self.version = tools.load(path.join(self.recipe_folder, "version.txt")).strip()

    def imports(self):
        self.copy("proxyfmu*", dst="bin", src="bin", keep_path=False)

    def configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["vico"]
