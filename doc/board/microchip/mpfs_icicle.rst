.. SPDX-License-Identifier: GPL-2.0+

Microchip PolarFire SoC Icicle Kit
==================================

RISC-V PolarFire SoC
--------------------

The PolarFire SoC is the 4+1 64-bit RISC-V SoC from Microchip.

The Icicle Kit development platform is based on PolarFire SoC and capable
of running Linux.

Mainline support
----------------

The support for following drivers are already enabled:

1. NS16550 UART Driver.
2. Microchip Clock Driver.
3. Cadence MACB ethernet driver for networking support.
4. Cadence MMC Driver for eMMC/SD support.
5. Microchip I2C Driver.

.. include:: mpfs_build_boot.rst

Microchip boot-flow
~~~~~~~~~~~~~~~~~~~

HSS with OpenSBI (M-Mode) -> U-Boot (S-Mode) -> Linux (S-Mode)

Build the HSS (Hart Software Services) - Microchip boot-flow
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

(Note: HSS git repo is at https://github.com/polarfire-soc/hart-software-services)

1. Configure

.. code-block:: none

   make BOARD=mpfs-icicle-kit-es config

Alternatively, copy the default config for Microchip boot-flow.

.. code-block:: none

   cp boards/mpfs-icicle-kit-es/def_config .config

2. make BOARD=mpfs-icicle-kit-es
3. In the Default subdirectory, the standard build will create hss.elf and
   various binary formats (hss.hex and hss.bin).

The FPGA design will use the hss.hex or hss.bin.

.. include:: mpfs_design_hss.rst

Custom boot-flow
~~~~~~~~~~~~~~~~

HSS without OpenSBI (M-Mode) -> OpenSBI (M-Mode) -> U-Boot (S-Mode) -> Linux (S-Mode)

Build OpenSBI
'''''''''''''

1. Get the OpenSBI source

.. code-block:: none

   git clone https://github.com/riscv/opensbi.git
   cd opensbi

2. Build

.. code-block:: none

   make PLATFORM=generic FW_PAYLOAD_PATH=<u-boot-directory>/u-boot.bin
   FW_FDT_PATH=<u-boot-directory>/dts/upstream/src/riscv/microchip/mpfs-icicle-kit-.dtb

3. Output "fw_payload.bin" file available at
   "<opensbi-directory>/build/platform/generic/firmware/fw_payload.bin"

Build the HSS (Hart Software Services)- Custom boot-flow
''''''''''''''''''''''''''''''''''''''''''''''''''''''''

(Note: HSS git repo is at https://github.com/polarfire-soc/hart-software-services)

1. Configure

.. code-block:: none

   make BOARD=mpfs-icicle-kit-es config

Alternatively, copy the default custom config for Custom boot-flow.

.. code-block:: none

   cp boards/mpfs-icicle-kit-es/def_config_custom .config

2. make BOARD=mpfs-icicle-kit-es
3. In the Default subdirectory, the standard build will create hss.elf and
   various binary formats (hss.hex and hss.bin).

The FPGA design will use the hss.hex or hss.bin.

.. include:: mpfs_common.rst
