.. SPDX-License-Identifier: GPL-2.0+

Renesas 32-bit ARM SoC build environment setup
==============================================

In order to cross-compile U-Boot for the ARM 32-bit target, it is necessary
to install a suitable toolchain. The toolchain is available in various
distribution repositories as ``arm-linux-gnueabi-*`` or similar package.

Debian example:

.. code-block:: console

    $ apt install binutils-arm-linux-gnueabi cpp-arm-linux-gnueabi gcc-arm-linux-gnueabi

Alternatively, it is possible to download prebuilt toolchain from
kernel.org cross-development toolchains page
https://www.kernel.org/pub/tools/crosstool/ .

Once the toolchain is installed, add toolchain into ``PATH`` variable:

.. code-block:: console

    $ export PATH=$PATH:<path/to/arm32/toolchain/bin/>
    $ export CROSS_COMPILE=arm-linux-gnueabi-
