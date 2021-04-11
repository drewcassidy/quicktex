import click
import os
import pathlib
import quicktex.s3tc.bc1
import quicktex.s3tc.bc3
import quicktex.s3tc.bc4
import quicktex.s3tc.bc5
import quicktex.dds as dds
import quicktex.cli.common as common
from PIL import Image


@click.group()
def encode():
    """Encode images to DDS files of the given format."""


@click.command()
@click.option('-f/-F', '--flip/--no-flip', default=True, show_default=True, help="Vertically flip image before converting.")
@click.option('-r', '--remove', is_flag=True, help="Remove input images after converting.")
@click.option('-s', '--suffix', type=str, default='', help="Suffix to append to output file(s). Ignored if output is a single file.")
@click.option('-o', '--output',
              type=click.Path(writable=True), default=None,
              help="Output file or directory. If outputting to a file, input filenames must be only a single item. By default, files are decoded in place.")
@click.argument('filenames', nargs=-1, type=click.Path(exists=True, readable=True, dir_okay=False))
def encode_format(encoder, four_cc, flip, remove, suffix, output, filenames):
    path_pairs = common.path_pairs(filenames, output, suffix, '.dds')

    with click.progressbar(path_pairs, show_eta=False, show_pos=True, item_show_func=lambda x: str(x[0]) if x else '') as bar:
        for inpath, outpath in bar:
            image = Image.open(inpath)

            if flip:
                image = image.transpose(Image.FLIP_TOP_BOTTOM)

            dds.encode(image, encoder, four_cc).save(outpath)

            if remove:
                os.remove(inpath)


@click.command('auto')
@click.option('-l', '--level', type=click.IntRange(0, 18), default=18, help='Quality level to use. Higher values = higher quality, but slower.')
@click.option('-b/-B', '--black/--no-black',
              help='[BC1 only] Enable 3-color mode for blocks containing black or very dark pixels. --3color must also be enabled for this to work.'
                   ' (Important: engine/shader MUST ignore decoded texture alpha if this flag is enabled!)')
@click.option('-3/-4', '--3color/--4color', 'threecolor', default=True, help='[BC1 only] Enable 3-color mode for non-black pixels. Higher quality, but slightly slower.')
@click.option('-f/-F', '--flip/--no-flip', default=True, show_default=True, help="Vertically flip image before converting.")
@click.option('-r', '--remove', is_flag=True, help="Remove input images after converting.")
@click.option('-s', '--suffix', type=str, default='', help="Suffix to append to output file(s). Ignored if output is a single file.")
@click.option('-o', '--output',
              type=click.Path(writable=True), default=None,
              help="Output file or directory. If outputting to a file, input filenames must be only a single item. By default, files are decoded in place.")
@click.argument('filenames', nargs=-1, type=click.Path(exists=True, readable=True, dir_okay=False))
def encode_auto(level, black, threecolor, flip, remove, suffix, output, filenames):
    """Encode images to BC1 or BC3, with the format chosen based on each image's alpha channel."""

    color_mode = quicktex.s3tc.bc1.BC1Encoder.ColorMode
    if not threecolor:
        mode = color_mode.FourColor
    elif not black:
        mode = color_mode.ThreeColor
    else:
        mode = color_mode.ThreeColorBlack

    bc1_encoder = quicktex.s3tc.bc1.BC1Encoder(level, mode)
    bc3_encoder = quicktex.s3tc.bc3.BC3Encoder(level)
    path_pairs = common.path_pairs(filenames, output, suffix, '.dds')

    with click.progressbar(path_pairs, show_eta=False, show_pos=True, item_show_func=lambda x: str(x[0]) if x else '') as bar:
        for inpath, outpath in bar:
            image = Image.open(inpath)

            if flip:
                image = image.transpose(Image.FLIP_TOP_BOTTOM)

            if 'A' not in image.mode:
                has_alpha = False
            else:
                alpha_hist = image.getchannel('A').histogram()
                has_alpha = any([a > 0 for a in alpha_hist[:-1]])

            if has_alpha:
                dds.encode(image, bc3_encoder, 'DXT5').save(outpath)
            else:
                dds.encode(image, bc1_encoder, 'DXT1').save(outpath)

            if remove:
                os.remove(inpath)


@click.command('bc1')
@click.option('-l', '--level', type=click.IntRange(0, 18), default=18, help='Quality level to use. Higher values = higher quality, but slower.')
@click.option('-b/-B', '--black/--no-black',
              help='Enable 3-color mode for blocks containing black or very dark pixels. --3color must also be enabled for this to work.'
                   ' (Important: engine/shader MUST ignore decoded texture alpha if this flag is enabled!)')
@click.option('-3/-4', '--3color/--4color', 'threecolor', default=True, help='Enable 3-color mode for non-black pixels. Higher quality, but slightly slower.')
def encode_bc1(level, black, threecolor, **kwargs):
    """Encode images to BC1 (RGB, no alpha)."""
    color_mode = quicktex.s3tc.bc1.BC1Encoder.ColorMode
    if not threecolor:
        mode = color_mode.FourColor
    elif not black:
        mode = color_mode.ThreeColor
    else:
        mode = color_mode.ThreeColorBlack

    encode_format.callback(encoder=quicktex.s3tc.bc1.BC1Encoder(level, mode), four_cc='DXT1', **kwargs)


@click.command('bc3')
@click.option('-l', '--level', type=click.IntRange(0, 18), default=18, help='Quality level to use. Higher values = higher quality, but slower.')
def encode_bc3(level, **kwargs):
    """Encode images to BC4 (RGBA, 8-bit interpolated alpha)."""
    encode_format.callback(quicktex.s3tc.bc3.BC3Encoder(level), 'DXT5', **kwargs)


@click.command('bc4')
def encode_bc4(**kwargs):
    """Encode images to BC4 (Single channel, 8-bit interpolated red channel)."""
    encode_format.callback(quicktex.s3tc.bc4.BC4Encoder(), 'ATI1', **kwargs)


@click.command('bc5')
def encode_bc5(**kwargs):
    """Encode images to BC5 (2-channel, 8-bit interpolated red and green channels)."""
    encode_format.callback(quicktex.s3tc.bc5.BC5Encoder(), 'ATI2', **kwargs)


encode_bc1.params += encode_format.params
encode_bc3.params += encode_format.params
encode_bc4.params += encode_format.params
encode_bc5.params += encode_format.params

encode.add_command(encode_bc1)
encode.add_command(encode_bc3)
encode.add_command(encode_bc4)
encode.add_command(encode_bc5)
encode.add_command(encode_auto)
