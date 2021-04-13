# Quicktex
A python library for using DDS files

Quicktex is a python library and command line tool for encoding and decoding DDS files.
It is based on the [RGBCX encoder](https://github.com/richgel999/bc7enc), which is currently
one of the [highest quality S3TC encoders available](https://aras-p.info/blog/2020/12/08/Texture-Compression-in-2020/).
Quicktex has a python front end, but the encoding and decoding is all done in C++ for speed
comparable to the original library.

## Installation

To install, first clone this repo and cd into it, then run:

```shell
git submodule update --init
pip install .
```
and setuptools will take care of any dependencies for you.

If you are on macOS, it is recommended to first install openMP from homebrew to enable 
multithreading, since it is not included in the default Apple Clang install:

```shell
brew install libomp
```

The package also makes tests, stub generation, and docs available. To install the 
required dependencies for them, install with options like so:

```shell
pip install .[tests,stubs,docs]
```

Quicktex will be available on Pypi once it is out of alpha.

## Usage

```
Usage: quicktex [OPTIONS] COMMAND [ARGS]...

  Encode and Decode various image formats

Options:
  --version  Show the version and exit.
  --help     Show this message and exit.

Commands:
  decode  Decode DDS files to images.
  encode  Encode images to DDS files of the given format.
```

To decode DDS files to images, use the `decode` subdommand, along with a glob or a
list of files to decode. 

To encode images to DDS files, use the `encode` subcommand, plus an additional
subcommand for the format. For example, `quicktex encode bc1 bun.png` will encode
bun.png in the current directory to a bc1/DXT1 dds file next to it.

`encode` and `decode` both accept several common parameters:

- `-f, --flip / -F, --no-flip`:  Vertically flip image before/after converting. 
  [default: True]
- `-r, --remove`: Remove input images after converting.
- `-s, --suffix TEXT`: Suffix to append to output filename(s). 
  Ignored if `output` is a single file.
- `-o, --output`:  Output file or directory. If outputting to a file, input filenames 
  must be only a single item. By default, files are converted in place.

