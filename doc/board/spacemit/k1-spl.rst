.. SPDX-License-Identifier: GPL-2.0-or-later

SpacemiT K1 SPL Build and Test Guide
=====================================

This guide explains how to build and test U-Boot SPL on SpacemiT K1 based
boards. It covers building SPL with DDR initialization, generating the signed
FSBL image, and deploying via USB fastboot.

Tested boards: Banana Pi BPI-F3, MusePi Pro.

.. note::

   The procedure described here loads SPL into SRAM via USB fastboot and
   does not modify the on-board flash, so a power cycle restores the
   board's normal boot path.

   At this stage of the patchset, SPL initializes DDR and then halts with
   "SPL: Unsupported Boot Device!" because SPI NOR storage support is not
   yet available in U-Boot proper. This document will be updated when
   U-Boot stage support is ready.

Prerequisites
~~~~~~~~~~~~~

- A SpacemiT K1 board with USB Type-C and UART access
- USB-to-UART adapter (3.3V TTL)
- ``minicom`` or equivalent serial terminal, configured at 115200 8N1
- ``fastboot`` tool on the host

Hardware Setup
~~~~~~~~~~~~~~

**1. UART Connection**

Remove all other cables first, then attach UART. Connect a 3.3V
USB-to-UART cable to the board's UART header (J25 on the BPI-F3)::

    BPI-F3 top view
    +--------------------------------------------------+
    |                                                  |
    |   J15: USB-C [====]     [FDL] [PWR] [RST]        |
    |                                                  |
    |                                                  |
    |                              J25 (UART header)   |
    |                              [TXD] [RXD] [GND]   |
    +--------------------------------------------------+

After UART is connected, attach the USB Type-C cable to the OTG port
(J15 on BPI-F3) to power on.

**2. Serial Console**

.. code-block:: console

   $ minicom -D /dev/ttyUSB0

Default baudrate: 115200.

Building U-Boot SPL
~~~~~~~~~~~~~~~~~~~~

**1. Obtain the DDR training firmware**

The DDR training firmware is a proprietary binary provided by SpacemiT. It is
not included in U-Boot and must be downloaded separately from:

https://github.com/spacemit-com/spacemit-firmware/tree/master/k1/v0.2

Download ``ddr_fw.bin`` from that directory.

This binary is integrated into the SPL image at build time via the binman
framework. When the SPL image is loaded to SRAM (e.g., via USB fastboot),
the SPL executes the DDR firmware from SRAM to perform DDR initialization.

**2. Obtain OpenSBI fw_dynamic.bin**

Any pre-built ``fw_dynamic.bin`` is sufficient. Upstream OpenSBI
sources are at https://github.com/riscv-software-src/opensbi if you
need to build one.

At this stage of the patchset, SPL halts before invoking OpenSBI,
but U-Boot's binman still packages ``fw_dynamic.bin`` into
``u-boot.itb`` and the build fails if it is missing.

**3. Build SPL**

.. code-block:: console

   $ export CROSS_COMPILE=riscv64-linux-gnu-
   $ export ARCH=riscv
   $ export DDR_FW_FILE=$(pwd)/ddr_fw.bin
   $ export OPENSBI=/path/to/fw_dynamic.bin
   $ make spacemit_k1_defconfig
   $ make

Output: ``u-boot-spl-ddr.bin`` in the build directory. This image contains the
SPL code and the DDR firmware blob packaged together via binman.

.. note::

   If ``DDR_FW_FILE`` is not set, the build completes with an empty
   placeholder. The resulting SPL will boot but cannot initialize DDR.

**4. Generate signed FSBL image**

The K1 BootROM requires a signed first-stage bootloader (FSBL). The signing
tool (``tools/build_binary_file.py``) is in SpacemiT's vendor U-Boot repository:

.. code-block:: console

   $ git clone -b k1-bl-v2.2.y https://gitee.com/spacemit-buildroot/uboot-2022.10

The script uses ``fsbl_ddr.json`` which may not exist by default. If
``fsbl_ddr.json`` does not exist in
``uboot-2022.10/board/spacemit/k1-x/configs/``,
create it by copying ``fsbl.json`` and replacing the reference to
``u-boot-spl.bin`` with ``u-boot-spl-ddr.bin``:

.. code-block:: console

   $ cd uboot-2022.10/board/spacemit/k1-x/configs
   $ cp fsbl.json fsbl_ddr.json
   $ sed -i 's/u-boot-spl\.bin/u-boot-spl-ddr.bin/g' fsbl_ddr.json

Create the ``fsbl.sh`` script below in the ``uboot-2022.10`` directory.
Update the path variables to match your local setup:

.. code-block:: bash

   #!/bin/sh
   MAINLINE_UBOOT_IMG_PATH="{your path}/u-boot"
   MAINLINE_SPL_IMG_PATH="{your path}/u-boot/spl"
   FSBL_PATH="{your path}/uboot-2022.10/spl_bin"
   KEY_TOOL_PATH="{your path}/uboot-2022.10/tools"
   CONFIG_PATH="{your path}/uboot-2022.10/board/spacemit/k1-x/configs"

   mkdir -p ${FSBL_PATH}
   echo "Clean binaries in ${FSBL_PATH}"
   rm -f ${FSBL_PATH}/u-boot-spl-ddr.bin
   rm -f ${FSBL_PATH}/u-boot-spl.bin

   if [ ! -d ${MAINLINE_SPL_IMG_PATH} ]; then
           MAINLINE_UBOOT_IMG_PATH="{your path}/build"
           MAINLINE_SPL_IMG_PATH="{your path}/build/spl"
   fi

   # The signing tool reads u-boot-spl-ddr.bin from the path declared in
   # fsbl_ddr.json's "source" field, which is resolved relative to the
   # JSON file's directory -- i.e. one level up from CONFIG_PATH, not
   # FSBL_PATH. Stage the unsigned binary there before signing.
   cp ${MAINLINE_UBOOT_IMG_PATH}/u-boot-spl-ddr.bin ${CONFIG_PATH}/../
   python3 ${KEY_TOOL_PATH}/build_binary_file.py \
       -c ${CONFIG_PATH}/fsbl_ddr.json \
       -o ${FSBL_PATH}/FSBL.bin

Then run:

.. code-block:: console

   $ chmod +x fsbl.sh
   $ ./fsbl.sh

Output: ``FSBL.bin`` in the ``spl_bin`` directory, ready for deployment.

Deploying via USB Fastboot
~~~~~~~~~~~~~~~~~~~~~~~~~~~

To enter BootROM fastboot mode:

1. Power off the board by unplugging its power supply.
2. **Press and hold** the FDL button (called "Boot Key" on some boards;
   see the board layout above for the BPI-F3).
3. While holding the button, use a USB cable to connect the OTG port to
   your host. This cable is also used by fastboot to upload the firmware.
4. Release the button.

On the host, ``fastboot devices`` should list the board::

    dfu-device     DFU download

The serial console shows the BootROM's USB download handler trace,
including a line like::

    usb2d_initialize : enter

This indicates the board is ready to accept an image via USB.

.. tip::

   If you are worried about insufficient USB power, you can first plug
   in the power, then release the button, and then plug in the USB
   cable.

On the host:

.. code-block:: console

   $ sudo fastboot stage FSBL.bin
   $ sudo fastboot continue

Expected Output
~~~~~~~~~~~~~~~~

During successful SPL boot with DDR initialization, the serial console
shows output like::

    <debug_uart>

    U-Boot SPL 2026.04-rc4-00430-g8b84b1ed8ea9 (May 08 2026 - 09:08:27 -0400)
    Fail to detect board:-19
    vdd_core, value:900000
    vdd_1v8, value:1800000
    vdd_1v8_mmc, value:1800000
    DDR firmware: [0xc0815840]:0xf0227179, size:0x8d98
    DDR is ready
    SPL: Unsupported Boot Device!
    SPL: failed to boot from all boot devices
    ### ERROR ### Please RESET the board ###

To walk through the key lines:

- ``DDR firmware: [...], size:0x8d98`` - DDR firmware loaded successfully
- ``DDR is ready`` - DDR initialization completed
- ``SPL: failed to boot from all boot devices`` - expected at this stage,
  confirms that SPL with DDR init is working correctly

If SPL hangs before printing DDR messages, verify that ``DDR_FW_FILE`` was set
during build and that ``ddr_fw.bin`` is not empty.

.. tip::

   To see verbose DDR training output from the DDR firmware (per-phase
   training pass/fail logs), add ``#define DEBUG`` at the top of
   ``board/spacemit/k1/spl.c`` and rebuild. SPL then passes ``puts`` as
   the firmware's log callback; otherwise the callback is ``NULL`` and
   the firmware runs silently.

References
~~~~~~~~~~~

- `DDR firmware repository <https://github.com/spacemit-com/spacemit-firmware>`_
- `SpacemiT vendor U-Boot (signing tool) <https://gitee.com/spacemit-buildroot/uboot-2022.10>`_
