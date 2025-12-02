.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Kaustabh Chakraborty <kauschluss@disroot.org>

Samsung Exynos Generic ARMv8 Boards (for mobile devices)
========================================================

Overview
--------
This document describes how to build and run U-Boot for Samsung Exynos generic
boards. Boards are expected to boot with a primary bootloader, such as S-BOOT or
S-LK, which hands off control to U-Boot. Presently, only ARMv8 devices are
supported.

The U-Boot image is built with all device tree blobs packed in a single FIT
image. During boot, it uses simple heuristics to detect the target board, and
subsequently the appropriate FDT is selected.

Installation
------------
Building
^^^^^^^^
If a cross-compiler is required, install it and set it up like so:

.. prompt:: bash $

	export CROSS_COMPILE=aarch64-linux-gnu-

Then, run the following commands to build U-Boot:

.. prompt:: bash $

	make O=.output exynos-mobile_defconfig
	make O=.output -j$(nproc)

If successful, the U-Boot binary will be present in ``.output/u-boot.bin``.

Preparation and Flashing
^^^^^^^^^^^^^^^^^^^^^^^^
Since U-Boot supports multiple boards, and devices have different requirements,
this step will vary depending on your target.

.. toctree::
	:maxdepth: 1

	exynos-mobile/exynos7870
