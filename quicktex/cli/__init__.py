from PIL import Image
from typing import List
import click


def get_decoded_extensions(feature: str = 'open') -> List[str]:
    """Gets a list of extensions for Pillow formats supporting a supplied feature"""
    extensions = Image.registered_extensions()  # using registered_extensions() triggers lazy loading of format data
    formats = getattr(Image, feature.upper()).keys()

    return [ext for ext, fmt in extensions.items() if fmt in formats]


# noinspection PyUnusedLocal
def validate_decoded_extension(ctx, param, value):
    """Check if an extension for a decoded image is valid"""
    if value[0] != '.':
        value = '.' + value

    if value not in decoded_extensions:
        raise click.BadParameter(f'Invalid extension for decoded file. Valid extensions are:\n{decoded_extensions}')

    return value


decoded_extensions = get_decoded_extensions()
encoded_extensions = '.dds'
