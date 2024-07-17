.. SPDX-License-Identifier: GPL-2.0+

Milk-V Mars CM
==============

U-Boot for the Milk-V Mars CM uses the same U-Boot binaries as the VisionFive 2
board. In U-Boot SPL the actual board is detected and the device-tree patched
accordingly.

The Milk-V Mars CM Lite comes without eMMC and needs a different pin muxing
than the Milk-V Mars CM. The availability and size of the eMMC shows up in the
serial number displayed by the *mac* command, e.g.
MARC-V10-2340-D002E016-00000304. The number after the E is the MMC size. U-Boot
takes a value of E000 as an indicator for the Lite version. Unfortunately the
vendor has not set this value correctly on some Lite boards.

Please, use CONFIG_STARFIVE_NO_EMMC=y if EEPROM data indicates eMMC is present
on the Milk-V Mars CM Lite. Otherwise you will not be able to read from the
SD-card.

The serial number can be corrected using the *mac* command:

.. code-block::

    mac read_eeprom
    mac product_id MARC-V10-2340-D002E000-00000304
    mac write_eeprom

.. note::

   The *mac initialize* command overwrites the vendor string and the MAC
   addresses. This is why it is avoided here.

By default the EEPROM is write protected. The write protection may be overcome
by connecting the "GND" and "EN" test pads on top of the module.

Building
~~~~~~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

   export CROSS_COMPILE=<riscv64 toolchain prefix>

The M-mode software OpenSBI provides the supervisor binary interface (SBI) and
is responsible for the switch to S-Mode. It is a prerequisite to build U-Boot.
Support for the JH7110 was introduced in OpenSBI 1.2. It is recommended to use
a current release.

.. code-block:: console

	git clone https://github.com/riscv/opensbi.git
	cd opensbi
	make PLATFORM=generic FW_TEXT_START=0x40000000

(*FW_TEXT_START* is not needed anymore after OpenSBI patch d4d2582eef7a
"firmware: remove FW_TEXT_START" which should appear in OpenSBI 1.5.)

Now build the U-Boot SPL and U-Boot proper.

.. code-block:: console

	cd <U-Boot-dir>
	make starfive_visionfive2_defconfig
	make OPENSBI=$(opensbi_dir)/build/platform/generic/firmware/fw_dynamic.bin

This will generate the U-Boot SPL image (spl/u-boot-spl.bin.normal.out) as well
as the FIT image (u-boot.itb) with OpenSBI and U-Boot.

Device-tree selection
~~~~~~~~~~~~~~~~~~~~~

Depending on the board version U-Boot sets variable $fdtfile to either
starfive/jh7110-milkv-mars-cm.dtb (with eMMC storage) or
starfive/jh7110-milkv-mars-cm-lite.dtb (without eMMC storage).

To overrule this selection the variable can be set manually and saved in the
environment

::

    env set fdtfile my_device-tree.dtb
    env save

or the configuration variable CONFIG_DEFAULT_FDT_FILE can be used to set to
provide a default value.

The variable *$fdtfile* is used in the boot process to automatically load
a device-tree provided by the operating system. For details of the boot
process refer to the :doc:`/develop/bootstd/index`
description.

Boot source selection
~~~~~~~~~~~~~~~~~~~~~

The low speed connector nRPIBOOT line is used to switch the boot source.

* If nRPIBOOT is connected to ground, the board boots from UART.
* If nRPIBOOT is not connected, the board boots from SPI flash.

Compute module boards typically have a switch or jumper for this line.

Flashing a new U-Boot version
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

U-Boot SPL is provided as file spl/u-boot-spl.bin.normal.out. Main U-Boot is
in file u-boot.itb.

Assuming your new U-Boot version is on partition 1 of an SD-card you could
install it to the SPI flash with:

::

    sf probe
    load mmc 0:1 $kernel_addr_r u-boot-spl.bin.normal.out
    sf update $kernel_addr_r 0 $filesize
    load mmc 0:1 $kernel_addr_r u-boot.itb
    sf update $kernel_addr_r 0x100000 $filesize

For loading the files from a TFTP server refer to the dhcp and tftpboot
commands.

After updating U-Boot you may want to reboot and reset the environment to the
default.

::

    env default -f -a
    env save

Booting from UART
~~~~~~~~~~~~~~~~~

For booting via UART U-Boot must be built with CONFIG_SPL_YMODEM_SUPPORT=y.

With nRPIBOOT connected to ground for UART boot, power the board and upload
u-boot-spl.bin.normal.out via XMODEM. Then upload u-boot.itb via YMODEM.

The XMODEM implementation in the boot ROM is not fully specification compliant.
It sends too many NAKs in a row. Tio is a terminal emulation that tolerates
these faults.

::

    $ tio -b 115200 --databits 8 --flow none --stopbits 1 /dev/ttyUSB0
    [08:14:54.700] tio v2.7
    [08:14:54.700] Press ctrl-t q to quit
    [08:14:54.701] Connected

    (C)StarFive
    CCC
    (C)StarFive
    CCCCCCCC

Press *ctrl-t x* to initiate XMODEM-1K transfer.

::

    [08:15:14.778] Send file with XMODEM
    [08:15:22.459] Sending file 'u-boot-spl.bin.normal.out'
    [08:15:22.459] Press any key to abort transfer
    ........................................................................
    .......................................................................|
    [08:15:22.459] Done

    U-Boot SPL 2024.07-rc1-00075-gd6a4ab20097 (Apr 25 2024 - 16:32:10 +0200)
    DDR version: dc2e84f0.
    Trying to boot from UART
    CC

Press *ctrl-t y* to initiate YMODEM transfer.

::

    [08:15:50.331] Send file with YMODEM
    [08:15:53.540] Sending file 'u-boot.itb'
    [08:15:53.540] Press any key to abort transfer
    ........................................................................
    …
    ...............|
    [08:15:53.540] Done
    Loaded 1040599 bytes


    U-Boot 2024.07-rc1-00075-gd6a4ab20097 (Apr 25 2024 - 16:32:10 +0200)

Booting from SPI flash
~~~~~~~~~~~~~~~~~~~~~~

With nRPIBOOT disconnected from ground for SPI boot, power up the board. You
should see the U-Boot prompt on the serial console.
