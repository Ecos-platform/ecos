
import glob
from setuptools import setup


def version():
    with open("version.txt", "r") as f:
        return f.readline().strip()


def binaries():
    return glob.glob("ecospy/binaries/ecos*") + glob.glob("ecospy/binaries/proxyfmu*")


setup(name="ecospy",
      version=version(),
      data_files=[
          ("Scripts", binaries())
      ]
      )
