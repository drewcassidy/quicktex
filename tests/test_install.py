"""Test if everything is installed correctly"""

import unittest
import os.path
import quicktex
import nose

tests_path = os.path.dirname(os.path.realpath(__file__))


def test_images():
    """Test for the images submodule"""

    images_path = os.path.join(tests_path, 'images')

    assert os.path.isdir(images_path), 'test images repo not present. run "git clone https://git.pileof.rocks/drewcassidy/quicktex-test-images.git tests/images" to download them'
    assert os.path.isfile(os.path.join(images_path, '__init__.py')), 'images __init__.py not present, is the test image repo present?'
    bp_size = os.path.getsize(os.path.join(images_path, 'Boilerplate.png'))
    assert bp_size == 955989, 'Boilerplate.png is the wrong size, is the test image repo checked out with LFS enabled?'


def test_version():
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
