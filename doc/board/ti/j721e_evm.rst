.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Lokesh Vutla <lokeshvutla@ti.com>

Texas Instruments K3 Platforms
==============================

Introduction:
-------------
The J721e family of SoCs are part of K3 Multicore SoC architecture platform
targeting automotive applications. They are designed as a low power, high
performance and highly integrated device architecture, adding significant
enhancement on processing power, graphics capability, video and imaging
processing, virtualization and coherent memory support.

The device is partitioned into three functional domains, each containing
specific processing cores and peripherals:

1. Wake-up (WKUP) domain:
        * Device Management and Security Controller (DMSC)

2. Microcontroller (MCU) domain:
        * Dual Core ARM Cortex-R5F processor

3. MAIN domain:
        * Dual core 64-bit ARM Cortex-A72
        * 2 x Dual cortex ARM Cortex-R5 subsystem
        * 2 x C66x Digital signal processor sub system
        * C71x Digital signal processor sub-system with MMA.

More info can be found in TRM: http://www.ti.com/lit/pdf/spruil1

Boot Flow:
----------
Boot flow is similar to that of AM65x SoC and extending it with remoteproc
support. Below is the pictorial representation of boot flow:

.. code-block:: text

 +------------------------------------------------------------------------+-----------------------+
 |        DMSC            |      MCU R5           |        A72            |  MAIN R5/C66x/C7x     |
 +------------------------------------------------------------------------+-----------------------+
 |    +--------+          |                       |                       |                       |
 |    |  Reset |          |                       |                       |                       |
 |    +--------+          |                       |                       |                       |
 |         :              |                       |                       |                       |
 |    +--------+          |   +-----------+       |                       |                       |
 |    | *ROM*  |----------|-->| Reset rls |       |                       |                       |
 |    +--------+          |   +-----------+       |                       |                       |
 |    |        |          |         :             |                       |                       |
 |    |  ROM   |          |         :             |                       |                       |
 |    |services|          |         :             |                       |                       |
 |    |        |          |   +-------------+     |                       |                       |
 |    |        |          |   |  *R5 ROM*   |     |                       |                       |
 |    |        |          |   +-------------+     |                       |                       |
 |    |        |<---------|---|Load and auth|     |                       |                       |
 |    |        |          |   | tiboot3.bin |     |                       |                       |
 |    |        |          |   +-------------+     |                       |                       |
 |    |        |          |         :             |                       |                       |
 |    |        |          |         :             |                       |                       |
 |    |        |          |         :             |                       |                       |
 |    |        |          |   +-------------+     |                       |                       |
 |    |        |          |   |  *R5 SPL*   |     |                       |                       |
 |    |        |          |   +-------------+     |                       |                       |
 |    |        |          |   |    Load     |     |                       |                       |
 |    |        |          |   |  sysfw.itb  |     |                       |                       |
 |    | Start  |          |   +-------------+     |                       |                       |
 |    | System |<---------|---|    Start    |     |                       |                       |
 |    |Firmware|          |   |    SYSFW    |     |                       |                       |
 |    +--------+          |   +-------------+     |                       |                       |
 |        :               |   |             |     |                       |                       |
 |    +---------+         |   |   Load      |     |                       |                       |
 |    | *SYSFW* |         |   |   system    |     |                       |                       |
 |    +---------+         |   | Config data |     |                       |                       |
 |    |         |<--------|---|             |     |                       |                       |
 |    |         |         |   +-------------+     |                       |                       |
 |    |         |         |   |    DDR      |     |                       |                       |
 |    |         |         |   |   config    |     |                       |                       |
 |    |         |         |   +-------------+     |                       |                       |
 |    |         |         |   |    Load     |     |                       |                       |
 |    |         |         |   |  tispl.bin  |     |                       |                       |
 |    |         |         |   +-------------+     |                       |                       |
 |    |         |         |   |   Load R5   |     |                       |                       |
 |    |         |         |   |   firmware  |     |                       |                       |
 |    |         |         |   +-------------+     |                       |                       |
 |    |         |<--------|---| Start A72   |     |                       |                       |
 |    |         |         |   | and jump to |     |                       |                       |
 |    |         |         |   | DM fw image |     |                       |                       |
 |    |         |         |   +-------------+     |                       |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |---------|-----------------------|---->| Reset rls |     |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |  TIFS   |         |                       |          :            |                       |
 |    |Services |         |                       |     +-----------+     |                       |
 |    |         |<--------|-----------------------|---->|*ATF/OPTEE*|     |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |         |                       |          :            |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |<--------|-----------------------|---->| *A72 SPL* |     |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |         |                       |     |   Load    |     |                       |
 |    |         |         |                       |     | u-boot.img|     |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |         |                       |          :            |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |<--------|-----------------------|---->| *U-Boot*  |     |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |         |                       |     |  prompt   |     |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |         |                       |     |  Load R5  |     |                       |
 |    |         |         |                       |     |  Firmware |     |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |<--------|-----------------------|-----|  Start R5 |     |      +-----------+    |
 |    |         |---------|-----------------------|-----+-----------+-----|----->| R5 starts |    |
 |    |         |         |                       |     |  Load C6  |     |      +-----------+    |
 |    |         |         |                       |     |  Firmware |     |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |<--------|-----------------------|-----|  Start C6 |     |      +-----------+    |
 |    |         |---------|-----------------------|-----+-----------+-----|----->| C6 starts |    |
 |    |         |         |                       |     |  Load C7  |     |      +-----------+    |
 |    |         |         |                       |     |  Firmware |     |                       |
 |    |         |         |                       |     +-----------+     |                       |
 |    |         |<--------|-----------------------|-----|  Start C7 |     |      +-----------+    |
 |    |         |---------|-----------------------|-----+-----------+-----|----->| C7 starts |    |
 |    +---------+         |                       |                       |      +-----------+    |
 |                        |                       |                       |                       |
 +------------------------------------------------------------------------+-----------------------+

- Here DMSC acts as master and provides all the critical services. R5/A72
  requests DMSC to get these services done as shown in the above diagram.

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

4. DM Firmware:
	Tree: git://git.ti.com/processor-firmware/ti-linux-firmware.git
	Branch: ti-linux-firmware

5. U-Boot:
	Tree: https://source.denx.de/u-boot/u-boot
	Branch: master

Build procedure:
----------------
1. SYSFW:

.. code-block:: bash

    make CROSS_COMPILE=arm-linux-gnueabihf- SOC=j721e

2. ATF:

.. code-block:: bash

    make CROSS_COMPILE=aarch64-linux-gnu- ARCH=aarch64 PLAT=k3 TARGET_BOARD=generic SPD=opteed

3. OPTEE:

.. code-block:: bash

    make PLATFORM=k3-j721e CFG_ARM64_core=y

4. U-Boot:

* 4.1 R5:

.. code-block:: bash

    make CROSS_COMPILE=arm-linux-gnueabihf- j721e_evm_r5_defconfig O=build/r5
    make CROSS_COMPILE=arm-linux-gnueabihf- O=build/r5

* 4.2 A72:

.. code-block:: bash

    make CROSS_COMPILE=aarch64-linux-gnu- j721e_evm_a72_defconfig O=build/a72
    make CROSS_COMPILE=aarch64-linux-gnu- ATF=<ATF dir>/build/k3/generic/release/bl31.bin TEE=<OPTEE OS dir>/out/arm-plat-k3/core/tee-pager_v2.bin DM=<DM firmware>/ti-dm/j721e/ipc_echo_testb_mcu1_0_release_strip.xer5f O=build/a72

Target Images
--------------
Copy the below images to an SD card and boot:
 - sysfw.itb from step 1
 - tiboot3.bin from step 4.1
 - tispl.bin, u-boot.img from 4.2

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
                | |      A72 ATF      | |
                | +-------------------+ |
                | |                   | |
                | |     A72 OPTEE     | |
                | +-------------------+ |
                | |                   | |
                | |      R5 DM FW     | |
                | +-------------------+ |
                | |                   | |
                | |      A72 SPL      | |
                | +-------------------+ |
                | |                   | |
                | |   SPL DTB 1...N   | |
                | +-------------------+ |
                +-----------------------+

- sysfw.itb

.. code-block:: text

                +-----------------------+
                |                       |
                |       FIT HEADER      |
                | +-------------------+ |
                | |                   | |
                | |     sysfw.bin     | |
                | +-------------------+ |
                | |                   | |
                | |    board config   | |
                | +-------------------+ |
                | |                   | |
                | |     PM config     | |
                | +-------------------+ |
                | |                   | |
                | |     RM config     | |
                | +-------------------+ |
                | |                   | |
                | |    Secure config  | |
                | +-------------------+ |
                +-----------------------+

OSPI:
-----
ROM supports booting from OSPI from offset 0x0.

Flashing images to OSPI:

Below commands can be used to download tiboot3.bin, tispl.bin, u-boot.img,
and sysfw.itb over tftp and then flash those to OSPI at their respective
addresses.

.. code-block:: text

 => sf probe
 => tftp ${loadaddr} tiboot3.bin
 => sf update $loadaddr 0x0 $filesize
 => tftp ${loadaddr} tispl.bin
 => sf update $loadaddr 0x80000 $filesize
 => tftp ${loadaddr} u-boot.img
 => sf update $loadaddr 0x280000 $filesize
 => tftp ${loadaddr} sysfw.itb
 => sf update $loadaddr 0x6C0000 $filesize

Flash layout for OSPI:

.. code-block:: text

         0x0 +----------------------------+
             |     ospi.tiboot3(512K)     |
             |                            |
     0x80000 +----------------------------+
             |     ospi.tispl(2M)         |
             |                            |
    0x280000 +----------------------------+
             |     ospi.u-boot(4M)        |
             |                            |
    0x680000 +----------------------------+
             |     ospi.env(128K)         |
             |                            |
    0x6A0000 +----------------------------+
	     |	 ospi.env.backup (128K)   |
	     |                            |
    0x6C0000 +----------------------------+
             |      ospi.sysfw(1M)        |
             |                            |
    0x7C0000 +----------------------------+
	     |      padding (256k)        |
    0x800000 +----------------------------+
             |     ospi.rootfs(UBIFS)     |
             |                            |
             +----------------------------+

Firmwares:
----------

The J721e u-boot allows firmware to be loaded for the Cortex-R5 subsystem.
The CPSW5G in J7200 and CPSW9G in J721E present in MAIN domain is configured
and controlled by the ethernet firmware that executes in the MAIN Cortex R5.
The default supported environment variables support loading these firmwares
from only MMC. "dorprocboot" env variable has to be set for the U-BOOT to load
and start the remote cores in the system.

J721E common processor board can be attached to a Ethernet QSGMII card and the
PHY in the card has to be reset before it can be used for data transfer.
"do_main_cpsw0_qsgmii_phyinit" env variable has to be set for the U-BOOT to
configure this PHY.
