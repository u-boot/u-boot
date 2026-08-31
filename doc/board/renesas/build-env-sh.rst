.. SPDX-License-Identifier: GPL-2.0+

Renesas 32-bit SH4 SoC build environment setup
==============================================

In order to cross-compile U-Boot for the SH4 32-bit target, it is necessary
to install a suitable toolchain. The toolchain is available in various
distribution repositories as ``sh4-linux-gnu-*`` or similar package.

Debian example:

.. code-block:: console

    $ apt install binutils-sh4-linux-gnu cpp-sh4-linux-gnu gcc-sh4-linux-gnu

Alternatively, it is possible to download prebuilt toolchain from
kernel.org cross-development toolchains page
https://www.kernel.org/pub/tools/crosstool/ .

Once the toolchain is installed, add toolchain into ``PATH`` variable:

.. code-block:: console

    $ export PATH=$PATH:<path/to/sh4/toolchain/bin/>
    $ export CROSS_COMPILE=sh4-linux-gnu-
