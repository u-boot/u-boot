.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

Coreboot
========

Build Instructions for U-Boot as coreboot payload
-------------------------------------------------
Building U-Boot as a coreboot payload is just like building U-Boot for targets
on other architectures, like below::

   $ make coreboot_defconfig
   $ make all

Test with coreboot
------------------
For testing U-Boot as the coreboot payload, there are things that need be paid
attention to. coreboot supports loading an ELF executable and a 32-bit plain
binary, as well as other supported payloads. With the default configuration,
U-Boot is set up to use a separate Device Tree Blob (dtb). As of today, the
generated u-boot-dtb.bin needs to be packaged by the cbfstool utility (a tool
provided by coreboot) manually as coreboot's 'make menuconfig' does not provide
this capability yet. The command is as follows::

   # in the coreboot root directory
   $ ./build/util/cbfstool/cbfstool build/coreboot.rom add-flat-binary \
     -f u-boot-dtb.bin -n fallback/payload -c lzma -l 0x1110000 -e 0x1110000

Make sure 0x1110000 matches CONFIG_SYS_TEXT_BASE, which is the symbol address
of _x86boot_start (in arch/x86/cpu/start.S).

If you want to use ELF as the coreboot payload, change U-Boot configuration to
use CONFIG_OF_EMBED instead of CONFIG_OF_SEPARATE.

To enable video you must enable these options in coreboot:

   - Set framebuffer graphics resolution (1280x1024 32k-color (1:5:5))
   - Keep VESA framebuffer

At present it seems that for Minnowboard Max, coreboot does not pass through
the video information correctly (it always says the resolution is 0x0). This
works correctly for link though.
