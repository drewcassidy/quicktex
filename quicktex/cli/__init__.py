from PIL import Image
from typing import List
import pathlib
import click


def get_decoded_extensions(feature: str = 'open') -> List[str]:
    extensions = Image.registered_extensions()  # using registered_extensions() triggers lazy loading of format data
    formats = getattr(Image, feature.upper()).keys()

    return [ext for ext, fmt in extensions.items() if fmt in formats]


def validate_decoded_extension(ctx, param, value):
    if value[0] != '.':
        value = '.' + value

    if value not in decoded_extensions:
        raise click.BadParameter(f'Invalid extension for decoded file. Valid extensions are:\n{decoded_extensions}')

    return value


decoded_extensions = get_decoded_extensions()
encoded_extensions = '.dds'
