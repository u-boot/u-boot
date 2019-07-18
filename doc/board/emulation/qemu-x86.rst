.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

QEMU x86
========

Build instructions for bare mode
--------------------------------

To build u-boot.rom for QEMU x86 targets, just simply run::

   $ make qemu-x86_defconfig (for 32-bit)
   $ make qemu-x86_64_defconfig (for 64-bit)
   $ make all

Note this default configuration will build a U-Boot for the QEMU x86 i440FX
board. To build a U-Boot against QEMU x86 Q35 board, you can change the build
configuration during the 'make menuconfig' process like below::

   Device Tree Control  --->
       ...
       (qemu-x86_q35) Default Device Tree for DT control

Test with QEMU for bare mode
----------------------------

QEMU is a fancy emulator that can enable us to test U-Boot without access to
a real x86 board. Please make sure your QEMU version is 2.3.0 or above test
U-Boot. To launch QEMU with u-boot.rom, call QEMU as follows::

   $ qemu-system-i386 -nographic -bios path/to/u-boot.rom

This will instantiate an emulated x86 board with i440FX and PIIX chipset. QEMU
also supports emulating an x86 board with Q35 and ICH9 based chipset, which is
also supported by U-Boot. To instantiate such a machine, call QEMU with::

   $ qemu-system-i386 -nographic -bios path/to/u-boot.rom -M q35

Note by default QEMU instantiated boards only have 128 MiB system memory. But
it is enough to have U-Boot boot and function correctly. You can increase the
system memory by pass '-m' parameter to QEMU if you want more memory::

   $ qemu-system-i386 -nographic -bios path/to/u-boot.rom -m 1024

This creates a board with 1 GiB system memory. Currently U-Boot for QEMU only
supports 3 GiB maximum system memory and reserves the last 1 GiB address space
for PCI device memory-mapped I/O and other stuff, so the maximum value of '-m'
would be 3072.

QEMU emulates a graphic card which U-Boot supports. Removing '-nographic' will
show QEMU's VGA console window. Note this will disable QEMU's serial output.
If you want to check both consoles, use '-serial stdio'.

Multicore is also supported by QEMU via '-smp n' where n is the number of cores
to instantiate. Note, the maximum supported CPU number in QEMU is 255.

The fw_cfg interface in QEMU also provides information about kernel data,
initrd, command-line arguments and more. U-Boot supports directly accessing
these informtion from fw_cfg interface, which saves the time of loading them
from hard disk or network again, through emulated devices. To use it , simply
providing them in QEMU command line::

   $ qemu-system-i386 -nographic -bios path/to/u-boot.rom -m 1024 \
     -kernel /path/to/bzImage -append 'root=/dev/ram console=ttyS0' \
     -initrd /path/to/initrd -smp 8

Note: -initrd and -smp are both optional

Then start QEMU, in U-Boot command line use the following U-Boot command to
setup kernel::

   => qfw
   qfw - QEMU firmware interface

   Usage:
   qfw <command>
       - list                             : print firmware(s) currently loaded
       - cpus                             : print online cpu number
       - load <kernel addr> <initrd addr> : load kernel and initrd (if any) and setup for zboot

   => qfw load
   loading kernel to address 01000000 size 5d9d30 initrd 04000000 size 1b1ab50

Here the kernel (bzImage) is loaded to 01000000 and initrd is to 04000000. Then,
'zboot' can be used to boot the kernel::

   => zboot 01000000 - 04000000 1b1ab50

To run 64-bit U-Boot, qemu-system-x86_64 should be used instead, e.g.::

   $ qemu-system-x86_64 -nographic -bios path/to/u-boot.rom

A specific CPU can be specified via the '-cpu' parameter but please make
sure the specified CPU supports 64-bit like '-cpu core2duo'. Conversely
'-cpu pentium' won't work for obvious reasons that the processor only
supports 32-bit.

Note 64-bit support is very preliminary at this point. Lots of features
are missing in the 64-bit world. One notable feature is the VGA console
support which is currently missing, so that you must specify '-nographic'
to get 64-bit U-Boot up and running.
