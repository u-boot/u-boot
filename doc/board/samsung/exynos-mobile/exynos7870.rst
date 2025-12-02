.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Kaustabh Chakraborty <kauschluss@disroot.org>

Samsung Exynos 7870 Boards
==========================

Preparation
-----------
Create the following device tree (named ``stub.dts``)

.. code-block:: devicetree

	/dts-v1/;

	/ {
		compatible = "samsung,exynos7870";
		#address-cells = <2>;
		#size-cells = <1>;

		model_info-chip = <7870>;
		model_info-hw_rev = <0>;
		model_info-hw_rev_end = <255>;

		chosen {
		};

		memory@80000000 {
			device_type = "memory";
			reg = <0x0 0x80000000 0x0>;
		};

		memory@100000000 {
			device_type = "memory";
			reg = <0x1 0x00000000 0x0>;
		};
	};

The chosen node and memory ranges are populated by S-BOOT. A certain device
model may have multiple variants, with differing amounts of RAM and storage. The
RAM capacity information is graciously provided by S-BOOT's device tree
overlays.

Compile it to a device tree blob, then pack it in the QCDT format [1]_ using
``dtbTool-exynos`` [2]_ by issuing the following commands:

.. prompt:: bash $

	dtc -I dts -O dtb -o stub.dtb stub.dts
	dtbTool-exynos -o stub-dt.img stub.dtb

Finally, use ``mkbootimg`` by osm0sis [3]_ to generate the boot image:

.. prompt:: bash $

	mkbootimg -o u-boot.img \
		--kernel	.output/u-boot.bin \
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
