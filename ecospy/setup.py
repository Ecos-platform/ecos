import os
import subprocess
from pathlib import Path

from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext

WINDOWS = (os.name == 'nt')

build_folder = "build"
bin_folder = f"{build_folder}/bin"
bin_files = [str(p) for p in Path(bin_folder).glob("*") if p.suffix in {".so", ".dll"}]

def version():
    with open("../version.txt", "r") as f:
        return f.readline().strip()


class CMakeExtension(Extension):

    def __init__(self, name):
        super().__init__(
            name,
            sources=[]
        )


class CMakeBuild(build_ext):

    def run(self):

        if os.path.exists(build_folder):
            print("CMake build folder exists, skipping redundant build.")
        else:
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
            build_folder,
            '-DCMAKE_BUILD_TYPE={}'.format(build_type),
        ]
        if WINDOWS:
            cmake_args.extend(['-A', 'x64'])

        subprocess.check_call(cmake_args)
        cmake_args_build = [
            'cmake',
            '--build',
            build_folder
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
      author="Lars Ivar Hatledal",
      license="MIT",
      include_package_data=False,
      packages=find_packages(),
      data_files=[
          ("Scripts", [f"{build_folder}/bin/proxyfmu{binary_suffix()}", f"{build_folder}/bin/ecos{binary_suffix()}"]),
          ("ecospy/build/bin", bin_files)
      ],
      ext_modules=[CMakeExtension("ecospy")],
      cmdclass=dict(build_ext=CMakeBuild),
      )
