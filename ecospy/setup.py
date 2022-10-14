import os
import subprocess
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

WINDOWS = (os.name == 'nt')

buildFolder = "build"


def version():
    with open("../version.txt", "r") as f:
        return f.readline().strip()


class CMakeExtension(Extension):

    def __init__(self, name):
        super().__init__(name, sources=[])


class CMakeBuild(build_ext):

    def run(self):
        for ext in self.extensions:
            self.build_extension(ext)
        super().run()

    def build_extension(self, ext):

        build_type = 'Release'
        # configure
        cmake_args = [
            'cmake',
            '..',
            '-B',
            buildFolder,
            '-DCMAKE_BUILD_TYPE={}'.format(build_type)
        ]
        if WINDOWS:
            cmake_args.extend(['-G', 'Visual Studio 16 2019', '-A', 'x64'])

        subprocess.check_call(cmake_args)
        cmake_args_build = [
            'cmake',
            '--build',
            buildFolder
        ]
        if WINDOWS:
            cmake_args_build.extend(['--config', 'Release'])
        subprocess.check_call(cmake_args_build)


def binary_suffix():
    return ".exe" if WINDOWS else ""


setup(name="ecospy",
    version=version(),
    description="Ecos co-simulation library",
    url="https://github.com/Ecos-platform/ecos",
    license="MIT",
    include_package_data=True,
    packages=['ecospy'],
    package_dir={'ecospy': '.'},
    package_data={'ecospy': [f"{buildFolder}/bin/*.dll"]},
    data_files=[("Scripts", [f"{buildFolder}/bin/proxyfmu{binary_suffix()}", f"{buildFolder}/bin/ecos{binary_suffix()}"])],
    ext_modules=[CMakeExtension('ecospy')],
    cmdclass=dict(build_ext=CMakeBuild),
)
