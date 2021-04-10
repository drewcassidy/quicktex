import click
import os.path
import quicktex.dds as dds
import quicktex.image_utils
import quicktex.cli
from PIL import Image
from pathlib import Path


@click.command()
@click.option('-f/-F', '--flip/--no-flip', default=True, show_default=True, help="Vertically flip image after converting.")
@click.option('-r', '--remove', is_flag=True, help="Remove input images after converting.")
@click.option('-s', '--suffix', type=str, default='', help="Suffix to append to output file(s). Ignored if output is a single file.")
@click.option('-x', '--extension',
              callback=quicktex.cli.validate_decoded_extension,
              type=str, default='.png', show_default=True,
              help="Extension to use for output. Ignored if output is a single file. Output filetype is deduced from this")
@click.option('-o', '--output',
              type=click.Path(writable=True), default=None,
              help="Output file or directory. If outputting to a file, input filenames must be only a single item. By default, files are decoded in place.")
@click.argument('filenames', nargs=-1, type=click.Path(exists=True, readable=True, dir_okay=False))
def decode(flip, remove, suffix, extension, output, filenames):
    """Decode DDS files to images."""

    if len(filenames) < 1:
        raise click.BadArgumentUsage('No input files were provided.')

    # decode in place
    def make_outpath(p):
        return p.with_name(p.stem + suffix + extension)

    if output:
        outpath = Path(output)
        if outpath.is_file():
            # decode to a file
            if len(filenames) > 1:
                raise click.BadOptionUsage('output', 'Output is a single file, but multiple input files were provided.')
            if outpath.suffix not in quicktex.cli.decoded_extensions:
                raise click.BadOptionUsage('output', f'File has incorrect extension for decoded file. Valid extensions are:\n{quicktex.cli.decoded_extensions}')

            # noinspection PyUnusedLocal
            def make_outpath(p):
                return outpath
        else:
            # decode to directory
            def make_outpath(p):
                return outpath / (p.stem + suffix + extension)

    with click.progressbar(filenames, show_eta=False, show_pos=True, item_show_func=lambda x: str(x) if x else '') as bar:
        for filename in bar:
            filepath = Path(filename)
            if filepath.suffix != '.dds':
                raise click.BadArgumentUsage(f"Input file '{filename}' is not a DDS file")

            image = dds.read(filepath).decode()

            if flip:
                image = image.transpose(Image.FLIP_TOP_BOTTOM)

            image.save(make_outpath(filepath))

            if remove:
                os.remove(filepath)


if __name__ == '__main__':
    decode()
