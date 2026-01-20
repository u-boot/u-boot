.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Paresh Bhagat <p-bhagat@ti.com>

AM62D Platforms
===============

Introduction:
-------------
The AM62D2 SoC belongs to the K3 Multicore SoC architecture with DSP core
targeted for applications needing high-performance Digital Signal
Processing. It is used in applications like automotive audio systems,
professional sound equipment, radar and radio for aerospace, sonar in
marine devices, and ultrasound in medical imaging. It also supports
precise signal analysis in test and measurement tools.

Some highlights of AM62D2 SoC are:

* Quad-Cortex-A53s (running up to 1.4GHz) in a single cluster. Dual/Single
  core variants are provided in the same package to allow HW compatible
  designs.
* One Device manager Cortex-R5F for system power and resource management,
  and one Cortex-R5F for Functional Safety or general-purpose usage.
* DSP with Matrix Multiplication Accelerator(MMA) (up to 2 TOPS) based on
  single core C7x.
* 3x Multichannel Audio Serial Ports (McASP) Up to 4/6/16 Serial Data Pins
  which can Transmit and Receive Clocks up to 50MHz, with multi-channel I2S
  and TDM Audio inputs and outputs.
* Integrated Giga-bit Ethernet switch supporting up to a total of two
  external ports with TSN capable to enable audio networking features such
  as, Ethernet Audio Video Bridging (eAVB) and Dante.
* 9xUARTs, 5xSPI, 6xI2C, 2xUSB2, 3xCAN-FD, 3x eMMC and SD, OSPI memory
  controller, 1x CSI-RX-4L for Camera, eCAP/eQEP, ePWM, among other
  peripherals.
* Dedicated Centralized Hardware Security Module with support for secure
  boot, debug security and crypto acceleration and trusted execution
  environment.
* One 32 bit DDR Subsystem that supports LPDDR4, DDR4 memory types.
* Low power mode support: Partial IO support for CAN/GPIO/UART wakeup.

This SoC is of part K3 AM62x family, which includes the AM62A and AM62P
variants. While the AM62A and AM62D are largely similar, the AM62D is
specifically targeted for general-purpose DSP applications, whereas the
AM62A focuses on edge AI workloads. A key distinction is that the AM62D
does not include multimedia components such as the video encoder/decoder,
MJPEG encoder, Vision Processing Accelerator (VPAC) for image signal
processing, or the display subsystem. Additionally, the AM62D has a
different pin configuration compared to the AM62A, which impacts
embedded software development.

More details can be found in the Technical Reference Manual:
https://www.ti.com/lit/pdf/sprujd4

Platform information:

* https://www.ti.com/tool/AUDIO-AM62D-EVM

Boot Flow:
----------
Below is the pictorial representation of boot flow:

.. image:: img/boot_diagram_am62.svg
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
    :end-before: .. k3_rst_include_end_tifsstub

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

   export UBOOT_CFG_CORTEXR=am62dx_evm_r5_defconfig
   export UBOOT_CFG_CORTEXA=am62dx_evm_a53_defconfig
   export TFA_BOARD=lite
   # we dont use any extra TFA parameters
   unset TFA_EXTRA_ARGS
   export OPTEE_PLATFORM=k3-am62ax
   # we dont use any extra OPTEE parameters
   unset OPTEE_EXTRA_ARGS

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

Target Images
--------------
In order to boot we need tiboot3.bin, tispl.bin and u-boot.img.  Each SoC
variant (HS-FS, HS-SE) requires a different source for these files.

 - HS-FS

        * tiboot3-am62ax-hs-fs-evm.bin from step 3.1
        * tispl.bin, u-boot.img from step 3.2

 - HS-SE

        * tiboot3-am62ax-hs-evm.bin from step 3.1
        * tispl.bin, u-boot.img from step 3.2

Image formats:
--------------

- tiboot3.bin

.. image:: img/multi_cert_tiboot3.bin.svg
  :alt: tiboot3.bin image format

- tispl.bin

.. image:: img/tifsstub_dm_tispl.bin.svg
  :alt: tispl.bin image format

Switch Setting for Boot Mode
----------------------------

Boot Mode pins provide means to select the boot mode and options before the
device is powered up. After every POR, they are the main source to populate
the Boot Parameter Tables.

The following table shows some common boot modes used on AM62D platform. More
details can be found in the Technical Reference Manual:
https://www.ti.com/lit/pdf/sprujd4 under the `Boot Mode Pins` section.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW2: 12345678
     - SW3: 12345678

   * - SD
     - 01000000
     - 11000010

   * - OSPI
     - 00000000
     - 11001110

   * - EMMC
     - 00000000
     - 11010010

   * - UART
     - 00000000
     - 11011100

   * - USB DFU
     - 00000000
     - 11001010

For SW2 and SW1, the switch state in the "ON" position = 1.

Falcon Mode
-----------

Falcon Mode on AM62dx platforms bypasses the A53 SPL and U-Boot with the overall
boot flow as below:

.. include:: am62x_sk.rst
    :start-after: .. am62x_evm_falcon_start_boot_flow
    :end-before: .. am62x_evm_falcon_end_boot_flow

Build Process
^^^^^^^^^^^^^

.. include:: am62x_sk.rst
    :start-after: .. am62x_evm_falcon_start_build_process
    :end-before: .. am62x_evm_falcon_end_build_process

Usage
^^^^^

.. include:: am62x_sk.rst
    :start-after: .. am62x_evm_falcon_start_usage
    :end-before: .. am62x_evm_falcon_end_usage

R5 SPL Memory Map
^^^^^^^^^^^^^^^^^

.. include:: am62x_sk.rst
    :start-after: .. am62x_evm_falcon_start_r5_memory_map
    :end-before: .. am62x_evm_falcon_end_r5_memory_map

Debugging U-Boot
----------------

See :ref:`Common Debugging environment - OpenOCD<k3_rst_refer_openocd>`: for
detailed setup information.

.. warning::

  **AM62A**: OpenOCD support since August 2023 (git master)

  **AM62D**: Uses AM62A configuration (compatible due to same core architecture)

  Until the next stable release of OpenOCD is available in your development
  environment's distribution, it might be necessary to build OpenOCD `from the
  source <https://github.com/openocd-org/openocd>`_.

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_openocd_connect_XDS110
    :end-before: .. k3_rst_include_end_openocd_connect_XDS110

To start OpenOCD and connect to the board

.. prompt:: bash

  openocd -f board/ti/am62a7evm.cfg
