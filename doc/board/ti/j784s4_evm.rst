.. SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
.. sectionauthor:: Apurva Nandan <a-nandan@ti.com>

J784S4 and AM69 Platforms
=========================

Introduction
------------
The J784S4 SoC belongs to the K3 Multicore SoC architecture
platform, providing advanced system integration in automotive,
ADAS and industrial applications requiring AI at the network edge.
This SoC extends the K3 Jacinto 7 family of SoCs with focus on
raising performance and integration while providing interfaces,
memory architecture and compute performance for multi-sensor, high
concurrency applications.

The device is partitioned into three functional domains, each containing
specific processing cores and peripherals:

1. Wake-up (WKUP) domain
    * ARM Cortex-M4F processor, runs TI Foundational Security (TIFS)

2. Microcontroller (MCU) domain
    * Dual core ARM Cortex-R5F processor, runs device management
      and SoC early boot

3. MAIN domain
    * Two clusters of quad core 64-bit ARM Cortex-A72, runs HLOS
    * Dual core ARM Cortex-R5F processor used for RTOS applications
    * Four C7x DSPs used for Machine Learning applications.


More info can be found in TRM: http://www.ti.com/lit/zip/spruj52

Platform information:

* https://www.ti.com/tool/J784S4XEVM
* https://www.ti.com/tool/SK-AM69

Boot Flow
---------
Below is the pictorial representation of boot flow:

.. image:: img/boot_diagram_k3_current.svg
    :alt: K3 boot flow

- On this platform, "TI Foundational Security" (TIFS) functions as the
  security enclave master. While "Device Manager" (DM), also known as the
  "TISCI server" in TI terminology, offers all the essential services.

- As illustrated in the diagram above, R5 SPL manages power and clock
  services independently before handing over control to DM. The A72 or
  the C7x (Aux core) software components request TIFS/DM to handle
  security or device management services.

Sources
-------

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_boot_sources
    :end-before: .. k3_rst_include_end_boot_sources

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_boot_firmwares
    :end-before: .. k3_rst_include_end_boot_firmwares

Build procedure
---------------
0. Setup the environment variables:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_desc
    :end-before: .. k3_rst_include_end_common_env_vars_desc

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_board_env_vars_desc
    :end-before: .. k3_rst_include_end_board_env_vars_desc

Set the variables corresponding to this platform:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_defn
    :end-before: .. k3_rst_include_end_common_env_vars_defn
.. prompt:: bash

   export UBOOT_CFG_CORTEXR=j784s4_evm_r5_defconfig
   export UBOOT_CFG_CORTEXA=j784s4_evm_a72_defconfig
   export TFA_BOARD=j784s4
   export TFA_EXTRA_ARGS="K3_USART=0x8"
   export OPTEE_PLATFORM=k3-j784s4
   export OPTEE_EXTRA_ARGS="CFG_CONSOLE_UART=0x8"

.. note::

   For AM69-SK, use the following U_BOOT_CFG instead:

   .. prompt:: bash

      export UBOOT_CFG_CORTEXR=am69_sk_r5_defconfig
      export UBOOT_CFG_CORTEXA=am69_sk_a72_defconfig

.. j784s4_evm_rst_include_start_build_steps

1. Trusted Firmware-A

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_tfa
    :end-before: .. k3_rst_include_end_build_steps_tfa


2. OP-TEE

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_optee
    :end-before: .. k3_rst_include_end_build_steps_optee

3. U-Boot

.. _j784s4_evm_rst_u_boot_r5:

* 3.1 R5

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_spl_r5
    :end-before: .. k3_rst_include_end_build_steps_spl_r5

.. _j784s4_evm_rst_u_boot_a72:

* 3.2 A72

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_uboot
    :end-before: .. k3_rst_include_end_build_steps_uboot
.. j784s4_evm_rst_include_end_build_steps

Target Images
-------------
In order to boot we need tiboot3.bin, tispl.bin and u-boot.img. Each SoC
variant (GP, HS-FS, HS-SE) requires a different source for these files.

 - GP

    * tiboot3-j784s4-gp-evm.bin from :ref:`step 3.1 <j784s4_evm_rst_u_boot_r5>`
    * tispl.bin_unsigned, u-boot.img_unsigned from :ref:`step 3.2 <j784s4_evm_rst_u_boot_a72>`

 - HS-FS

    * tiboot3-j784s4-hs-fs-evm.bin from :ref:`step 3.1 <j784s4_evm_rst_u_boot_r5>`
    * tispl.bin, u-boot.img from :ref:`step 3.2 <j784s4_evm_rst_u_boot_a72>`

 - HS-SE

    * tiboot3-j784s4-hs-evm.bin from :ref:`step 3.1 <j784s4_evm_rst_u_boot_r5>`
    * tispl.bin, u-boot.img from :ref:`step 3.2 <j784s4_evm_rst_u_boot_a72>`

Image formats
-------------

- tiboot3.bin

.. image:: img/multi_cert_tiboot3.bin.svg
    :alt: tiboot3.bin format

- tispl.bin

.. image:: img/dm_tispl.bin.svg
    :alt: tispl.bin format

OSPI:
-----
ROM supports booting from OSPI from offset 0x0.

Flashing images to OSPI NOR:

Below commands can be used to download tiboot3.bin, tispl.bin, and
u-boot.img over tftp and then flash those to OSPI at their respective
addresses.

.. prompt:: bash =>

  sf probe
  tftp ${loadaddr} tiboot3.bin
  sf update $loadaddr 0x0 $filesize
  tftp ${loadaddr} tispl.bin
  sf update $loadaddr 0x80000 $filesize
  tftp ${loadaddr} u-boot.img
  sf update $loadaddr 0x280000 $filesize

Flash layout for OSPI NOR:

.. image:: img/ospi_sysfw3.svg
  :alt: OSPI NOR flash partition layout

R5 Memory Map
-------------

.. list-table::
   :widths: 16 16 16
   :header-rows: 1

   * - Region
     - Start Address
     - End Address

   * - SPL
     - 0x41c00000
     - 0x41c40000

   * - EMPTY
     - 0x41c40000
     - 0x41c61f20

   * - STACK
     - 0x41c65f20
     - 0x41c61f20

   * - Global data
     - 0x41c65f20
     - 0x41c66000

   * - Heap
     - 0x41c66000
     - 0x41c76000

   * - BSS
     - 0x41c76000
     - 0x41c80000

   * - DM DATA
     - 0x41c80000
     - 0x41c84130

   * - EMPTY
     - 0x41c84130
     - 0x41cff9fc

   * - MCU Scratchpad
     - 0x41cff9fc
     - 0x41cffbfc

   * - ROM DATA
     - 0x41cffbfc
     - 0x41cfffff

Switch Setting for Boot Mode
----------------------------

Boot Mode pins provide means to select the boot mode and options before the
device is powered up. After every POR, they are the main source to populate
the Boot Parameter Tables.

Boot Mode Pins for J784S4-EVM
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following tables show some common boot modes used on J784S4 EVM platform.
More details can be found in the Technical Reference Manual:
http://www.ti.com/lit/zip/spruj52 under the `Boot Mode Pins` section.

.. list-table:: J784S4 EVM Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW11: 12345678
     - SW7: 12345678

   * - SD
     - 10000010
     - 00000000

   * - EMMC
     - 10000000
     - 01000000

   * - OSPI
     - 00000110
     - 01000000

   * - UART
     - 00000000
     - 01110000

For SW7 and SW11, the switch state in the "ON" position = 1.

Boot Mode Pins for AM69-SK
^^^^^^^^^^^^^^^^^^^^^^^^^^

The following table show some common boot modes used on AM69-SK platform.
More details can be found in the User Guide for AM69-SK:
https://www.ti.com/lit/ug/spruj70/spruj70.pdf under the `Bootmode Settings`
section.

.. list-table:: AM69 SK Boot Modes
   :widths: 16 16
   :header-rows: 1

   * - Switch Label
     - SW2: 1234

   * - SD
     - 0000

   * - OSPI
     - 0010

   * - EMMC
     - 0110

   * - UART
     - 1010

For SW2, the switch state in the "ON" position = 1.

Debugging U-Boot
----------------

See :ref:`Common Debugging environment - OpenOCD<k3_rst_refer_openocd>`: for
detailed setup information.

.. warning::

  **OpenOCD support since**: September 2023 (git master)

  Until the next stable release of OpenOCD is available in your development
  environment's distribution, it might be necessary to build OpenOCD `from the
  source <https://github.com/openocd-org/openocd>`_.

Debugging U-Boot on J784S4-EVM and AM69-SK
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_openocd_connect_XDS110
    :end-before: .. k3_rst_include_end_openocd_connect_XDS110

To start OpenOCD and connect to J784S4-EVM or AM69-SK board, use the
following.

.. prompt:: bash

  openocd -f board/ti_j784s4evm.cfg
