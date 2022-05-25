# Changelog

All notable changes to this project will be documented in this file

## Unreleased

### Fixed

- Fixed LeastSquares endpoint mode producint incorrect results


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