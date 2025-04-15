.. SPDX-License-Identifier: GPL-2.0-or-later

Memory Management
=================

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

System RAM Utilization in U-Boot in ARM
---------------------------------------

Let us break down the relevant parts of the execution sequence where system RAM
comes into play. Please note that these are individual pieces of the entire
boot sequence. It is not exhaustive, rather it aims to show only the pieces
where the system RAM is modified. See arch/arm/lib/crt0* to understand the
complete execution sequence.

Also note that the below sequence is not a hard and fast rule on how DRAM usage
would be and an architecture and board specific analysis is required for the
exact flow.

SPL Flow
........

   #. Pre-DRAM

      Prior to setting up of the DRAM, the stack, malloc is defined as below
      possibly sitting on a smaller readily available memory (SRAM etc.):

      .. image:: pics/spl_before_reloc.svg
         :alt: contents of ready RAM before relocation in SPL

      Please see CONFIG_SPL_EARLY_BSS if BSS initialization is needed prior
      to entering board_init_f().


   #. DRAM Initialization

      This is typically triggered by board_init_f prior to relocating the stack
      and the GD (optionally) to the system RAM. DRAM drivers reside in
      drivers/ram/. Their probe/configuration can be done either via placing the
      logic in dram_init() or wherever deemed applicable within board_init_f.

      Post board_init_f, spl_relocate_stack_gd() is called to relocate the stack
      and the GD to the newly initialized DRAM. If CONFIG_SPL_SYS_MALLOC_SIMPLE
      is set it is also possible to use some amount of this DRAM stack as memory
      pool for malloc_simple.

      Both of which are an optional move at this point in the sequence. This is
      still an intermediate environment.

   #. Final Environment Set Up

      The final environment is setup and the system RAM now looks like this:

      .. image:: pics/spl_after_reloc.svg
         :alt: contents of DRAM after relocation in SPL

      Again stack and gd are an optional move and may still remain in the
      available RAM (SRAM, locked cache etc.)

U-Boot Proper Flow
..................

   TODO: this section is still under progress

   #. DRAM Initialization

      This follows the same as in SPL flow. In board_init_f(), a part of memory
      is reserved at the end of RAM (see reserve_* functions in init_sequence_f)

   #. Code Relocation

      relocate_code() is called which relocates U-Boot code from the current
      location into the relocation destination in system RAM. Typically it is
      relocated to the upper portion of the memory. So DRAM now has:
      * stack
      * gd
      * code

      The code relocation happens to the upper portion of the memory after certain
      portion of memory is reserved. This is memory that is intended to not be
      "touched" by U-Boot.

   #. Final Environment Set Up

      At this stage we are completely running out of the system RAM with:
      * stack
      * gd
      * code
      * bss
      * initialized non-const data
      * initialized const data

      It is better to do a complete analysis to visualize the layers the system
      RAM is composed of at the end of this flow. This is entirely dependent on
      CPU/SoC architecture.

Getting information about system RAM
....................................

   At boot:

   The prints given by announce_dram_init() and show_dram_config() come up in the
   boot banner like so::

      DRAM:  2 GiB (total 32 GiB)

   U-Boot supports addressing upto 39-bit. To avoid trying to access higher
   addresses in systems with > 39-bit addresses, U-Boot caps itself (gd->ram_size)
   to the first bank. This is also inline with philosophy that U-Boot is a
   bootloader and not a full-fledged operating system. The first value represents
   this memory that is available for U-Boot while the "total" value represents the
   total system RAM available on the device.

   Getting the most basic information on how system RAM has been set up is by
   running `bdinfo` at U-Boot prompt::

      => bdinfo
      boot_params = 0x0000000000000000
      DRAM bank   = 0x0000000000000000
      -> start    = 0x0000000080000000
      -> size     = 0x0000000080000000
      DRAM bank   = 0x0000000000000001
      -> start    = 0x0000000880000000
      -> size     = 0x0000000780000000
      flashstart  = 0x0000000000000000
      flashsize   = 0x0000000000000000
      flashoffset = 0x0000000000000000
      baudrate    = 115200 bps
      relocaddr   = 0x00000000ffec1000
      reloc off   = 0x000000007f6c1000
      Build       = 64-bit
      current eth = ethernet@46000000port@1
      ethaddr     = 3c:e0:64:62:4b:4e
      IP addr     = <NULL>
      fdt_blob    = 0x00000000fde7df60
      lmb_dump_all:
       memory.count = 0x1
       memory[0]      [0x80000000-0xffffffff], 0x80000000 bytes flags: none
       reserved.count = 0x2
       reserved[0]    [0x9e800000-0xabffffff], 0x0d800000 bytes flags: no-map
       reserved[1]    [0xfce79f50-0xffffffff], 0x031860b0 bytes flags: no-overwrite
      devicetree  = separate
      serial addr = 0x0000000002880000
       width      = 0x0000000000000000
       shift      = 0x0000000000000002
       offset     = 0x0000000000000000
       clock      = 0x0000000002dc6c00
      arch_number = 0x0000000000000000
      TLB addr    = 0x00000000ffff0000
      irq_sp      = 0x00000000fde7df50
      sp start    = 0x00000000fde7df50
      Early malloc usage: 3288 / 8000


   Here you are able to see the banks of DDR that have been set up in DRAM bank
   -> start and -> size as well as the reserved memories in lmb_dump_all.

Testing Memory
--------------

   Please see doc/README.memory-test
