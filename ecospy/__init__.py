import os
from ctypes import *


class Version(Structure):
    _fields_ = [("major", c_int), ("minor", c_int), ("patch", c_int)]


def loadlib():

    def suffix() -> str:
        if os.name == "nt":
            return ".dll"
        else:
            return ".so"

    # path = os.path.abspath(f"libecosc{suffix()}")
    return CDLL(f"{__file__}/../build-pip/bin/libecosc{suffix()}")


if __name__ == "__main__":
    lib = loadlib()
    pass
