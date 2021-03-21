s3tc module
===========

The s3tc module provides encoders and decoders for the common S3 Texture Compression formats. BC1-5 encoding is done
internally using a modified version of rgbcx.h.

bc1 module
----------
.. automodule:: quicktex.s3tc.bc1

    .. autoclass:: BC1Encoder(BlockEncoder)

        .. automethod:: __init__
        .. automethod:: set_level

        .. autoproperty:: color_mode(self) -> ColorMode
        .. autoproperty:: interpolator(self) -> quicktex.s3tc.interpolator.Interpolator

        .. autoclass:: quicktex.s3tc.bc1::BC1Encoder.ColorMode

        **Advanced API**

        Additional properties are provided for finer-grained control over quality and performance

        .. autoproperty:: error_mode(self) -> ErrorMode
        .. autoproperty:: endpoint_mode(self) -> EndpointMode

        .. autoproperty:: two_ls_passes(self) -> bool
        .. autoproperty:: two_ep_passes(self) -> bool
        .. autoproperty:: two_cf_passes(self) -> bool

        .. autoproperty:: exhaustive(self) -> bool
        .. autoproperty:: search_rounds(self) -> int
        .. autoproperty:: orderings(self) -> tuple[int, int]
        .. autoproperty:: power_iterations(self) -> int
        .. autoattribute:: max_power_iterations
        .. autoattribute:: min_power_iterations

        .. autoclass:: quicktex.s3tc.bc1::BC1Encoder.EndpointMode
        .. autoclass:: quicktex.s3tc.bc1::BC1Encoder.ErrorMode

    .. autoclass:: BC1Decoder(BlockDecoder)

        .. automethod:: __init__
        .. autoproperty:: interpolator(self) -> quicktex.s3tc.interpolator.Interpolator
        .. autoproperty:: write_alpha(self) -> bool

bc3 module
----------
.. automodule:: quicktex.s3tc.bc3

    .. autoclass:: BC3Encoder(BlockEncoder)

        .. automethod:: __init__
        .. autoproperty:: bc1_encoder(self) -> quicktex.s3tc.bc1.BC1Encoder
        .. autoproperty:: bc4_encoder(self) -> quicktex.s3tc.bc4.BC4Encoder

    .. autoclass:: BC3Decoder(BlockDecoder)

        .. automethod:: __init__
        .. autoproperty:: bc1_decoder(self) -> quicktex.s3tc.bc1.BC1Decoder
        .. autoproperty:: bc4_decoder(self) -> quicktex.s3tc.bc4.BC4Decoder

bc4 module
----------
.. automodule:: quicktex.s3tc.bc4

    .. autoclass:: BC4Encoder(BlockEncoder)

        .. automethod:: __init__
        .. autoproperty:: channel(self) -> int

    .. autoclass:: BC4Decoder(BlockDecoder)

        .. automethod:: __init__
        .. autoproperty:: channel(self) -> int

bc5 module
----------
.. automodule:: quicktex.s3tc.bc5

    .. autoclass:: BC5Encoder(BlockEncoder)

        .. automethod:: __init__
        .. autoproperty:: bc4_encoders(self) -> tuple[quicktex.s3tc.bc4.BC4Encoder]
        .. autoproperty:: channels(self) -> tuple[int, int]

    .. autoclass:: BC5Decoder(BlockDecoder)

        .. automethod:: __init__
        .. autoproperty:: bc4_decoders(self) -> tuple[quicktex.s3tc.bc4.BC4Decoder]
        .. autoproperty:: channels(self) -> tuple[int, int]

interpolator module
-------------------

.. automodule:: quicktex.s3tc.interpolator
    :members:

    .. autoclass:: Interpolator
    .. autoclass:: InterpolatorRound(Interpolator)
    .. autoclass:: InterpolatorNvidia(Interpolator)
    .. autoclass:: InterpolatorAMD(Interpolator)
