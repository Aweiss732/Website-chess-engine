from setuptools import setup, Extension
import pybind11
import os

ext_modules = [
    Extension(
        "engine_core",
        ["engine/engine_core.cpp"],
        include_dirs=[pybind11.get_include(), os.path.abspath("engine"), os.path.abspath("engine/virgo")],
        language="c++",
        extra_compile_args=["-g", "-O0", "-std=c++17", "-fexceptions", "-fno-omit-frame-pointer"],
    )
]

setup(name="engine_core", ext_modules=ext_modules)
