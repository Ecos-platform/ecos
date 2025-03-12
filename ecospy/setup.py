
from setuptools import setup

def version():
    with open("../version.txt", "r") as f:
        return f.readline().strip()

setup(name="ecospy",
      version=version()
      )
