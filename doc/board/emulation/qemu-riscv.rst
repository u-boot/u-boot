.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>

QEMU RISC-V
===========

QEMU for RISC-V supports a special 'virt' machine designed for emulation and
virtualization purposes. This document describes how to run U-Boot under it.
Both 32-bit and 64-bit targets are supported, running in either machine or
supervisor mode.

The QEMU virt machine models a generic RISC-V virtual machine with support for
the VirtIO standard networking and block storage devices. It has CLINT, PLIC,
16550A UART devices in addition to VirtIO and it also uses device-tree to pass
configuration information to guest software. It implements RISC-V privileged
architecture spec v1.10.

Building U-Boot
---------------
Set the CROSS_COMPILE environment variable as usual, and run:

- For 32-bit RISC-V::

    make qemu-riscv32_defconfig
    make

- For 64-bit RISC-V::

    make qemu-riscv64_defconfig
    make

This will compile U-Boot for machine mode. To build supervisor mode binaries,
use the configurations qemu-riscv32_smode_defconfig and
qemu-riscv64_smode_defconfig instead. Note that U-Boot running in supervisor
mode requires a supervisor binary interface (SBI), such as RISC-V OpenSBI.

Running U-Boot
--------------
The minimal QEMU command line to get U-Boot up and running is:

- For 32-bit RISC-V::

    qemu-system-riscv32 -nographic -machine virt -bios u-boot

- For 64-bit RISC-V::

    qemu-system-riscv64 -nographic -machine virt -bios u-boot

The commands above create targets with 128MiB memory by default.
A freely configurable amount of RAM can be created via the '-m'
parameter. For example, '-m 2G' creates 2GiB memory for the target,
and the memory node in the embedded DTB created by QEMU reflects
the new setting.

For instructions on how to run U-Boot in supervisor mode on QEMU
with OpenSBI, see the documentation available with OpenSBI:
https://github.com/riscv/opensbi/blob/master/docs/platform/qemu_virt.md

These have been tested in QEMU 5.0.0.

Running U-Boot SPL
------------------
In the default SPL configuration, U-Boot SPL starts in machine mode. U-Boot
proper and OpenSBI (FW_DYNAMIC firmware) are bundled as FIT image and made
available to U-Boot SPL. Both are then loaded by U-Boot SPL and the location
of U-Boot proper is passed to OpenSBI. After initialization, U-Boot proper is
started in supervisor mode by OpenSBI.

OpenSBI must be compiled before compiling U-Boot. Version 0.4 and higher is
supported by U-Boot. Clone the OpenSBI repository and run the following command.

.. code-block:: console

    git clone https://github.com/riscv/opensbi.git
    cd opensbi
    make PLATFORM=generic

See the OpenSBI documentation for full details:
https://github.com/riscv/opensbi/blob/master/docs/platform/qemu_virt.md

To make the FW_DYNAMIC binary (build/platform/qemu/virt/firmware/fw_dynamic.bin)
available to U-Boot, either copy it into the U-Boot root directory or specify
its location with the OPENSBI environment variable. Afterwards, compile U-Boot
with the following commands.

- For 32-bit RISC-V::

    make qemu-riscv32_spl_defconfig
    make

- For 64-bit RISC-V::

    make qemu-riscv64_spl_defconfig
    make

The minimal QEMU commands to run U-Boot SPL in both 32-bit and 64-bit
configurations are:

- For 32-bit RISC-V::

    qemu-system-riscv32 -nographic -machine virt -bios spl/u-boot-spl \
    -device loader,file=u-boot.itb,addr=0x80200000

- For 64-bit RISC-V::

    qemu-system-riscv64 -nographic -machine virt -bios spl/u-boot-spl \
    -device loader,file=u-boot.itb,addr=0x80200000

An attached disk can be emulated by adding::

    -device ich9-ahci,id=ahci \
    -drive if=none,file=riscv64.img,format=raw,id=mydisk \
    -device ide-hd,drive=mydisk,bus=ahci.0

You will have to run 'scsi scan' to use it.
