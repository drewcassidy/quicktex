# Development

This document outlines how to set up a development environment for Quicktex. Documentation on writing Python extension modules is sparse, so I hope this document is useful for other projects as well. The [Coding Patterns for Python Extensions](https://pythonextensionpatterns.readthedocs.io/en/latest/) site has some useful information and will be linked to often in this document. 

## Setting up Debug Python

Many development tools require debug symbols to function, and since the front-end for accessing an extension module is Python, that usually means adding debug symbols to Python. [This Page](https://pythonextensionpatterns.readthedocs.io/en/latest/debugging/debug_python.html) has some instructions on building python with debug symbols. 

If you plan to use DTrace, enable the `--with-dtrace` flag when running `configure`. 

It's useful for this debug python to have SSL enabled so that packages can be installed using pip. Enable SSL with the `--with-openssl` flag when running `configure`. If you are on macOS and installed OpenSSL through Homebrew, you may need to use `--with-openssl=$(brew --prefix openssl)` to help the compiler find it. 

### Installing Debug Python

You can keep the resulting binary in your local copy of the cpython repo and symlink to it, but I like to install it somewhere like `/opt/python-debug/`. The install location is set in the `configure` tool using the `--prefix` flag, and installation is done by running `make install`

### Mixing Debug and Release Python

The debug build of python is slow (It may be possible to build with debug symbols but full optimization, I have not looked into it). If you already have a venv setup for your project, you can just symlink the debug python binary into `env/bin` with a full name like `python3.9d`. Make sure that the debug build has the same minor version (e.g '3.9') as the version you made the virtual environment with to maintain ABI compatibility. 

## Profiling with Dtrace

DTrace is the default program profiler on macOS and other Unix systems, but it's also available for use on Windows and Linux. Using DTrace requires building Python with DTrace hooks as seen above. 

Your extension module does not need a full debug build to profile, but it does need frame pointers to see the stack trace at each sample, as well as debug symbols to give functions names. The cmake build type `RelWithDebInfo` handles this automatically. 