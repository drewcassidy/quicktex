import click
import io
import os
from PIL import Image


@click.command()
@click.option('-f', '--flip', type=bool, default=True, show_default=True, help="vertically flip image after converting")
@click.option('-r', '--remove', help="remove input images after converting")
@click.option('-o', '--output', help="output file name. Must only specify one input image.")
@click.option('-s', '--suffix', type=str, default='', help="suffix to append to output file(s).")
@click.argument('filenames', nargs=-1, type=click.Path(exists=True))
def decode(flip, remove, output, suffix, filenames):
    """Decode an input texture file to an image"""
    for filename in filenames:
        assert filename.endswith(".dds"), "Incorrect file extension"
