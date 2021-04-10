import click
import os.path
import quicktex.dds as dds
import quicktex.cli.common as common
from PIL import Image


@click.command()
@click.option('-f/-F', '--flip/--no-flip', default=True, show_default=True, help="Vertically flip image after converting.")
@click.option('-r', '--remove', is_flag=True, help="Remove input images after converting.")
@click.option('-s', '--suffix', type=str, default='', help="Suffix to append to output file(s). Ignored if output is a single file.")
@click.option('-x', '--extension',
              callback=common.validate_decoded_extension,
              type=str, default='.png', show_default=True,
              help="Extension to use for output. Ignored if output is a single file. Output filetype is deduced from this")
@click.option('-o', '--output',
              type=click.Path(writable=True), default=None,
              help="Output file or directory. If outputting to a file, input filenames must be only a single item. By default, files are decoded in place.")
@click.argument('filenames', nargs=-1, type=click.Path(exists=True, readable=True, dir_okay=False))
def decode(flip, remove, suffix, extension, output, filenames):
    """Decode DDS files to images."""

    path_pairs = common.path_pairs(filenames, output, suffix, extension)

    with click.progressbar(path_pairs, show_eta=False, show_pos=True, item_show_func=lambda x: str(x[0]) if x else '') as bar:
        for inpath, outpath in bar:
            if inpath.suffix != '.dds':
                raise click.BadArgumentUsage(f"Input file '{inpath}' is not a DDS file.")

            image = dds.read(inpath).decode()

            if flip:
                image = image.transpose(Image.FLIP_TOP_BOTTOM)

            image.save(outpath)

            if remove:
                os.remove(inpath)


if __name__ == '__main__':
    decode()
