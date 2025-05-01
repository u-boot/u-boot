.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Jayesh Choudhary <j-choudhary@ti.com>

J722S-EVM Platform
==================

The J722S is a family of  application processors built for Automotive and
Linux Application development. J722S family of SoCs is a superset of the
AM62P SoC family and shares similar memory map, thus the nodes are being
reused from AM62P includes instead of duplicating the definitions.

Some highlights of J722S SoC (in addition to AM62P SoC features) are:

* Two Cortex-R5F for Functional Safety or general-purpose usage and
  two C7x floating point vector DSP with Matrix Multiply Accelerator
  for deep learning.

* Vision Processing Accelerator (VPAC) with image signal processor
  and Depth and Motion Processing Accelerator (DMPAC).

* 7xUARTs, 3xSPI, 5xI2C, 2xUSB2, 2xCAN-FD, 3xMMC and SD, GPMC for
  NAND/FPGA connection, OSPI memory controller, 5xMcASP for audio,
  4xCSI-RX for Camera, 1 PCIe Gen3 controller, USB3.0 eCAP/eQEP,
  ePWM, among other peripherals.

For those interested, more details about this SoC can be found in the
Technical Reference Manual here: https://www.ti.com/lit/zip/sprujb3

Boot Flow:
----------

The bootflow is exactly the same as all SoCs in the am62xxx extended SoC
family. Below is the pictorial representation:

.. image:: img/boot_diagram_k3_current.svg
  :alt: Boot flow diagram

- Here TIFS acts as master and provides all the critical services. R5/A53
  requests TIFS to get these services done as shown in the above diagram.

Sources:
--------

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_boot_sources
    :end-before: .. k3_rst_include_end_boot_sources

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_boot_firmwares
    :end-before: .. k3_rst_include_end_boot_firmwares

Build procedure:
----------------

0. Setup the environment variables:

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

.. prompt:: bash

   export UBOOT_CFG_CORTEXR=j722s_evm_r5_defconfig
   export UBOOT_CFG_CORTEXA=j722s_evm_a53_defconfig
   export TFA_BOARD=lite
   export OPTEE_PLATFORM=k3-am62x

.. j722s_evm_rst_include_start_build_steps

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
.. j722s_evm_rst_include_end_build_steps

Target Images
--------------

In order to boot we need tiboot3.bin, tispl.bin and u-boot.img.  Each SoC
variant (HS-FS, HS-SE) requires a different source for these files.

 - HS-FS

        * tiboot3-j722s-hs-fs-evm.bin from step 3.1
        * tispl.bin, u-boot.img from step 3.2

 - HS-SE

        * tiboot3-j722s-hs-evm.bin from step 3.1
        * tispl.bin, u-boot.img from step 3.2

Image formats:
--------------

- tiboot3.bin

.. image:: img/multi_cert_tiboot3.bin.svg
  :alt: tiboot3.bin image format

- tispl.bin

.. image:: img/dm_tispl.bin.svg
  :alt: tispl.bin image format

A53 SPL DDR Memory Layout
-------------------------

.. j722s_evm_rst_include_start_ddr_mem_layout

This provides an overview memory usage in A53 SPL stage.

.. list-table::
   :widths: 16 16 16
   :header-rows: 1

   * - Region
     - Start Address
     - End Address

   * - EMPTY
     - 0x80000000
     - 0x80080000

   * - TEXT BASE
     - 0x80080000
     - 0x800d8000

   * - EMPTY
     - 0x800d8000
     - 0x80477660

   * - STACK
     - 0x80477660
     - 0x80477e60

   * - GD
     - 0x80477e60
     - 0x80478000

   * - MALLOC
     - 0x80478000
     - 0x80480000

   * - EMPTY
     - 0x80480000
     - 0x80a00000

   * - BSS
     - 0x80a00000
     - 0x80a80000

   * - BLOBS
     - 0x80a80000
     - 0x80d00400

   * - EMPTY
     - 0x80d00400
     - 0x81000000
.. j722s_evm_rst_include_end_ddr_mem_layout

Switch Setting for Boot Mode
----------------------------

Boot Mode pins provide means to select the boot mode and options before the
device is powered up. After every POR, they are the main source to populate
the Boot Parameter Tables.

The following table shows some common boot modes used on J722S-EVM
platform. More details can be found in the Technical Reference Manual:
https://www.ti.com/lit/zip/sprujb3 under the `Boot Mode Pins` section.

.. note::

   This device is very new. Currently only UART boot is available while
   we continue to add support for the other bootmodes.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW3: 12345678
     - SW4: 12345678

   * - SD
     - 11000010
     - 01000000

   * - OSPI
     - 11001110
     - 00000000

   * - EMMC
     - 11010010
     - 00000000

   * - UART
     - 11011100
     - 00000000

   * - USB DFU
     - 11001010
     - 00000000

For SW2 and SW1, the switch state in the "ON" position = 1.

Debugging U-Boot
----------------

See :ref:`Common Debugging environment - OpenOCD<k3_rst_refer_openocd>`: for
detailed setup information.

.. warning::

  **OpenOCD support after**: v0.12.0

  While support for the entire K3 generation including the am62xxx
  extended family was added before v0.12.0, the tcl scripts for the
  am62px have been accepted and will be available in the next release of
  OpenOCD. It may be necessary to build OpenOCD from source depending on
  the version your distribution has packaged.

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_openocd_connect_XDS110
    :end-before: .. k3_rst_include_end_openocd_connect_XDS110

To start OpenOCD and connect to the board

.. prompt:: bash

  openocd -f board/ti_j722sevm.cfg
