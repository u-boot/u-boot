.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Wadim Egorov <w.egorov@phytec.de>

phyCORE-AM64x
=============

The `phyCORE-AM64x <https://www.phytec.com/product/phycore-am64x>`_ is a
SoM (System on Module) featuring TI's AM64x SoC. It can be used in combination
with different carrier boards. This module can come with different sizes and
models for DDR, eMMC, SPI NOR Flash and various SoCs from the AM64x family.

A development Kit, called `phyBOARD-Electra <https://www.phytec.com/product/phyboard-am64x>`_
is used as a carrier board reference design around the AM64x SoM.

Quickstart
----------

* Download sources and TI firmware blobs
* Build Trusted Firmware-A
* Build OP-TEE
* Build U-Boot for the R5
* Build U-Boot for the A53
* Create bootable uSD Card
* Boot

Sources
-------

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_boot_sources
    :end-before: .. k3_rst_include_end_boot_sources

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_boot_firmwares_sysfw
    :end-before: .. k3_rst_include_end_boot_firmwares_sysfw

Build procedure
---------------

Setup the environment variables:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_desc
    :end-before: .. k3_rst_include_end_common_env_vars_desc

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_board_env_vars_desc
    :end-before: .. k3_rst_include_end_board_env_vars_desc

Set the variables corresponding to this platform:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_defn
    :end-before: .. k3_rst_include_end_common_env_vars_defn
.. code-block:: bash

 $ export UBOOT_CFG_CORTEXR=phycore_am64x_r5_defconfig
 $ export UBOOT_CFG_CORTEXA=phycore_am64x_a53_defconfig
 $ export TFA_BOARD=lite
 $ # we don't use any extra TFA parameters
 $ unset TFA_EXTRA_ARGS
 $ export OPTEE_PLATFORM=k3-am64x
 # we don't use any extra OPTEE parameters
 unset OPTEE_EXTRA_ARGS

.. include::  ../ti/am62x_sk.rst
    :start-after: .. am62x_evm_rst_include_start_build_steps
    :end-before: .. am62x_evm_rst_include_end_build_steps

uSD Card creation
-----------------

Use fdisk to partition the uSD card. The layout should look similar to:

.. code-block:: bash

 $ sudo fdisk -l /dev/mmcblk0
 Disk /dev/mmcblk0: 7.56 GiB, 8120172544 bytes, 15859712 sectors
 Units: sectors of 1 * 512 = 512 bytes
 Sector size (logical/physical): 512 bytes / 512 bytes
 I/O size (minimum/optimal): 512 bytes / 512 bytes
 Disklabel type: dos
 Disk identifier: 0x6583d9a3

 Device         Boot  Start     End Sectors   Size Id Type
 /dev/mmcblk0p1 *      2048  264191  262144   128M  c W95 FAT32 (LBA)
 /dev/mmcblk0p2      264192 1934953 1670762 815.8M 83 Linux


Once partitioned, the boot partition has to be formatted with a FAT filesystem.
Assuming the uSD card is `/dev/mmcblk0`:

.. code-block:: bash

 $ mkfs.vfat /dev/mmcblk0p1

To boot from a micro SD card on a HSFS device simply copy the following
artifacts to the FAT partition:

* tiboot3.bin from R5 build
* tispl.bin from Cortex-A build
* u-boot.img from Cortex-A build

Boot
----

Put the uSD card in the slot on the board and apply power. Check the serial
console for output.

Flash to SPI NOR
----------------

Below commands can be used to flash the SPI NOR flash; assuming
tiboot3.bin, tispl.bin and u-boot.img are stored on the uSD card.

.. code-block:: bash

  mtd list
  fatload mmc 1 ${loadaddr} tiboot3.bin
  mtd write ospi.tiboot3 ${loadaddr} 0 ${filesize}
  fatload mmc 1 ${loadaddr} tispl.bin
  mtd write ospi.tispl ${loadaddr} 0 ${filesize}
  fatload mmc 1 ${loadaddr} u-boot.img
  mtd write ospi.u-boot ${loadaddr} 0 ${filesize}

UART based boot
---------------

To boot the board via UART, set the switches to UART mode and connect to the
micro USB port labeled as "Debug UART". After power-on the build artifacts
needs to be uploaded one by one with a tool like sz.

Example bash script sequence for running on a Linux host PC feeding all boot
artifacts needed to the device. Assuming the host uses /dev/ttyUSB0 as
the main domain serial port:

.. prompt:: bash $

  stty -F /dev/ttyUSB0 115200
  sb --xmodem tiboot3.bin > /dev/ttyUSB0 < /dev/ttyUSB0
  # Resend tiboot3.bin a 2nd time due to ErrataID:i2331
  sb --xmodem tiboot3.bin > /dev/ttyUSB0 < /dev/ttyUSB0
  sb --ymodem tispl.bin > /dev/ttyUSB0 < /dev/ttyUSB0
  sb --ymodem u-boot.img > /dev/ttyUSB0 < /dev/ttyUSB0

Boot Modes
----------

The phyCORE-AM64x development kit supports booting from many different
interfaces. By default, the development kit is set to boot from the micro-SD
card. To change the boot device, DIP switches S5 and S6 can be used.
Boot switches should be changed with power off.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW3: 12345678
     - SW4: 12345678

   * - uSD
     - 11000010
     - 01000000

   * - eMMC
     - 11010010
     - 00000000

   * - OSPI
     - 11010000
     - 10000000

   * - UART
     - 11011100
     - 00000000

.. include:: k3-common.rst

Further Information
-------------------

Please see :doc:`../ti/am64x_evm` chapter for further AM64 SoC related documentation
and https://docs.phytec.com/projects/yocto-phycore-am64x/en/latest/ for vendor documentation.
