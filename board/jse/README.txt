JSE Configuration Details

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


Memory Bank 1 -- SystemACE Controller
-------------------------------------

0xf0000000 - 0xf00fffff

The SystemACE chip is along on peripheral bank CS#1. We don't need
much space, but 1Meg is the smallest we can configure the chip to
allocate. We need it far away from the flash region, because this
region is set to be non-cached.


Internal Peripherals
--------------------

0xef600300 - 0xef6008ff

These are scattered various peripherals internal to the PPC405GPr
chip.

SDRAM
-----

0x00000000 - 0x07ffffff  (128 MBytes)
