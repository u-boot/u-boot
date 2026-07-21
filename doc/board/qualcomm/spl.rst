.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Michael Srba <Michael.Srba@seznam.cz>

===================================
Booting U-Boot SPL on Qualcomm SoCs
===================================

Overview
--------
The boot process on sdm845 (and some other Qualcomm SoCs) starts with the bootrom
of the Application Processor, which executes XBL_SEC, which jumps to "OEM" code
in EL1. Production devices are typically "fused", with a hash of the OEM's signing
key burnt into one of the "QFUSE" banks on the SoC making it impossible to run
custom bootloader code. As a result U-Boot SPL is only supported on unfused
("secureboot off") devices. XBL_SEC is always signed by Qualcomm, and the fuses
to disable turning off signature verification for it are always burnt at the
factory, so replacing XBL_SEC is impossible without using JTAG. Of course JTAG
is typically disabled on devices that have secure boot enabled, or at minimum
greatly neutered.

U-Boot SPL for Qualcomm platforms uses a custom linker script (per SoC) to build a bootable ELF.
For sdm845 (and some other platforms) this has two sections, u-boot code and an embedded
xbl_sec elf (signed by Qualcomm). To boot on an unfused SoC, the elf additionally
needs to have hash sections added, which can be accomplished with qtestsign.

Currently, sdm845 is supported. You need a device with secure boot disabled
(or with secure boot enabled if you enabled it yourself and have the private key,
though for full security you'd also want to disable JTAG which will remove your ability
to mess with the control flow in the bootrom (immutable) and in XBL_SEC (signed)).

Building
--------
First, obtain an xbl_sec that includes the EL3 privilege escalation feature
and place it at .output/xbl_sec.elf. You can extract it from an xbl elf.
If you're unable to find one, you can also use JTAG/SWD to break at the SMC
entry and use gdb to jump to the u-boot entry point in EL3.

To build a bootable image, you need to use a defconfig specific to your SoC.
This is because the ELF has to specify where in the address space to put u-boot SPL,
and this may differ per SoC. There may be other SoC-dependent build time choices,
though in principle those could be made at runtime.

First run ``make sdm845_spl_defconfig``::

	make CROSS_COMPILE=aarch64-suse-linux- O=.output DEVICE_TREE=qcom/sdm845-shift-axolotl sdm845_spl_defconfig

Then compile u-boot and specify the dts for your board (technically nothing about the resulting
SPL image should be board-specific, but there are no non-board-specific device trees in Linux)::

	make CROSS_COMPILE=aarch64-suse-linux- O=.output DEVICE_TREE=qcom/sdm845-shift-axolotl

Finally, use ``qtestsign`` to add the hash segments required by PBL::

	qtestsign -v 5 -o .output/spl/u-boot-spl_signed.elf prog .output/spl/u-boot-spl.elf

Running
-------
Currently, U-Boot SPL for Qualcomm platforms expects to be booted via EDL::

	edl.py --loader=$PWD/.output/spl/u-boot-spl_signed.elf

SPL will then launch the DFU gadget and wait for you to upload u-boot proper::

	dfu-util -RD .output/u-boot.img

u-boot proper will then likely crash, since SPL currently doesn't init DRAM on Qualcomm platforms
and u-boot proper currently doesn't support running from SRAM. The latter should be an easy fix.

Notes on memory map
-------------------
| There are various banks of SRAM on a Qualcomm SoC that we can use prior to DRAM init.
| For example:
| msm8916 - 512K L2-as-TCM (at ``0x08000000``), 16K OCIMEM (at ``0x08600000``)
| msm8998 - 1M L2-as-TCM (at ``0x14000000``), 256K OCIMEM (at ``0x14680000``)
| sdm845 - 1.5M BOOT_IMEM (at ``0x14800000``), 256K OCIMEM (at ``0x14680000``)

There's also RPM code/data RAM and hexagon TCMs, but unless we want to boot dram-less Linux
we can probably safely ignore those. On msm8916 they may come in handy though.

sdm845 can also have 8M LLCC-as-TCM in theory, but this appears to be broken.
L2-as-TCM is no longer present.

Since a limited amount of not necessarily continuous SRAM is available, we need to manually
specify where .text, .bss, the malloc pool and the stack go. The Kconfig contains reasonable
defaults per SoC.

On sdm845, we by default put U-Boot SPL in BOOT_IMEM, with .bss, malloc pool and the stack
filling OCIMEM. We can also fit U-Boot proper in BOOT_IMEM, for dram-less DFU or peek/poke
with a shell. To that end, we set ``CONFIG_TEXT_BASE`` at 512K into BOOT_IMEM, and set
``CONFIG_SPL_MAX_SIZE`` to 512K - 64. We also configure dfu to load U-Boot proper
to ``CONFIG_TEXT_BASE`` - 64. (64 bytes is the size of u-boot legacy header)
