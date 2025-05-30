.. SPDX-License-Identifier: GPL-2.0+

Sipeed's Lichee PI 4A based on T-HEAD TH1520 SoC
================================================

The LicheePi4A is a high-performance RISC-V SBC based on TH1520(4xC910@1.85GHz),
comes with 4/8/16 GB RAM, and up to 128GB eMMC, and rich peripherals.

 - SoC            T-HEAD TH1520 SoC
 - System Memory  4GB, 8GB, or 16GB LPDDR4X
 - Storage        eMMC flash with 8/32/128 GB
 -                external microSD slot
 - Networking     2x Gigabit Ethernet
 -                WiFi+BT
 - Display        HDMI2.0, 4-lane MIPI DSI
 - Camera         4-lane MIPI CSI + 2x2-lane MIPI CSI
 - Audio          Onboard Speaker, 2xMEMS MIC, 3.5mm headphone jack
 - USB            4xUSB3.0 Type-A, 1xUSB2.0 Type-C
 - GPIO           2x10Pin breakout, UART/IIC/SPI
 - Power          DC 12V/2A, POE 5V/2.4A, USB Type-C 5V/2A

TH1520 RISC-V SoC
-----------------

The TH1520 SoC consist of quad-core RISC-V Xuantie C910 (RV64GCV) processor,
Xuantie C906 audio DSP, low power Xuantie E902 core, it also integrate
Imagination GPU for graphics, and 4 TOPS NPU for AI acceleration.

Mainline support
----------------

The support for following drivers are already enabled:

1. ns16550 UART Driver.
2. eMMC and SD card


Building
~~~~~~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

   export CROSS_COMPILE=<riscv64 toolchain prefix>

3. Build DDR firmware

DDR driver requires a firmware to function, to build it:

.. code-block:: bash

	git clone --depth 1 https://github.com/ziyao233/th1520-firmware
	cd th1520-firmware
	lua5.4 ddr-generate.lua src/<CONFIGURATION_NAME>.lua th1520-ddr-firmware.bin

4. Build OpenSBI Firmware

TH1520 port of proper U-Boot runs in S mode, thus OpenSBI is required as
SBI firmware to setup S-mode environment and provide SBI calls. It could
be cloned and built for TH1520 as below,

.. code-block:: bash

	git clone https://github.com/riscv-software-src/opensbi.git
	cd opensbi
	make PLATFORM=generic

TH1520 support in OpenSBI requires v1.2 or a more recent version.

More detailed description of steps required to build fw_dynamic firmware
is beyond the scope of this document. Please refer to OpenSBI
documenation.

5. Build U-Boot images

The DDR firmware should be copied to U-Boot source directory before
building.

.. code-block:: bash

	cd <U-Boot-dir>
	cp <path-to-ddr-firmware> th1520-ddr-firmware.bin
	make th1520_lpi4a_defconfig
	make OPENSBI=<opensbi_dir>/build/platform/generic/firmware/fw_dynamic.bin

This will generate u-boot-with-spl.bin, which contains SPL, DDR firmware,
OpenSBI firmware and proper U-Boot.

Booting
~~~~~~~

u-boot-with-spl.bin should be loaded to SRAM through fastboot. Connect
the board to computer with Type-C cable and run

.. code-block:: bash

	fastboot flash ram u-boot-with-spl.bin
	fastboot reboot

Sample boot log from Lichee PI 4A board via fastboot
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

	[APP][E] protocol_connect failed, exit.
	Starting download of 940681 bytes

	downloading of 940681 bytes finished

	U-Boot SPL 2025.07-rc3-00005-g3a0ef515b8bb (May 29 2025 - 10:42:46 +0000)
	Trying to boot from RAM


	U-Boot 2025.07-rc3-00005-g3a0ef515b8bb (May 29 2025 - 10:42:46 +0000)

	CPU:   thead,c910
	Model: Sipeed Lichee Pi 4A
	DRAM:  8 GiB
	Core:  110 devices, 9 uclasses, devicetree: separate
	MMC:   mmc@ffe7080000: 0, mmc@ffe7090000: 1
	Loading Environment from <NULL>... OK
	In:    serial@ffe7014000
	Out:   serial@ffe7014000
	Err:   serial@ffe7014000
	Model: Sipeed Lichee Pi 4A
	LPI4A=>
