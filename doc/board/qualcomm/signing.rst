.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Casey Connolly <casey.connolly@linaro.org>

Qualcomm Image Signing
======================

On some boards like the RB3 Gen 2 where U-Boot runs as the first stage bootloader,
it must be in a Qualcomm specific signed ELF format called ``mbn``.

For most boards this is handled automatically with the ``mkmbn`` tool in the U-Boot
build system. If you're bringing up a new platform which will run U-Boot as the first
stage bootloader, you may need to add your board and platform compatible string and
the load address used by your board to the ``boards`` table in ``tools/qcom/mkmbn/mkmbn.py``.

For example:

.. code-block:: python

	boards: dict[bytes, int] = {
		# Exact matches for boards, these are preferred
		# Don't forget the null terminator!
		b"qcom,qcs6490-rb3gen2\0": MbnData(0x9FC00000, 6, SwId.aboot),
	...
	}


When you run make to build the ``u-boot.mbn`` target, ``mkmbn`` will inspect the DTB in your
U-Boot image and try to match the compatible to the table, then it will build an ELF image and
hash/sign it per the MBN spec.
