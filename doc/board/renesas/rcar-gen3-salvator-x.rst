.. SPDX-License-Identifier: GPL-2.0+

Renesas R-Car Gen3 H3/M3-W/M3-N Salvator-X(S) board
===================================================

- Renesas R-Car Gen3 Salvator-X board: https://elinux.org/R-Car/Boards/Salvator-X
- Renesas R-Car Gen3 Salvator-XS board: https://elinux.org/R-Car/Boards/Salvator-XS

Build U-Boot
------------

Please follow :doc:`Renesas 64-bit ARM SoC build environment setup <build-env-aarch64>`
to correctly set up the build environment before attempting to build U-Boot.

Clone up to date U-Boot source code and change directory into the
newly cloned source directory:

.. code-block:: console

    $ git clone https://source.denx.de/u-boot/u-boot.git/
    $ cd u-boot

Configure U-Boot:

.. code-block:: console

    $ make rcar3_salvator-x_defconfig

Compile U-Boot:

.. code-block:: console

    $ make

To speed up build process, -jN option can be passed to make to start
multiple jobs at the same time, this is beneficial especially on SMP
systems. The following example starts up to number of CPUs in the
system jobs, which is the recommended amount:

.. code-block:: console

    $ make -j$(nproc)

Install U-Boot
--------------

Please follow :doc:`Renesas R-Car Gen3 U-Boot installation <rcar-gen3-install>`
to install U-Boot into HyperFlash.
