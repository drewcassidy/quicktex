# Getting Started

## Installation

Install and update using [pip](https://pip.pypa.io/en/stable/quickstart/):

```shell
pip install -U quicktex
```

If you are on macOS, you need to install openMP to allow multithreading, since it does not ship with the built-in Clang.
This can be done easily
using [homebrew](https://brew.sh). This is not required if building from source, but highly recommended.

```shell
brew install libomp
```

If you want, you can also install from source. First clone the [git repo](https://github.com/drewcassidy/quicktex) and
install it with:

```shell
pip install .
```

and setuptools will take care of any dependencies for you.

The package also makes tests, stub generation, and docs available. To install the
required dependencies for them, install with options like so:

```shell
pip install quicktex[tests,stubs,docs]
```

## Usage

For detailed documentation on the {command}`quicktex` command and its subcommands see the {doc}`commands`.

### Examples

#### Encoding a file

To encode a file in place, use the {command}`encode` command

```shell
quicktex encode auto bun.png # chooses format based on alpha
quicktex encode bc3 bun.png # encodes as bc3
```

the auto subcommand automatically chooses between bc1 and bc3 for your image depending on the contents of its alpha
channel. Quicktex supports reading from all image formats supported by [pillow](https://pillow.readthedocs.io/en/stable/handbook/image-file-formats.html).

By default, Quicktex converts in place, meaning the above command will produce a `bun.dds` file alongside the png. If
you want to replace the png, use the `-r` flag to remove it after converting.

if you want to specify an output filename or directory use the `-o` flag.

```shell
quicktex encode auto -o rabbit.dds bun.png # produces rabbit.dds
quicktex.encode auto -o textures/ bun.png # produces textures/bun.dds, if textures/ exists
```

#### Encoding multiple files

quicktex is also able to convert multiple files at once, for example, to encode every png file in the images folder,
use:

```shell
quicktex encode auto images/*.png # encodes in-place
quicktex encode auto -o textures/ images/*.png # encodes to the textures/ directory
```

please note that globbing is an operation performed by your shell and is not supported by the built in windows `cmd.exe`
. If you are on Windows, please use Powershell or any posix-ish shell like [fish](https://fishshell.com).

#### Decoding files

decoding is performed exactly the same as encoding, except without having to specify a format. The output image format
is set using the `-x` flag, and defaults to png. Quicktex supports writing to all image formats supported by [pillow](https://pillow.readthedocs.io/en/stable/handbook/image-file-formats.html)

```shell
quicktex decode bun.dds # produces bun.png
quicktex decode -x .tga bun2.dds # produces bun.tga
```
