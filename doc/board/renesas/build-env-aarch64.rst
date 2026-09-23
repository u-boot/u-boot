.. SPDX-License-Identifier: GPL-2.0+

Renesas 64-bit ARM SoC build environment setup
==============================================

In order to cross-compile U-Boot for the aarch64 target, it is necessary
to install a suitable toolchain. The toolchain is available in various
distribution repositories as ``aarch64-linux-gnu-*`` or similar package.

Debian example:

.. code-block:: console

    $ apt install binutils-aarch64-linux-gnu cpp-aarch64-linux-gnu gcc-aarch64-linux-gnu

Alternatively, it is possible to download prebuilt toolchain from
kernel.org cross-development toolchains page
https://www.kernel.org/pub/tools/crosstool/ .

Once the toolchain is installed, add toolchain into ``PATH`` variable:

.. code-block:: console

    $ export PATH=$PATH:<path/to/arm64/toolchain/bin/>
    $ export CROSS_COMPILE=aarch64-linux-gnu-
