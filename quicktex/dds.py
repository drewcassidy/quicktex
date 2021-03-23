from __future__ import annotations

import enum
import struct
import typing


class DDSFlags(enum.IntFlag):
    CAPS = 0x1
    HEIGHT = 0x2
    WIDTH = 0x4
    PITCH = 0x8
    PIXEL_FORMAT = 0x1000
    MIPMAPCOUNT = 0x20000
    LINEAR_SIZE = 0x80000
    DEPTH = 0x800000


class DDSPixelFlags(enum.IntFlag):
    ALPHAPIXELS = 0x1
    ALPHA = 0x2
    FOURCC = 0x4
    RGB = 0x40
    YUV = 0x200
    LUMINANCE = 0x20000


class PixelFormat:
    """
    DDS header surface format.

    For more information, see microsoft documentation for `DDS_PIXELFORMAT <https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dds-pixelformat>`_.
    """
    size = 32
    """The size of a PixelFormat block in bytes."""

    def __init__(self):
        self.flags: DDSPixelFlags = DDSPixelFlags(0)
        self.four_cc: str = "NONE"
        """ FourCC code of the texture format. Valid texture format strings are ``DXT1``, ``DXT2``, ``DXT3``, ``DXT4``, or ``DXT5``. 
        If a DirectX 10 header is used, this is ``DX10``."""
        self.rgb_bit_count: int = 0
        self.color_bitmask: typing.Tuple[int, int, int, int] = (0, 0, 0, 0)

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
        pf.four_cc = unpacked[2].decode()
        pf.rgb_bit_count = unpacked[3]
        pf.color_bitmask = unpacked[4:8]
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
        data = struct.pack('<2I4s5I', 32, int(self.flags), bytes(self.four_cc, 'ascii'), self.rgb_bit_count, *self.color_bitmask)
        assert len(data) == PixelFormat.size
        return data


class DDSHeader:
    """
    Header for a microsoft DDS file

    For more information, see microsoft documentation for `DDS_HEADER <https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dds-header>`_.
    """
    size = 124
    """The size of a DDS header in bytes."""

    def __init__(self):
        self.flags: DDSFlags = DDSFlags(DDSFlags.CAPS | DDSFlags.HEIGHT | DDSFlags.WIDTH | DDSFlags.PIXEL_FORMAT)
        self.image_size: typing.Tuple[int, int] = (0, 0)
        self.linear_size: int = 0
        self.depth = 0
        self.mipmap_count = 0
        self.pixel_format = PixelFormat()
        self.caps = (0, 0, 0, 0)

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
        header.flags = DDSFlags(unpacked[1])
        header.image_size = unpacked[2:4:-1]
        header.linear_size, header.depth, header.mipmapcount = unpacked[4:7]

        header.pixel_format = PixelFormat.from_bytes(data[72:104])

        header.caps = struct.unpack('<4I4x', data[104:124])
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
        data += struct.pack('<7I44x', DDSHeader.size, int(self.flags), self.image_size[1], self.image_size[0],
                            self.linear_size, self.depth, self.mipmap_count)
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
