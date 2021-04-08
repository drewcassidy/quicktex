import unittest
import os.path

tests_path = os.path.dirname(os.path.realpath(__file__))


class TestInstall(unittest.TestCase):
    """Test if everything is installed correctly"""

    def test_images(self):
        """Test for the images submodule"""

        images_path = os.path.join(tests_path, 'images')

        assert os.path.isdir(images_path), 'images directory/submodule not present'
        assert os.path.isfile(os.path.join(images_path, '__init__.py')), 'images __init__.py not present, is the submodule checked out?'
        bp_size = os.path.getsize(os.path.join(images_path, 'Boilerplate.png'))
        assert bp_size == 955989, 'Boilerplate.png is the wrong size, is the submodule checked out with LFS enabled?'
