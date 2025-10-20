# setup.py
from setuptools import setup, Extension
import pybind11
import os

engine_dir = os.path.abspath("engine")
virgo_dir = os.path.join(engine_dir, "virgo")

ext_modules = [
    # Chess engine
    Extension(
        "engine_core",
        [os.path.join(engine_dir, "engine_core.cpp")],
        include_dirs=[pybind11.get_include(), engine_dir, virgo_dir],
        language="c++",
        extra_compile_args=[
            "-O3",
            "-std=c++17",
            "-fexceptions",
            "-fno-omit-frame-pointer",
        ],
    ),

    # Connect-Five engine
    Extension(
        "engine_connect5_core",
        [os.path.join(engine_dir, "engine_connect5_core.cpp")],
        include_dirs=[pybind11.get_include(), engine_dir],
        language="c++",
        extra_compile_args=[
            "-O3",
            "-std=c++17",
            "-fexceptions",
            "-fno-omit-frame-pointer",
        ],
    ),
]

setup(
    name="engine_bundle",
    version="1.0",
    description="Combined C++ engines (chess + connect five)",
    ext_modules=ext_modules,
)