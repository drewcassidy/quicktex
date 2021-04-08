import click
from encode import encode
from decode import decode


@click.group()
def cli():
    """Encode and Decode various image formats"""


cli.add_command(encode)
cli.add_command(decode)

if __name__ == '__main__':
    cli()
