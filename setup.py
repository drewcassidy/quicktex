import os
import sys
import glob
import subprocess

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext

project_path = os.path.dirname(os.path.realpath(__file__))

with open(os.path.join(project_path, 'README.md')) as f:
    readme = f.read()


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        from setuptools_scm import get_version
        version = get_version(root='.', relative_to=__file__)

        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        # required for auto-detection of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        cfg = "Debug" if self.debug else "Release"

        # CMake lets you override the generator - we need to check this.
        # Can be set with Conda-Build, for example.
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        # Set Python_EXECUTABLE instead if you use PYBIND11_FINDPYTHON
        cmake_args = [
            "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={}".format(extdir),
            "-DPython_EXECUTABLE={}".format(sys.executable),
            "-DQUICKTEX_VERSION_INFO={}".format(version),
            "-DCMAKE_BUILD_TYPE={}".format(cfg),  # not used on MSVC, but no harm
        ]
        build_args = []

        if self.compiler.compiler_type != "msvc":
            # Using Ninja-build since it a) is available as a wheel and b)
            # multithreads automatically. MSVC would require all variables be
            # exported for Ninja to pick it up, which is a little tricky to do.
            # Users can override the generator with CMAKE_GENERATOR in CMake
            # 3.15+.
            if not cmake_generator:
                cmake_args += ["-GNinja"]

        else:
            # Single config generators are handled "normally"
            single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})

            # CMake allows an arch-in-generator style for backward compatibility
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

            # Convert distutils Windows platform specifiers to CMake -A arguments
            plat_to_cmake = {
                "win32": "Win32",
                "win-amd64": "x64",
                "win-arm32": "ARM",
                "win-arm64": "ARM64",
            }

            # Specify the arch if using MSVC generator, but only if it doesn't
            # contain a backward-compatibility arch spec already in the
            # generator name.
            if not single_config and not contains_arch:
                cmake_args += ["-A", plat_to_cmake[self.plat_name]]

            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [
                    "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}".format(cfg.upper(), extdir)
                ]
                build_args += ["--config", cfg]

        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                # CMake 3.12+ only.
                build_args += ["-j{}".format(self.parallel)]

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp
        )
        subprocess.check_call(
            ["cmake", "--build", ".", "--target", ext.name] + build_args, cwd=self.build_temp
        )


# Find stub files
stubs = [path.replace('quicktex/', '') for path in glob.glob('quicktex/**/*.pyi', recursive=True)]

# The information here can also be placed in setup.cfg - better separation of
# logic and declaration, and simpler if you include description/version in a file.
setup(
    name="quicktex",
    use_scm_version=True,
    author="Andrew Cassidy",
    author_email="drewcassidy@me.com",
    description="A fast block compression library for python",
    url='https://github.com/drewcassidy/quicktex',
    long_description=readme,
    long_description_content_type='text/markdown',
    python_requires=">=3.7",
    ext_modules=[CMakeExtension("_quicktex")],
    cmdclass={"build_ext": CMakeBuild},
    packages=find_packages(where='.', include=['quicktex*']),
    package_dir={'': '.'},
    package_data={'': ['py.typed'] + stubs},
    include_package_data=True,
    install_requires=["Pillow", "click"],
    extras_require={
        "tests": ["nose", "parameterized"],
        "docs": ["sphinx", "myst-parser", "sphinx-rtd-theme"],
        "stubs": ["pybind11-stubgen"],
    },
    entry_points={
        'console_scripts': ['quicktex = quicktex.__main__:main']
    },
    zip_safe=False,
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 3 :: Only',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        "Topic :: Multimedia :: Graphics :: Graphics Conversion",
        'Programming Language :: Python :: Implementation :: CPython',
        'Programming Language :: C++'
    ],
)
