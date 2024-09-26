.. SPDX-License-Identifier: GPL-2.0-or-later

Memory Management
-----------------

.. note::

  This information is outdated and needs to be updated.

U-Boot runs in system state and uses physical addresses, i.e. the
MMU is not used either for address mapping nor for memory protection.

The available memory is mapped to fixed addresses using the
memory-controller. In this process, a contiguous block is formed for each
memory type (Flash, SDRAM, SRAM), even when it consists of several
physical-memory banks.

U-Boot is installed in XIP flash memory, or may be loaded into a lower region of
RAM by a secondary program loader (SPL). After
booting and sizing and initialising DRAM, the code relocates itself
to the upper end of DRAM. Immediately below the U-Boot code some
memory is reserved for use by malloc() [see CONFIG_SYS_MALLOC_LEN
configuration setting]. Below that, a structure with global Board-Info
data is placed, followed by the stack (growing downward).

Additionally, some exception handler code may be copied to the low 8 kB
of DRAM (0x00000000 ... 0x00001fff).

So a typical memory configuration with 16 MB of DRAM could look like
this::

	0x0000 0000	Exception Vector code
	      :
	0x0000 1fff
	0x0000 2000	Free for Application Use
	      :
	      :

	      :
	      :
	0x00fb ff20	Monitor Stack (Growing downward)
	0x00fb ffac	Board Info Data and permanent copy of global data
	0x00fc 0000	Malloc Arena
	      :
	0x00fd ffff
	0x00fe 0000	RAM Copy of Monitor Code
	...		eventually: LCD or video framebuffer
	...		eventually: pRAM (Protected RAM - unchanged by reset)
	0x00ff ffff	[End of RAM]
