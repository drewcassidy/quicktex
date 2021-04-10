import click
from quicktex.cli.encode import encode
from quicktex.cli.decode import decode


@click.group()
@click.version_option()
def main():
    """Encode and Decode various image formats"""


main.add_command(encode)
main.add_command(decode)

if __name__ == '__main__':
    main()
