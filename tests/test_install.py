"""Test if everything is installed correctly"""

import quicktex


class TestInstall:
    def test_version(self):
        """Test if the extension module version matches what setuptools returns"""
        try:
            from importlib import metadata
        except ImportError:
            # Python < 3.8, so we cant get the metadata, so just check if it exists
            assert quicktex.__version__
            print(f'Cannot check version in python < 3.8. __version__ is {quicktex.__version__}')
            return

        version = metadata.version('quicktex')

        assert version == quicktex.__version__, 'incorrect version string from extension module'
