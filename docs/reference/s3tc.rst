.. py:currentmodule:: quicktex.s3tc

s3tc module
===========

.. pybind handles enums in a really weird way that doesnt play nice with Sphinx,
   so the docs are rewritten here.

.. py:class:: InterpolatorType

   An enum representing various methods for interpolating colors, used by the BC1 and BC3 encoders/decoders.
   Vendor-specific interpolation modes should only be used when the result will only be used on that type of GPU.
   For most applications, :py:attr:`~quicktex.s3tc.InterpolatorType.Ideal` should be used.

   .. py:data:: Ideal

   The default mode, with no rounding for colors 2 and 3. This matches the D3D10 docs on BC1.

   .. py:data:: IdealRound

   Round colors 2 and 3. Matches the AMD Compressonator tool and the D3D9 docs on DXT1.

   .. py:data:: Nvidia

   Nvidia GPU mode.

   .. py:data:: AMD

   AMD GPU mode.

bc1 module
----------

.. automodule:: quicktex.s3tc.bc1

   .. autoclass:: BC1Encoder
      :members:
      :undoc-members:

   .. autoclass:: BC1Decoder
      :members:
      :undoc-members:

bc3 module
----------

.. automodule:: quicktex.s3tc.bc3

   .. autoclass:: BC3Encoder
      :members:
      :undoc-members:

   .. autoclass:: BC3Decoder
      :members:
      :undoc-members:

bc4 module
----------

.. automodule:: quicktex.s3tc.bc4

   .. autoclass:: BC4Encoder
      :members:
      :undoc-members:

   .. autoclass:: BC4Decoder
      :members:
      :undoc-members:

bc5 module
----------

.. automodule:: quicktex.s3tc.bc5

   .. autoclass:: BC5Encoder
      :members:
      :undoc-members:

   .. autoclass:: BC5Decoder
      :members:
      :undoc-members: