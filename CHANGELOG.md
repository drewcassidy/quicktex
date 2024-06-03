# Changelog

All notable changes to this project will be documented in this file

## 0.2.1 - 2024-06-03

### Fixed

- Fixed broken transparency on palettized PNG files

### Changed

- Changed which wheels are built by the CI. There are no changes to OS or Python version compatibility if you compile from source.
	- Stopped building Python 3.7 wheels
	- Stopped building macOS universal wheels
	- Wheels for macOS now require macOS 12 or later
	- Included macOS ARM wheels 
	- Included Python 3.12 wheels


## 0.2.0 - 2023-06-21

### Changed

- Updated Pybind11 to version 3.10, adding Python 3.11 support
- Updated install instructions in readme to reflect availability on PyPI
- Encode now skips .dds files in its input to prevent needless re-encoding

### Added

- Added the `-n` option for bc3 encoding to perform a BC3nm swizzle


## 0.1.3 - 2022-04-13

### Fixed

- Fixed quicktex not compiling for python 3.10 on Windows

### Changed

- Reworked CI job, adding wheels for ARM macOS, ARM Linux, and x86 musl Linux.
- Added wheels for python 3.10
- Added a more useful error message when importing quicktex on macOS when libomp.dylib isn't installed


## 0.1.2 - 2022-03-27

### Fixed

- Fixed sdist not including pybind


## 0.1.1 - 2021-09-29

### Fixed

- Fixed alpha premultiplication when generating mipmaps


## 0.1.0 - 2021-05-10

### Added

- Began publishing to PyPI

### Changed

- Rewrote CI workflow to include ManyLinux2014 builds
- Reverted project to C++17 for better compiler compatibility