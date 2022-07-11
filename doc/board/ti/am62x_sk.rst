.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Vignesh Raghavendra <vigneshr@ti.com>

Texas Instruments AM62 Platforms
================================

Introduction:
-------------
The AM62 SoC family is the follow on AM335x built on the K3 Multicore
SoC architecture platform, providing ultra-low-power modes, dual
display, multi-sensor edge compute, security and other BOM-saving
integrations.  The AM62 SoC targets a broad market to enable
applications such as Industrial HMI, PLC/CNC/Robot control, Medical
Equipment, Building Automation, Appliances and more.

Some highlights of this SoC are:

* Quad-Cortex-A53s (running up to 1.4GHz) in a single cluster.
  Pin-to-pin compatible options for single and quad core are available.
* Cortex-M4F for general-purpose or safety usage.
* Dual display support, providing 24-bit RBG parallel interface and
  OLDI/LVDS-4 Lane x2, up to 200MHz pixel clock support for 2K display
  resolution.
* Selectable GPU support, up to 8GFLOPS, providing better user experience
  in 3D graphic display case and Android.
* PRU(Programmable Realtime Unit) support for customized programmable
  interfaces/IOs.
* Integrated Giga-bit Ethernet switch supporting up to a total of two
  external ports (TSN capable).
* 9xUARTs, 5xSPI, 6xI2C, 2xUSB2, 3xCAN-FD, 3x eMMC and SD, GPMC for
  NAND/FPGA connection, OSPI memory controller, 3xMcASP for audio,
  1x CSI-RX-4L for Camera, eCAP/eQEP, ePWM, among other peripherals.
* Dedicated Centralized System Controller for Security, Power, and
  Resource Management.
* Multiple low power modes support, ex: Deep sleep, Standby, MCU-only,
  enabling battery powered system design.

More details can be found in the Technical Reference Manual:
https://www.ti.com/lit/pdf/spruiv7

Boot Flow:
----------
Below is the pictorial representation of boot flow:

.. code-block:: text

 +------------------------------------------------------------------------+
 |        TIFS            |      Main R5          |        A53            |
 +------------------------------------------------------------------------+
 |    +--------+          |                       |                       |
 |    |  Reset |          |                       |                       |
 |    +--------+          |                       |                       |
 |         :              |                       |                       |
 |    +--------+          |   +-----------+       |                       |
 |    | *ROM*  |----------|-->| Reset rls |       |                       |
 |    +--------+          |   +-----------+       |                       |
 |    |        |          |         :             |                       |
 |    |  ROM   |          |         :             |                       |
 |    |services|          |         :             |                       |
 |    |        |          |   +-------------+     |                       |
 |    |        |          |   |  *R5 ROM*   |     |                       |
 |    |        |          |   +-------------+     |                       |
 |    |        |<---------|---|Load and auth|     |                       |
 |    |        |          |   | tiboot3.bin |     |                       |
 |    +--------+          |   +-------------+     |                       |
 |    |        |<---------|---| Load sysfw  |     |                       |
 |    |        |          |   | part to TIFS|     |                       |
 |    |        |          |   | core        |     |                       |
 |    |        |          |   +-------------+     |                       |
 |    |        |          |         :             |                       |
 |    |        |          |         :             |                       |
 |    |        |          |         :             |                       |
 |    |        |          |   +-------------+     |                       |
 |    |        |          |   |  *R5 SPL*   |     |                       |
 |    |        |          |   +-------------+     |                       |
 |    |        |          |   |    DDR      |     |                       |
 |    |        |          |   |   config    |     |                       |
 |    |        |          |   +-------------+     |                       |
 |    |        |          |   |    Load     |     |                       |
 |    |        |          |   |  tispl.bin  |     |                       |
 |    |        |          |   +-------------+     |                       |
 |    |        |          |   |   Load R5   |     |                       |
 |    |        |          |   |   firmware  |     |                       |
 |    |        |          |   +-------------+     |                       |
 |    |        |<---------|---| Start A53   |     |                       |
 |    |        |          |   | and jump to |     |                       |
 |    |        |          |   | DM fw image |     |                       |
 |    |        |          |   +-------------+     |                       |
 |    |        |          |                       |     +-----------+     |
 |    |        |----------|-----------------------|---->| Reset rls |     |
 |    |        |          |                       |     +-----------+     |
 |    |  TIFS  |          |                       |          :            |
 |    |Services|          |                       |     +-----------+     |
 |    |        |<---------|-----------------------|---->|*ATF/OPTEE*|     |
 |    |        |          |                       |     +-----------+     |
 |    |        |          |                       |          :            |
 |    |        |          |                       |     +-----------+     |
 |    |        |<---------|-----------------------|---->| *A53 SPL* |     |
 |    |        |          |                       |     +-----------+     |
 |    |        |          |                       |     |   Load    |     |
 |    |        |          |                       |     | u-boot.img|     |
 |    |        |          |                       |     +-----------+     |
 |    |        |          |                       |          :            |
 |    |        |          |                       |     +-----------+     |
 |    |        |<---------|-----------------------|---->| *U-Boot*  |     |
 |    |        |          |                       |     +-----------+     |
 |    |        |          |                       |     |  prompt   |     |
 |    |        |----------|-----------------------|-----+-----------+-----|
 |    +--------+          |                       |                       |
 |                        |                       |                       |
 +------------------------------------------------------------------------+

- Here TIFS acts as master and provides all the critical services. R5/A53
  requests TIFS to get these services done as shown in the above diagram.

Sources:
--------
1. SYSFW:
	Tree: git://git.ti.com/k3-image-gen/k3-image-gen.git
	Branch: master

2. ATF:
	Tree: https://github.com/ARM-software/arm-trusted-firmware.git
	Branch: master

3. OPTEE:
	Tree: https://github.com/OP-TEE/optee_os.git
	Branch: master

4. U-Boot:
	Tree: https://source.denx.de/u-boot/u-boot
	Branch: master

5. TI Linux Firmware:
	Tree: git://git.ti.com/processor-firmware/ti-linux-firmware.git
	Branch: ti-linux-firmware

Build procedure:
----------------
1. ATF:

.. code-block:: text

 $ make CROSS_COMPILE=aarch64-none-linux-gnu- ARCH=aarch64 PLAT=k3 TARGET_BOARD=lite SPD=opteed

2. OPTEE:

.. code-block:: text

 $ make PLATFORM=k3 CFG_ARM64_core=y CROSS_COMPILE=arm-none-linux-gnueabihf- CROSS_COMPILE64=aarch64-none-linux-gnu-

3. U-Boot:

* 3.1 R5:

.. code-block:: text

 $ make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- am62x_evm_r5_defconfig O=/tmp/r5
 $ make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- O=/tmp/r5
 $ cd <k3-image-gen>
 $ make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- SOC=am62x SBL=/tmp/r5/spl/u-boot-spl.bin SYSFW_PATH=<path to ti-linux-firmware>/ti-sysfw/ti-fs-firmware-am62x-gp.bin

Use the tiboot3.bin generated from last command

* 3.2 A53:

.. code-block:: text

 $ make ARCH=arm CROSS_COMPILE=aarch64-none-linux-gnu- am62x_evm_a53_defconfig O=/tmp/a53
 $ make ARCH=arm CROSS_COMPILE=aarch64-none-linux-gnu- ATF=<path to ATF dir>/build/k3/lite/release/bl31.bin TEE=<path to OPTEE OS dir>/out/arm-plat-k3/core/tee-pager_v2.bin DM=<path to ti-linux-firmware>/ti-dm/am62xx/ipc_echo_testb_mcu1_0_release_strip.xer5f O=/tmp/a53

Target Images
--------------
Copy the below images to an SD card and boot:
 - tiboot3.bin from step 3.1
 - tispl.bin, u-boot.img from 3.2

Image formats:
--------------

- tiboot3.bin:

.. code-block:: text

                +-----------------------+
                |        X.509          |
                |      Certificate      |
                | +-------------------+ |
                | |                   | |
                | |        R5         | |
                | |   u-boot-spl.bin  | |
                | |                   | |
                | +-------------------+ |
                | |                   | |
                | |TIFS with board cfg| |
                | |                   | |
                | +-------------------+ |
                | |                   | |
                | |                   | |
                | |     FIT header    | |
                | | +---------------+ | |
                | | |               | | |
                | | |   DTB 1...N   | | |
                | | +---------------+ | |
                | +-------------------+ |
                +-----------------------+

- tispl.bin

.. code-block:: text

                +-----------------------+
                |                       |
                |       FIT HEADER      |
                | +-------------------+ |
                | |                   | |
                | |      A53 ATF      | |
                | +-------------------+ |
                | |                   | |
                | |     A53 OPTEE     | |
                | +-------------------+ |
                | |                   | |
                | |      R5 DM FW     | |
                | +-------------------+ |
                | |                   | |
                | |      A53 SPL      | |
                | +-------------------+ |
                | |                   | |
                | |   SPL DTB 1...N   | |
                | +-------------------+ |
                +-----------------------+
