import os
from setuptools import setup, find_packages

WINDOWS = (os.name == 'nt')


def version():
    with open("version.txt", "r") as f:
        return f.readline().strip()


def binary_suffix():
    return ".exe" if WINDOWS else ""


setup(name="ecospy",
      version=version(),
      data_files=[
          ("Scripts", [f"binaries/proxyfmu{binary_suffix()}", f"binaries/ecos{binary_suffix()}"])
      ]
      )
