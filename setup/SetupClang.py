import sys
import subprocess
import importlib.util as importlib_util

class ClangConfiguration:
    @classmethod
    def Validate(cls):
        if not cls.__ValidateClang():
            return # cannot validate further

        for packageName in ["requests"]:
            if not cls.__ValidatePackage(packageName):
                return # cannot validate further

    @classmethod
    def __ValidateClang(cls, versionMajor = 3, versionMinor = 3):
        pass
    # @TODO 在这里拉clang并且下载