import unittest
import python_rgbcx
import color
import s3tc


class TestBC1Encoder(unittest.TestCase):
    def setUp(self):
        self.bc1_encoder = python_rgbcx.BC1Encoder(python_rgbcx.InterpolatorType.Ideal, 5)
        self.bc1_encoder_no3color = python_rgbcx.BC1Encoder(python_rgbcx.InterpolatorType.Ideal, 5, False, False)
        self.bc1_encoder_noblack = python_rgbcx.BC1Encoder(python_rgbcx.InterpolatorType.Ideal, 5, True, False)

        # A block that should always encode greyscale
        self.greyscale = b'\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\x55\x55\x55\xFF\xAA\xAA\xAA\xFF' \
                         b'\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\x55\x55\x55\xFF\xAA\xAA\xAA\xFF' \
                         b'\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\x55\x55\x55\xFF\xAA\xAA\xAA\xFF' \
                         b'\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\x55\x55\x55\xFF\xAA\xAA\xAA\xFF'

        # A block that should always encode 3-color with black when available
        self.chroma_black = b'\x00\x00\x00\xFF\xFF\x00\x00\xFF\x88\x88\x00\xFF\x00\xFF\x00\xFF' \
                            b'\x00\x00\x00\xFF\xFF\x00\x00\xFF\x88\x88\x00\xFF\x00\xFF\x00\xFF' \
                            b'\x00\x00\x00\xFF\xFF\x00\x00\xFF\x88\x88\x00\xFF\x00\xFF\x00\xFF' \
                            b'\x00\x00\x00\xFF\xFF\x00\x00\xFF\x88\x88\x00\xFF\x00\xFF\x00\xFF'

    def test_block_size(self):
        out = self.bc1_encoder.encode_image(self.greyscale, 4, 4)

        self.assertEqual(self.bc1_encoder.block_width, 4, 'incorrect reported block width')
        self.assertEqual(self.bc1_encoder.block_height, 4, 'incorrect reported block height')
        self.assertEqual(self.bc1_encoder.block_size, 8, 'incorrect reported block size')
        self.assertEqual(len(out), 8, 'incorrect returned block size')

    def test_block_3color(self):
        out = s3tc.BC1Block.from_bytes(self.bc1_encoder.encode_image(self.chroma_black, 4, 4))
        out_no_3color = s3tc.BC1Block.from_bytes(self.bc1_encoder_no3color.encode_image(self.chroma_black, 4, 4))

        self.assertTrue(out.is_3color(), "incorrect color mode with use_3color enabled")
        self.assertFalse(out_no_3color.is_3color(), "incorrect color mode with use_3color disabled")

    def test_block_black(self):
        out = s3tc.BC1Block.from_bytes(self.bc1_encoder.encode_image(self.chroma_black, 4, 4))
        out_no_black = s3tc.BC1Block.from_bytes(self.bc1_encoder_noblack.encode_image(self.chroma_black, 4, 4))

        self.assertTrue(any(3 in row for row in out.selectors), "use_3color_black enabled but not used")
        self.assertFalse(out_no_black.is_3color()
                         and any(3 in row for row in out_no_black.selectors),
                         "use_3color_black disabled but 3 color block has black selectors")


if __name__ == '__main__':
    unittest.main()
