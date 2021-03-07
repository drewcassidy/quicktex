import unittest
import python_rgbcx


class MyTestCase(unittest.TestCase):
    def setUp(self):
        self.bc1_encoder = python_rgbcx.BC1Encoder()
        self.image = b'\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\x55\x55\x55\xFF\xAA\xAA\xAA\xFF' \
                     b'\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\x55\x55\x55\xFF\xAA\xAA\xAA\xFF' \
                     b'\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\x55\x55\x55\xFF\xAA\xAA\xAA\xFF' \
                     b'\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\x55\x55\x55\xFF\xAA\xAA\xAA\xFF'

    def test_block_size(self):
        out = self.bc1_encoder.encode_image(self.image, 4, 4)
        self.assertEqual(self.bc1_encoder.block_size, 8, 'incorrect reported block size')
        self.assertEqual(len(out), 8, 'incorrect returned block size')


if __name__ == '__main__':
    unittest.main()
