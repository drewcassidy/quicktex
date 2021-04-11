from PIL import Image
from typing import List
import pathlib
import click


def get_decoded_extensions(feature: str = 'open') -> List[str]:
    """Gets a list of extensions for Pillow formats supporting a supplied feature"""
    extensions = Image.registered_extensions()  # using registered_extensions() triggers lazy loading of format data
    formats = getattr(Image, feature.upper()).keys()

    return [ext for ext, fmt in extensions.items() if fmt in formats]


# noinspection PyUnusedLocal
def validate_decoded_extension(ctx, param, value) -> str:
    """Check if an extension for a decoded image is valid"""
    if value[0] != '.':
        value = '.' + value

    if value not in decoded_extensions:
        raise click.BadParameter(f'Invalid extension for decoded file. Valid extensions are:\n{decoded_extensions}')

    return value


decoded_extensions = get_decoded_extensions()
encoded_extensions = '.dds'


def path_pairs(inputs, output, suffix, extension):
    """
    Generates pairs of (inpath, outpath) for the given parameters
    :param inputs: A list of input paths to read from
    :param output: output to write to. can be a file, directory, or None to convert in place
    :param suffix: suffix to add to destinations
    :param extension: extension to use for destinations
    :return: A list of pairs of (inpath, outpath)
    """

    if len(inputs) < 1:
        raise click.BadArgumentUsage('No input files were provided.')

    inpaths = [pathlib.Path(i) for i in inputs]

    if not output:
        # decode in place
        return [(inpath, inpath.with_name(inpath.stem + suffix + extension)) for inpath in inpaths]

    else:
        outpath = pathlib.Path(output)
        if outpath.is_file():
            # decode to a file
            if len(inputs) > 1:
                raise click.BadOptionUsage('output', 'Output is a single file, but multiple input files were provided.')
            # if outpath.suffix not in decoded_extensions:
            #     raise click.BadOptionUsage('output', f'File has incorrect extension for decoded file. Valid extensions are:\n{decoded_extensions}')

            return [(inpath, outpath) for inpath in inpaths]
        else:
            # decode to directory
            return [(inpath, outpath / (inpath.stem + suffix + extension)) for inpath in inpaths]
