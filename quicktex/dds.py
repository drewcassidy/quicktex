from __future__ import annotations

import enum
import struct
import typing


class PixelFormat:
    """
    DDS header surface format.

    For more information, see microsoft documentation for `DDS_PIXELFORMAT <https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dds-pixelformat>`_.
    """
    size = 32
    """The size of a PixelFormat block in bytes."""

    class Flags(enum.IntFlag):
        """Values which indicate what type of data is in the surface."""

        ALPHAPIXELS = 0x1
        """Texture contains alpha data (:py:attr:`~PixelFormat.pixel_bitmasks[3]` contains valid data)."""

        ALPHA = 0x2
        """Used in some older DDS files for alpha channel only uncompressed data 
        (:py:attr:`~PixelFormat.pixel_size` contains the alpha channel bitcount; :py:attr:`~PixelFormat.pixel_bitmasks[3]` contains valid data)."""

        FOURCC = 0x4
        """Texture contains compressed RGB data; :py:attr:`~PixelFormat.four_cc` contains valid data."""

        RGB = 0x40
        """Texture contains uncompressed RGB data; :py:attr:`~PixelFormat.pixel_size` and the RGB masks 
        (:py:attr:`~PixelFormat.pixel_bitmasks[0:3]`) contain valid data."""

        YUV = 0x200
        """Used in some older DDS files for YUV uncompressed data 
        (:py:attr:`~PixelFormat.pixel_size` contains the YUV bit count; :py:attr:`~PixelFormat.pixel_bitmasks[0]` contains the Y mask, 
        :py:attr:`~PixelFormat.pixel_bitmasks[1]` contains the U mask, :py:attr:`~PixelFormat.pixel_bitmasks[2]` contains the V mask)."""

        LUMINANCE = 0x20000
        """Used in some older DDS files for single channel color uncompressed data (:py:attr:`~PixelFormat.pixel_size` 
        contains the luminance channel bit count; :py:attr:`~PixelFormat.pixel_bitmasks[0]` contains the channel mask). 
        Can be combined with :py:attr:`ALPHAPIXELS` for a two channel uncompressed DDS file."""

    def __init__(self):
        self.flags: PixelFormat.Flags = PixelFormat.Flags.FOURCC
        """Flags representing which data is valid."""

        self.four_cc: str = "NONE"
        """FourCC code of the texture format. Valid texture format strings are ``DXT1``, ``DXT2``, ``DXT3``, ``DXT4``, or ``DXT5``. 
        If a DirectX 10 header is used, this is ``DX10``."""

        self.pixel_size: int = 0
        """Number of bits in each pixel if the texture is uncompressed"""

        self.pixel_bitmasks: typing.Tuple[int, int, int, int] = (0, 0, 0, 0)
        """Tuple of bitmasks for each channel"""

    @staticmethod
    def from_bytes(data) -> PixelFormat:
        """
        Create a new PixelFormat object from a bytes-like object

        :param data: A bytes-like object holding the raw data to unpack
        :return: An unpacked PixelFormat object
        """
        assert len(data) == PixelFormat.size, "Incorrect number of bytes in input."

        unpacked = struct.unpack('<2I4s5I', data)
        assert unpacked[0] == PixelFormat.size, "Incorrect pixelformat size."

        pf = PixelFormat()
        pf.flags = PixelFormat.Flags(unpacked[1])
        pf.four_cc = unpacked[2].decode()
        pf.pixel_size = unpacked[3]
        pf.pixel_bitmasks = unpacked[4:8]
        return pf

    @staticmethod
    def from_file(file: typing.BinaryIO) -> PixelFormat:
        """
        Create a new PixelFormat object from a file. The file position will be advanced by 32 bytes.

        :param file: A file-like object to read from.
        :return: An unpacked PixelFormat object
        """
        assert file.readable(), "Input file is not readable."
        data = file.read(PixelFormat.size)
        return PixelFormat.from_bytes(data)

    def to_bytes(self) -> bytes:
        """
        Write the PixelFormat object to a bytes object.

        :return: The packed PixelFormat object
        """
        data = struct.pack('<2I4s5I', 32, int(self.flags), bytes(self.four_cc, 'ascii'), self.pixel_size, *self.pixel_bitmasks)
        assert len(data) == PixelFormat.size
        return data


class DDSHeader:
    """
    Header for a microsoft DDS file

    For more information, see microsoft documentation for `DDS_HEADER <https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dds-header>`_.
    """
    size = 124
    """The size of a DDS header in bytes."""

    class Flags(enum.IntFlag):
        """Flags to indicate which members contain valid data."""

        CAPS = 0x1
        """Required in every .dds file."""

        HEIGHT = 0x2
        """Required in every .dds file."""

        WIDTH = 0x4
        """Required in every .dds file."""

        PITCH = 0x8
        """Required when :py:attr:`~DDSHeader.pitch` is provided for an uncompressed texture."""

        PIXEL_FORMAT = 0x1000
        """Required in every .dds file."""

        MIPMAPCOUNT = 0x20000
        """Required when :py:attr:`~DDSHeader.mipmap_count` is provided for a mipmapped texture."""

        LINEAR_SIZE = 0x80000
        """Required when :py:attr:`~DDSHeader.pitch` is provided for a compressed texture."""

        DEPTH = 0x800000
        """Required when :py:attr:`~DDSHeader.depth` is provided for a depth texture."""

        REQUIRED = CAPS | HEIGHT | WIDTH | PIXEL_FORMAT

    def __init__(self):
        self.flags: DDSHeader.Flags = DDSHeader.Flags.REQUIRED
        """Flags to indicate which members contain valid data."""

        self.dimensions: typing.Tuple[int, int] = (0, 0)
        """Width and height of the texture or its first mipmap"""

        self.pitch: int = 0
        """The pitch or number of bytes per scan line in an uncompressed texture; 
        the total number of bytes in the top level texture for a compressed texture."""

        self.depth: int = 0
        """Depth of a volume texture (in pixels), otherwise unused."""

        self.mipmap_count: int = 0
        """Number of mipmap levels, otherwise unused."""

        self.pixel_format: PixelFormat = PixelFormat()
        """The pixel format"""

        self.caps: typing.Tuple[int, int, int, int] = (0, 0, 0, 0)
        """Specifies the complexity of the surfaces stored."""

    @staticmethod
    def from_bytes(data) -> DDSHeader:
        """
        Create a new DDS Header from a bytes-like object

        :param data: A bytes-like object holding the raw data to unpack
        :return: An unpacked DDS header
        """
        assert len(data) == PixelFormat.size, "Incorrect number of bytes in input."

        unpacked = struct.unpack('<7I44x', data[0:72])
        assert unpacked[0] == DDSHeader.size, "Incorrect DDS header size."

        header = DDSHeader()
        header.flags = DDSHeader.Flags(unpacked[1])
        header.dimensions = unpacked[2:4:-1]
        header.pitch, header.depth, header.mipmapcount = unpacked[4:7]

        header.pixel_format = PixelFormat.from_bytes(data[72:104])

        header.caps = struct.unpack('<4I4x', data[104:124])

        assert all([val > 0 for val in header.dimensions]), "Image size is zero"

        return header

    @staticmethod
    def from_file(file: typing.BinaryIO) -> DDSHeader:
        """
        Create a new DDS header from a file. the file position will be advanced by 124 bytes.

        :param file: A file-like object to read from.
        :return: An unpacked DDS header
        """
        assert file.readable(), "Input file is not readable."
        return DDSHeader.from_bytes(file.read(DDSHeader.size))

    def to_bytes(self) -> bytes:
        """
        Write the DDS header to a bytes object.

        :return: The packed DDS header
        """
        data = b''
        # write header
        data += struct.pack('<7I44x', DDSHeader.size, int(self.flags), self.dimensions[1], self.dimensions[0],
                            self.pitch, self.depth, self.mipmap_count)
        data += self.pixel_format.to_bytes()
        data += struct.pack('<4I4x', *self.caps)

        assert len(data) == DDSHeader.size
        return data


class DDSFile:
    """
    A microsoft DDS file, containing header information and one or more textures

    For more information, see microsoft's `Reference for DDS <https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-reference>`_.
    """

    magic = b'DDS '
    """Magic bytes at the start of every DDS file."""

    extension = 'dds'
    """Extension for a DDS file."""

    def __init__(self):
        self.header: DDSHeader = DDSHeader()
        """The DDS file's header object"""

        self.textures = []

    @staticmethod
    def from_file(file: typing.BinaryIO) -> DDSFile:
        """
        Create a new DDSFile object from the contents of a file.

        :param file: A file-like object to read from.
        :return: An unpacked DDS file
        """
        assert file.readable(), "Input file is not readable."
        assert file.read(4) == DDSFile.magic, "Incorrect magic bytes in DDS file."

        dds = DDSFile()
        dds.header = DDSHeader.from_file(file)

        # TODO: read file contents

        return dds

    def write(self, file: typing.BinaryIO):
        """
        Write a DDSFile object to a file.

        :param file: a file-like object to write to.
        """
        assert file.writable(), "Output file is not writable"

        file.write(DDSFile.magic)
        file.write(self.header.to_bytes())

        # TODO: write file contents
