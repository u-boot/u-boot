.. SPDX-License-Identifier: GPL-2.0+

Booting from eMMC using HSS
---------------------------

Building U-Boot
~~~~~~~~~~~~~~~

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

   export CROSS_COMPILE=<riscv64 toolchain prefix>

3. make microchip_mpfs_generic_defconfig
4. make

Flashing
~~~~~~~~

The current U-Boot port is supported in S-mode only and loaded from DRAM.

A prior stage M-mode firmware/bootloader (e.g HSS with OpenSBI) is required to
boot the u-boot.bin in S-mode.

Currently, the u-boot.bin is used as a payload of the HSS firmware (Microchip
boot-flow) and OpenSBI generic platform fw_payload.bin (with u-boot.bin embedded)
as HSS payload (Custom boot-flow)
