import click
import io
import os.path
import quicktex.dds as dds
from PIL import Image
from pathlib import Path


@click.command()
@click.option('-f/-F', '--flip/--no-flip', default=True, show_default=True, help="Vertically flip image after converting.")
@click.option('-r', '--remove', is_flag=True, help="Remove input images after converting.")
@click.option('-s', '--suffix', type=str, default='', help="Suffix to append to output file(s). Ignored if output is a single file.")
@click.option('-x', '--extension',
              type=str, default='png', show_default=True,
              help="Extension to use for output. Ignored if output is a single file. Output filetype is deduced from this")
@click.option('-o', '--output',
              type=click.Path(exists=True, readable=True), default=None,
              help="Output file name or directory. If outputting to a file, input filenames must be only a single item. By default, files are decoded in place.")
@click.argument('filenames',
                nargs=-1,
                type=click.Path(exists=True, readable=True))
def decode(flip, remove, suffix, extension, output, filenames):
    """Decode an input texture file to an image"""

    assert len(filenames) != 0, 'No input files provided.'

    if output:
        outpath = Path(output)
        if outpath.is_file():
            # decode to a file
            assert len(filenames) == 1, 'Provided an output file with multiple inputs.'

            def make_outpath(p):
                return outpath
        else:
            # decode to directory
            def make_outpath(p):
                return outpath / (p.stem + suffix + '.' + extension)
    else:
        # decode in place
        def make_outpath(p):
            return p.with_name(p.stem + suffix + '.' + extension)

    for filename in filenames:
        filepath = Path(filename)
        outpath = make_outpath(filepath)

        assert filepath.is_file(), f"{filename} is not a file!"
        assert filepath.suffix == '.dds', f"{filename} is not a DDS file!"

        ddsfile = dds.read(filepath)
        image = ddsfile.decode()

        if flip:
            image = image.transpose(Image.FLIP_TOP_BOTTOM)

        image.save(outpath)

        if remove:
            os.remove(filepath)
