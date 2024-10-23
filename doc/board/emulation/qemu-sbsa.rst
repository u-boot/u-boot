.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2024, Patrick Rudolph <patrick.rudolph@9elements.com>

QEMU ARM SBSA
=============

QEMU for ARM supports Arm Server Base System Architecture Reference board,
short 'sbsa-ref' that utilizes ACPI over FDT. This document describes how to run
U-Boot under it. Only AArch64 is supported.

The 'sbsa' platform provides the following as the basic functionality:

    - A freely configurable amount of CPU cores
    - U-Boot loaded and executing in the emulated flash at address 0x10000000
    - No device tree blob
    - A freely configurable amount of RAM
    - A PL011 serial port
    - An ARMv7/ARMv8 architected timer
    - PSCI for rebooting the system
    - A generic ECAM-based PCI host controller

Additionally, a number of optional peripherals can be added to the PCI bus.

Compile ARM Trusted Firmware (ATF)
----------------------------------

Get and Build the ARM Trusted firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://github.com/ARM-software/arm-trusted-firmware

.. code-block:: bash

  git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git tfa
  cd tfa
  make CROSS_COMPILE=aarch64-linux-gnu- all fip \
    ARM_LINUX_KERNEL_AS_BL33=1 DEBUG=1 PLAT=qemu_sbsa

Copy the resulting FIP and BL1 binary

.. code-block:: bash

  cp build/qemu_sbsa/debug/fip.bin ../
  cp build/qemu_sbsa/debug/bl1.bin ../

Building U-Boot
---------------
Set the CROSS_COMPILE environment variable as usual, and run:

.. code-block:: bash

    make qemu-arm-sbsa_defconfig
    make

Running U-Boot
--------------
The minimal QEMU command line to get U-Boot up and running is:

.. code-block:: bash

    qemu-system-aarch64 -machine sbsa-ref -nographic -cpu cortex-a57 \
                        -pflash secure-world.rom \
                        -pflash unsecure-world.rom

Note that for some odd reason qemu-system-aarch64 needs to be explicitly
told to use a 64-bit CPU or it will boot in 32-bit mode. The -nographic argument
ensures that output appears on the terminal. Use Ctrl-A X to quit.

Booting distros
---------------

It is possible to install and boot a standard Linux distribution using
sbsa by setting up a root disk::

.. code-block:: bash

    qemu-img create root.img 20G

then using the installer to install. For example, with Debian 12::

.. code-block:: bash

    qemu-system-aarch64 \
      -machine sbsa-ref -cpu cortex-a57 -m 4G -smp 4 \
      -pflash secure-world.rom \
      -pflash unsecure-world.rom \
      -device virtio-rng-pci \
      -device usb-kbd -device usb-tablet \
      -cdrom debian-12.0.0-arm64-netinst.iso \
      -hda root.img

Debug UART
----------

The debug UART on the ARM sbsa board uses these settings::

    CONFIG_DEBUG_UART=y
