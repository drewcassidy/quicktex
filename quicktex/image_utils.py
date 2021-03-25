"""Various utilities for working with Pillow images"""

from PIL import Image
import typing
import math


def pad(source: Image.Image, block_dimensions=(4, 4)) -> Image.Image:
    """
    Pad an image to be divisible by a specific block size. The input image is repeated into the unused areas so that bilinar filtering works correctly.

    :param source: Input image to add padding to. This will not be modified.
    :param block_dimensions: The size of a single block that the output must be divisible by.
    :return: A new image with the specified padding added.
    """

    assert all([dim > 0 for dim in block_dimensions]), "Invalid block size"

    padded_dimensions = tuple([
        math.ceil(i_dim / b_dim) * b_dim
        for i_dim in source.size
        for b_dim in block_dimensions
    ])

    if padded_dimensions == source.size:
        # no padding is necessary
        return source

    output = Image.new(source.mode, padded_dimensions)
    for x in range(math.ceil(padded_dimensions[0] / source.width)):
        for y in range(math.ceil(padded_dimensions[1] / source.height)):
            output.paste(source, (x * source.width, y * source.height))

    return output


def mip_sizes(dimensions: typing.Tuple[int, int], mip_count: typing.Optional[int] = None) -> typing.List[typing.Tuple[int, int]]:
    """
    Create a chain of mipmap sizes for a given source source size, where each source is half the size of the one before.
    Note that the division by 2 rounds down. So a 63x63 texture has as its next lowest mipmap level 31x31. And so on.

    See the `OpenGL wiki page on mipmaps <https://www.khronos.org/opengl/wiki/Texture#Mip_maps>`_ for more info.

    :param dimensions: Size of the source source in pixels
    :param mip_count: Number of mipmap sizes to generate. By default, generate until the last mip level is 1x1.
        Resulting mip chain will be smaller if a 1x1 mip level is reached before this value.
    :return: A list of 2-tuples representing the dimensions of each mip level, including ``dimensions`` at element 0.
    """
    assert all([dim > 0 for dim in dimensions]), "Invalid source size"
    if not mip_count:
        mip_count = math.ceil(math.log2(max(dimensions)))  # maximum possible number of mips for a given source

    assert mip_count > 0, "mip_count must be greater than 0"

    chain = []

    for mip in range(mip_count):
        chain.append(dimensions)
        dimensions = tuple([max(dim // 2, 1) for dim in dimensions])

        if all([dim == 1 for dim in dimensions]):
            break  # we've reached a 1x1 mip and can get no smaller

    return chain
