import click
from quicktex.cli.encode import encode
from quicktex.cli.decode import decode


@click.group()
@click.version_option()
def cli():
    """Encode and Decode various image formats"""


cli.add_command(encode)
cli.add_command(decode)

if __name__ == '__main__':
    cli()
