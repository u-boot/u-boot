HCU5 configuration details and startup sequence

(C) Copyright 2007 Netstal Maschinen AG
    Niklaus Giger (Niklaus.Giger@netstal.com)

TODO:
-----
- Fix error: Waiting for PHY auto negotiation to complete..... TIMEOUT !
     - Does not occur if both EMAC are connected
- Fix RTS/CTS problem (HW?)
  CONFIG_SERIAL_MULTI/CONFIG_SERIAL_SOFTWARE_FIFO hangs after
  Switching to interrupt driven serial input mode

Caveats:
--------
Errata CHIP_8: Incorrect Write to DDR SDRAM. (was not applied to sequoia.c)
see hcu5.c.


Memory Bank 0 -- Flash chip
---------------------------

0xfff00000 - 0xffffffff

The flash chip is really only 512Kbytes, but the high address bit of
the 1Meg region is ignored, so the flash is replicated through the
region. Thus, this is consistent with a flash base address 0xfff80000.

The placement at the end is to be consistent with reset behavior,
where the processor itself initially uses this bus to load the branch
vector and start running.

On-Chip Memory
--------------

0xe0010000- 0xe0013fff   CONFIG_SYS_OCM_BASE
The 440EPx includes a 16K on-chip memory that can be placed however
software chooses.

Internal Peripherals
--------------------

0xef600300 - 0xef6008ff

These are scattered various peripherals internal to the PPC440EPX
chip.

Chip-Select 2: Flash Memory
---------------------------

Not used

Chip-Select 3: CAN Interface
----------------------------
0xc800000: 2 Intel 82527 CAN-Controller


Chip-Select 4: IMC-bus standard
-------------------------------

0xcc00000: Netstal specific IO-Bus


Chip-Select 5: IMC-bus fast (inactive)
--------------------------------------

0xce00000: Netstal specific IO-Bus (fast, but not yet used)


Memory Bank 1 -- DDR2
-------------------------------------

0x00000000 - 0xfffffff   # Default 256 MB

PCI ??

USB ??
Only USB_STORAGE is enabled to load vxWorks
from a memory stick.

System-LEDs ??? (Analog zu HCU4 ???)

Startup sequence
----------------

(cpu/ppc4xx/resetvec.S)
depending on configs option
call _start_440 _start_pci oder _start

(cpu/ppc4xx/start.S)

_start_440:
	initialize register like
	CCR0
	debug
	setup interrupt vectors
	configure cache regions
	clear and setup TLB
	enable internal RAM
	jump start_ram
	which in turn will jump to start
_start:
	Clear and set up some registers.
	Debug setup
	Setup the internal SRAM
	Setup the stack in internal SRAM
    setup stack pointer (r1)
    setup GOT
	call cpu_init_f	/* run low-level CPU init code	   (from Flash) */

    call cpu_init_f
    board_init_f: (lib_ppc\board.c)
	init_sequence defines a list of function to be called
	    board_early_init_f: (board/netstal/hcu5/hcu5.c)
		We are using Bootstrap-Option A
		if CPR0_ICFG_RLI_MASK == 0 then set some registers and reboot
		Setup the GPIO pins
		Setup the interrupt controller polarities, triggers, etc.
		Ethernet, PCI, USB enable
		setup BOOT FLASH (Chip timing)
	    init_baudrate,
	    serial_init
	    checkcpu
	    misc_init_f #ifdef
	    init_func_i2c #ifdef
	    post_init_f  #ifdef
	    init_func_ram -> calls init_dram board/netstal/hcu5/sdram.c
		(EYE function removed!!)
	    test_dram call

	 * Reserve memory at end of RAM for (top down in that order):
	 *  - kernel log buffer
	 *  - protected RAM
	 *  - LCD framebuffer
	 *  - monitor code
	 *  - board info struct
	Save local variables to board info struct
	call relocate_code() does not return
	relocate_code: (cpu/ppc4xx/start.S)
-------------------------------------------------------
From now on our copy is in RAM and we will run from there,
	starting with board_init_r
-------------------------------------------------------
    board_init_r: (lib_ppc\board.c)
	setup bd function pointers
	trap_init
	flash_init: (board/netstal/hcu5/flash.c)
		/* setup for u-boot erase, update */
	setup bd flash info
	cpu_init_r: (cpu/ppc4xx/cpu_init.c)
	    peripheral chip select in using defines like
	    CONFIG_SYS_EBC_PB0A, CONFIG_SYS_EBC_PB0C from hcu5.h
	mem_malloc_init
	malloc_bin_reloc
	spi_init (r or f)??? (CONFIG_ENV_IS_IN_EEPROM)
	env_relocated
	misc_init_r(bd): (board/netstal/hcu5.c)
	    ethaddr mit serial number ergänzen
    Then we will somehow go into the command loop

Most of the HW specific code for the HCU5 may be found in
include/configs/hcu5.h
board/netstal/hcu5/*
cpu/ppc4xx/*
lib_ppc/*
include/ppc440.h

Drivers for serial etc are found under drivers/

Don't ask question if you did not look at the README !!
Most CONFIG_SYS_* and CONFIG_* switches are mentioned/explained there.
