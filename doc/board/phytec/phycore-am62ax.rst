.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Garrett Giordano <ggiordano@phytec.com>

phyCORE-AM62Ax
==============

The `phyCORE-AM62Ax <https://www.phytec.com/product/phycore-am62a>`_ is a
SoM (System on Module) featuring TI's AM62Ax SoC. It can be used in combination
with different carrier boards. This module can come with different sizes and
models for DDR, eMMC, SPI NOR Flash and various SoCs from the AM62Ax family.

A development Kit, called `phyBOARD-Lyra <https://www.phytec.com/product/phyboard-am62x>`_
is used as a carrier board reference design around the AM62Ax SoM.

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
    :start-after: .. k3_rst_include_start_boot_firmwares
    :end-before: .. k3_rst_include_end_tifsstub

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

 $ export UBOOT_CFG_CORTEXR=phycore_am62ax_r5_defconfig
 $ export UBOOT_CFG_CORTEXA=phycore_am62ax_a53_defconfig
 $ export TFA_BOARD=lite
 $ # we dont use any extra TFA parameters
 $ unset TFA_EXTRA_ARGS
 $ export OPTEE_PLATFORM=k3-am62ax
 $ # we dont use any extra OPTEE parameters
 $ unset OPTEE_EXTRA_ARGS

1. Trusted Firmware-A:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_tfa
    :end-before: .. k3_rst_include_end_build_steps_tfa

2. OP-TEE:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_optee
    :end-before: .. k3_rst_include_end_build_steps_optee

3. U-Boot:

* 3.1 R5:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_spl_r5
    :end-before: .. k3_rst_include_end_build_steps_spl_r5

* 3.2 A53:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_uboot
    :end-before: .. k3_rst_include_end_build_steps_uboot

uSD Card creation
-----------------

Use fdisk to partition the uSD card. The layout should look similar to:

.. code-block:: bash

 $ sudo fdisk -l /dev/mmcblk0
 Disk /dev/mmcblk0: 15 GB, 15913189376 bytes, 31080448 sectors
 242816 cylinders, 4 heads, 32 sectors/track
 Units: sectors of 1 * 512 = 512 bytes

 Device       Boot StartCHS    EndCHS        StartLBA     EndLBA    Sectors  Size Id Type
 /dev/mmcblk0p1 *  128,0,1     1023,3,32        16384     278527     262144  128M  c Win95 FAT32 (LBA)
 /dev/mmcblk0p2    1023,3,32   1023,3,32       278528    1693883    1415356  691M 83 Linux


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
  sb --ymodem tispl.bin > /dev/ttyUSB0 < /dev/ttyUSB0
  sb --ymodem u-boot.img > /dev/ttyUSB0 < /dev/ttyUSB0

Boot Modes
----------

The phyCORE-AM62x development kit supports booting from many different
interfaces. By default, the development kit is set to boot from the micro-SD
card. To change the boot device, DIP switches S5 and S6 can be used.
Boot switches should be changed with power off.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW5: 12345678
     - SW6: 12345678

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

Please see :doc:`../ti/am62ax_sk` chapter for further AM62Ax SoC related documentation
and https://docs.phytec.com/projects/yocto-phycore-am62ax/en/latest/ for vendor documentation.
