.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Kaustabh Chakraborty <kauschluss@disroot.org>

Samsung Exynos 7870 Boards
==========================

Preparation
-----------

Pack the device tree blob in the QCDT format [1]_ using ``dtbTool-exynos`` [2]_
by issuing the following commands:

.. prompt:: bash $

	dtbTool-exynos -o stub-dt.img .output/u-boot.dtb

Finally, use ``mkbootimg`` by osm0sis [3]_ to generate the boot image:

.. prompt:: bash $

	mkbootimg -o u-boot.img \
		--kernel	.output/u-boot-nodtb.bin \
		--dt		stub-dt.img

Offsets are not provided to ``mkbootimg`` as S-BOOT ignores them.

Flashing
--------
If flashing for the first time, it must be done via Samsung's Download (Odin)
mode. Heimdall [4]_ can be used for flashing, like so:

.. prompt:: bash $

	heimdall flash --BOOT u-boot.img

However, if U-Boot is already installed, you may also use its fastboot interface
for flashing. Boot into the boot menu by holding the volume down key. Enable
fastboot mode from there, connect the device to your host, then run:

.. prompt:: bash $

	fastboot flash boot u-boot.img

To flash an OS image in internal storage, fastboot is a reliable option.

References
----------
.. [1] https://wiki.postmarketos.org/wiki/QCDT
.. [2] https://github.com/dsankouski/dtbtool-exynos
.. [3] https://github.com/osm0sis/mkbootimg
.. [4] https://git.sr.ht/~grimler/Heimdall
