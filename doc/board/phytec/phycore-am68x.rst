.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Dominik Haller <d.haller@phytec.de>

phyCORE-AM68x/TDA4x
===================

The `phyCORE-AM68x/TDA4x <https://www.phytec.eu/en/produkte/system-on-modules/phycore-am68x-tda4x/>`_
is a SoM (System on Module) populated with AM68x or TDA4x SoCs from TI's J721S2
family. It can be used in combination with different carrier boards. This module
can come with different sizes and models for DDR, eMMC, ETH-PHY, SPI NOR Flash
and an optional SN65DSI83 DSI->LVDS transceiver using DSI0.

A development Kit, called `phyBOARD-Izar <https://www.phytec.eu/en/produkte/development-kits/phyboard-izar/>`_
is used as a carrier board reference design around the AM68x/TDA4x SoM.

Quickstart
----------

* Download sources and TI firmware blobs
* Build Trusted Firmware-A
* Build OP-TEE
* Build U-Boot for the R5
* Build U-Boot for the A72
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

 $ export UBOOT_CFG_CORTEXR=phycore_am68x_r5_defconfig
 $ export UBOOT_CFG_CORTEXA=phycore_am68x_a72_defconfig
 $ export TFA_BOARD=generic
 $ export TFA_EXTRA_ARGS="K3_USART=0x8"
 $ export OPTEE_PLATFORM=k3-j784s4
 $ export OPTEE_EXTRA_ARGS="CFG_CONSOLE_UART=0x8"

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

* 3.2 A72:

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

Boot Modes
----------

The phyBOARD-Izar development kit supports booting from many different
interfaces. By default, the development kit is set to boot from the micro-SD
card. To change the boot device, DIP switches S8 and S7 can be used.
Boot switches should be changed with power off.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW8: 12345678
     - SW7: 12345678

   * - uSD
     - 00000000
     - 01000001

   * - eMMC (FS/UDA)
     - 00000000
     - 10001011

   * - eMMC (HW partitions)
     - 00000010
     - 10001011

   * - OSPI0
     - 00000010
     - 00001100

.. include:: k3-common.rst

Further Information
-------------------

Please see :doc:`../ti/j721s2_evm` chapter for further J721S2 SoC family
related documentation.
