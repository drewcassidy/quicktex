import click
import io
import os.path
import quicktex.dds as dds
from PIL import Image
from pathlib import Path


@click.command()
@click.option('-f', '--flip', help="vertically flip image after converting")
@click.option('-r', '--remove', help="remove input images after converting")
@click.option('-s', '--suffix', type=str, default='', help="suffix to append to output file(s).")
@click.option('-x', '--extension',
              type=str, default='png',
              help="extension to use for output. ignored if output is a single file. output filetype is deduced from this")
@click.option('-o', '--output',
              type=click.Path(exists=True, readable=True), default='.',
              help="output file name or directory. If outputting to a file, input filenames must be only a single item.")
@click.argument('filenames',
                nargs=-1,
                type=click.Path(exists=True, readable=True))
def decode(flip, remove, suffix, extension, output, filenames):
    """Decode an input texture file to an image"""

    assert len(filenames) != 0, 'No input files provided.'
    outpath = Path(output)

    if outpath.is_file():
        assert len(filenames) == 1, 'Provided an output file with multiple inputs.'

        def make_outpath(p):
            return outpath
    else:
        def make_outpath(p):
            return outpath / (p.stem + suffix + '.' + extension)

    for filename in filenames:
        filepath = Path(filename)
        outpath = make_outpath(filepath)

        assert filepath.is_file(), f"{filename} is not a file!"
        assert filepath.suffix == '.dds', f"{filename} is not a DDS file!"

        ddsfile = dds.read(filepath)
        image = ddsfile.decode()
        image.save(outpath)
