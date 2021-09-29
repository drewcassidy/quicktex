from __future__ import annotations

import enum
import os
import struct
import typing
import quicktex.image_utils
import quicktex.s3tc.bc1 as bc1
import quicktex.s3tc.bc3 as bc3
import quicktex.s3tc.bc4 as bc4
import quicktex.s3tc.bc5 as bc5
from PIL import Image


class DDSFormat:
    def __init__(self, name: str, texture, encoder, decoder, four_cc: str = None):
        self.four_cc = four_cc
        self.decoder = decoder
        self.encoder = encoder
        self.texture = texture
        self.name = name


dds_formats = [
    DDSFormat('BC1', bc1.BC1Texture, bc1.BC1Encoder, bc1.BC1Decoder, 'DXT1'),
    DDSFormat('BC3', bc3.BC3Texture, bc3.BC3Encoder, bc3.BC3Decoder, 'DXT5'),
    DDSFormat('BC4', bc4.BC4Texture, bc4.BC4Encoder, bc4.BC4Decoder, 'ATI1'),
    DDSFormat('BC5', bc5.BC5Texture, bc5.BC5Encoder, bc5.BC5Decoder, 'ATI2'),
]


class PFFlags(enum.IntFlag):
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


class DDSFlags(enum.IntFlag):
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

    TEXTURE = CAPS | HEIGHT | WIDTH | PIXEL_FORMAT


class Caps0(enum.IntFlag):
    """Flags to indicate surface complexity"""

    COMPLEX = 0x8
    """Optional; must be used on any file that contains more than one surface (a mipmap, a cubic environment map, or mipmapped volume texture)."""

    MIPMAP = 0x400000
    """Optional; should be used for a mipmap."""

    TEXTURE = 0x1000
    """Required"""


@typing.final
class DDSFile:
    """
    A microsoft DDS file, containing header information and one or more textures

    For more information, see microsoft's `Reference for DDS <https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-reference>`_.
    """

    magic = b'DDS '
    """Magic bytes at the start of every DDS file."""

    extension = 'dds'
    """Extension for a DDS file."""

    header_bytes = 124
    """The size of a DDS header in bytes."""

    def __init__(self):
        self.flags: DDSFlags = DDSFlags.TEXTURE
        """Flags to indicate which members contain valid data."""

        self.size: typing.Tuple[int, int] = (0, 0)
        """Width and height of the texture or its first mipmap"""

        self.pitch: int = 0
        """The pitch or number of bytes per row in an uncompressed texture; 
        the total number of bytes in the top level texture for a compressed texture."""

        self.depth: int = 1
        """Depth of a volume texture (in pixels), otherwise unused."""

        self.mipmap_count: int = 1
        """Number of mipmap levels, otherwise unused."""

        self.pf_flags: PFFlags = PFFlags.FOURCC
        """Flags representing which pixel format data is valid."""

        self.four_cc: str = "NONE"
        """FourCC code of the texture format. Valid texture format strings are ``DXT1``, ``DXT2``, ``DXT3``, ``DXT4``, or ``DXT5``. 
        If a DirectX 10 header is used, this is ``DX10``."""

        self.pixel_size: int = 0
        """Number of bits in each pixel if the texture is uncompressed"""

        self.pixel_bitmasks: typing.Tuple[int, int, int, int] = (0, 0, 0, 0)
        """Tuple of bitmasks for each channel"""

        self.caps: typing.Tuple[Caps0, int, int, int] = (Caps0.TEXTURE, 0, 0, 0)
        """Specifies the complexity of the surfaces stored."""

        self.textures: typing.List = []
        """A list of bytes objects for each texture in the file"""

        self.format: DDSFormat = DDSFormat('NONE', None, None, None)
        """The format used by this dds file"""

    def save(self, path: os.PathLike) -> None:
        """
        Save the DDSFile to a file
        :param path: string or path-like object to write to
        """
        with open(path, 'wb') as file:
            file.write(DDSFile.magic)

            # WRITE HEADER
            file.write(struct.pack('<7I44x', DDSFile.header_bytes, int(self.flags), self.size[1], self.size[0], self.pitch, self.depth, self.mipmap_count))
            file.write(struct.pack('<2I4s5I', 32, int(self.pf_flags), bytes(self.four_cc, 'ascii'), self.pixel_size, *self.pixel_bitmasks))
            file.write(struct.pack('<4I4x', *self.caps))

            assert file.tell() == 4 + DDSFile.header_bytes, 'error writing file: incorrect header size'

            for texture in self.textures:
                file.write(texture)

    def decode(self, mip: int = 0, *args, **kwargs) -> Image.Image:
        """
        Decode a single texture in the file to images
        :param mip: the mip level to decode. Default: 0
        :return: The decoded image
        """
        decoder = self.format.decoder(*args, **kwargs)
        texture = decoder.decode(self.textures[mip])
        return Image.frombuffer('RGBA', texture.size, texture)

    def decode_all(self, *args, **kwargs) -> typing.List[Image.Image]:
        """
        Decade all textures in the file to images
        :return: the decoded images
        """
        decoder = self.format.decoder(*args, **kwargs)
        textures = [decoder.decode(encoded) for encoded in self.textures]
        return [Image.frombuffer('RGBA', tex.size, tex) for tex in textures]


def read(path: os.PathLike) -> DDSFile:
    with open(path, 'rb') as file:
        assert file.read(4) == DDSFile.magic, "Incorrect magic bytes in DDS file."

        dds = DDSFile()

        # READ HEADER
        header_bytes = struct.unpack('<I', file.read(4))[0]
        assert header_bytes == DDSFile.header_bytes, "Incorrect DDS header size."

        dds.flags = DDSFlags(struct.unpack('<I', file.read(4))[0])  # read flags enum
        dds.size = struct.unpack('<2I', file.read(8))[::-1]  # read dimensions
        dds.pitch, dds.depth, dds.mipmap_count = struct.unpack('<3I', file.read(12))
        file.read(44)  # skip 44 unused bytes of data

        assert struct.unpack('<I', file.read(4))[0] == 32, "Incorrect pixel format size."

        dds.pf_flags = PFFlags(struct.unpack('<I', file.read(4))[0])
        dds.four_cc = file.read(4).decode()
        dds.pixel_size, *pixel_bitmasks = struct.unpack('<5I', file.read(20))

        dds.caps = struct.unpack('<4I', file.read(16))
        file.read(4)  # skip 4 unused bytes of data

        assert file.tell() == 4 + DDSFile.header_bytes, "Unexpected EOF"  # make sure we are where we expect to be

        if DDSFlags.DEPTH not in dds.flags:
            dds.depth = 1
        if DDSFlags.MIPMAPCOUNT not in dds.flags:
            dds.mipmap_count = 1
        if PFFlags.FOURCC not in dds.pf_flags:
            dds.four_cc = 'NONE'

        # READ DX10_HEADER
        if dds.four_cc == 'DX10':
            raise NotImplementedError('DX10 headers are not yet supported')

        # identify the format used
        dds.format = next(entry for entry in dds_formats if entry.four_cc == dds.four_cc)

        # calculate the size of each level of the texture
        sizes = quicktex.image_utils.mip_sizes(dds.size, dds.mipmap_count)

        # READ TEXTURES
        dds.textures = []
        for size in sizes:
            texture = dds.format.texture(*size)  # make a new blocktexture of the current mip size
            nbytes = file.readinto(texture)

            assert nbytes == texture.nbytes, 'Unexpected end of file'

            dds.textures.append(texture)

        return dds


def encode(image: Image.Image, encoder, four_cc: str, mip_count: typing.Optional[int] = None) -> DDSFile:
    if image.mode != 'RGBA' or image.mode != 'RGBX':
        mode = 'RGBA' if 'A' in image.mode else 'RGBX'
        image = image.convert(mode)

    sizes = quicktex.image_utils.mip_sizes(image.size, mip_count)
    images = [image] + [quicktex.image_utils.resize_no_premultiply(image, size) for size in sizes[1:]]
    dds = DDSFile()

    for i in images:
        rawtex = quicktex.RawTexture.frombytes(i.tobytes('raw', mode), *i.size)
        dds.textures.append(encoder.encode(rawtex))

    dds.flags = DDSFlags.TEXTURE | DDSFlags.LINEAR_SIZE
    caps0 = Caps0.TEXTURE

    if len(images) > 1:
        dds.flags |= DDSFlags.MIPMAPCOUNT
        caps0 |= Caps0.MIPMAP | Caps0.COMPLEX

    dds.caps = (caps0, 0, 0, 0)
    dds.mipmap_count = len(images)
    dds.pitch = dds.textures[0].nbytes
    dds.size = dds.textures[0].size
    dds.pf_flags = PFFlags.FOURCC
    dds.four_cc = four_cc

    return dds
