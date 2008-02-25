MCU25 Configuration Details

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

0xf4000000 - 0xf4000fff

The 405GPr includes a 4K on-chip memory that can be placed however
software chooses. I choose to place the memory at this address, to
keep it out of the cachable areas.


Internal Peripherals
--------------------

0xef600300 - 0xef6008ff

These are scattered various peripherals internal to the PPC405GPr
chip.

Chip-Select 2: Flash Memory
---------------------------

0x70000000

Chip-Select 3: CAN Interface
----------------------------
0x7800000


Chip-Select 4: IMC-bus standard
-------------------------------

Our IO-Bus (slow version)


Chip-Select 5: IMC-bus fast (inactive)
--------------------------------------

Our IO-Bus (fast, but not yet use)


Memory Bank 1 -- SDRAM
-------------------------------------

0x00000000 - 0x2ffffff   # Default 64 MB

