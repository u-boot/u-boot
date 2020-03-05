.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Chromebook Coral
================

Coral is a Chromebook (or really about 20 different Chromebooks) which use the
Intel Apollo Lake platform (APL). The 'reef' Chromebooks use the same APL SoC so
should also work. Some later ones based on Glacier Lake (GLK) need various
changes in GPIOs, etc. but are very similar.

It is hoped that this port can enable ports to embedded APL boards which are
starting to appear.

Note that booting U-Boot on APL is already supported by coreboot and
Slim Bootloader. This documentation refers to a 'bare metal' port.


Boot flow - TPL
---------------

Apollo Lake boots via an IFWI (Integrated Firmware Image). TPL is placed in
this, in the IBBL entry.

On boot, an on-chip microcontroller called the CSE (Converged Security Engine)
sets up some SDRAM at ffff8000 and loads the TPL image to that address. The
SRAM extends up to the top of 32-bit address space, but the last 2KB is the
start16 region, so the TPL image must be 30KB at most, and CONFIG_TPL_TEXT_BASE
must be ffff8000. Actually the start16 region is small and it could probably
move from f800 to fe00, providing another 1.5KB, but TPL is only about 19KB so
there is no need to change it at present. The size limit is enforced by
CONFIG_TPL_SIZE_LIMIT to avoid producing images that won't boot.

TPL (running from start.S) first sets up CAR (Cache-as-RAM) which provides
larger area of RAM for use while booting. CAR is mapped at CONFIG_SYS_CAR_ADDR
(fef00000) and is 768KB in size. It then sets up the stack in the botttom 64KB
of this space (i.e. below fef10000). This means that the stack and early
malloc() region in TPL can be 64KB at most.

TPL operates without CONFIG_TPL_PCI enabled so PCI config access must use the
x86-specific functions pci_x86_write_config(), etc. SPL creates a simple-bus
device so that PCI devices are bound by driver model. Then arch_cpu_init_tpl()
is called to early init on various devices. This includes placing PCI devices
at hard-coded addresses in the memory map. PCI auto-config is not used.

Most of the 16KB ROM is mapped into the very top of memory, except for the
Intel descriptor (first 4KB) and the space for SRAM as above.

TPL does not set up a bloblist since at present it does not have anything to
pass to SPL.

Once TPL is done it loads SPL from ROM using either the memory-mapped SPI or by
using the Intel fast SPI driver. SPL is loaded into CAR, at the address given
by CONFIG_SPL_TEXT_BASE, which is normally fef10000.

Note that booting using the SPI driver results in an TPL image that is about
26KB in size instead of 19KB. Also boot speed is worse by about 340ms. If you
really want to use the driver, enable CONFIG_APL_SPI_FLASH_BOOT and set
BOOT_FROM_FAST_SPI_FLASH to true[2].


Boot flow - SPL
---------------

SPL (running from start_from_tpl.S) continues to use the same stack as TPL.
It calls arch_cpu_init_spl() to set up a few devices, then init_dram() loads
the FSP-M binary into CAR and runs to, to set up SDRAM. The address of the
output 'HOB' list (Hand-off-block) is stored into gd->arch.hob_list for parsing.
There is a 2GB chunk of SDRAM starting at 0 and the rest is at 4GB.

PCI auto-config is not used in SPL either, but CONFIG_SPL_PCI is defined, so
proper PCI access is available and normal dm_pci_read_config() calls can be
used. However PCI auto-config is not used so the same static memory mapping set
up by TPL is still active.

SPL on x86 always runs with CONFIG_SPL_SEPARATE_BSS=y and BSS is at 120000
(see u-boot-spl.lds). This works because SPL doesn't access BSS until after
board_init_r(), as per the rules, and DRAM is available then.

SPL sets up a bloblist and passes the SPL hand-off information to U-Boot proper.
This includes a pointer to the HOB list as well as DRAM information. See
struct arch_spl_handoff. The bloblist address is set by CONFIG_BLOBLIST_ADDR,
normally 100000.

SPL uses SPI flash to update the MRC caches in ROM. This speeds up subsequent
boots. Be warned that SPL can take 30 seconds without this cache! This is a
known issue with Intel SoCs with modern DRAM and apparently cannot be improved.
The MRC caches are used to work around this.

Once SPL is finished it loads U-Boot into SDRAM at CONFIG_SYS_TEXT_BASE, which
is normally 1110000. Note that CAR is still active.


Boot flow - U-Boot pre-relocation
---------------------------------

U-Boot (running from start_from_spl.S) starts running in RAM and uses the same
stack as SPL. It does various init activities before relocation. Notably
arch_cpu_init_dm() sets up the pin muxing for the chip using a very large table
in the device tree.

PCI auto-config is not used before relocation, but CONFIG_PCI of course is
defined, so proper PCI access is available. The same static memory mapping set
up by TPL is still active until relocation.

As per usual, U-Boot allocates memory at the top of available RAM (a bit below
2GB in this case) and copies things there ready to relocate itself. Notably
reserve_arch() does not reserve space for the HOB list returned by FSP-M since
this is already located in RAM.

U-Boot then shuts down CAR and jumps to its relocated version.


Boot flow - U-Boot post-relocation
----------------------------------

U-Boot starts up normally, running near the top of RAM. After driver model is
running, arch_fsp_init_r() is called which loads and runs the FSP-S binary.
This updates the HOB list to include graphics information, used by the fsp_video
driver.

PCI autoconfig is done and a few devices are probed to complete init. Most
others are started only when they are used.

Note that FSP-S is supposed to run after CAR has been shut down, which happens
immediately before U-Boot starts up in its relocated position. Therefore we
cannot run FSP-S before relocation. On the other hand we must run it before
PCI auto-config is done, since FSP-S may show or hide devices. The first device
that probes PCI after relocation is the serial port, in initr_serial(), so FSP-S
must run before that. A corollary is that loading FSP-S must be done without
using the SPI driver, to avoid probing PCI and causing an autoconfig, so
memory-mapped reading is always used for FSP-S.

It would be possible to tear down CAR in SPL instead of U-Boot. The SPL handoff
information could make sure it does not include any pointers into CAR (in fact
it doesn't). But tearing down CAR in U-Boot allows the initial state used by TPL
and SPL to be read by U-Boot, which seems useful. It also matches how older
platforms start up (those that don't use SPL).


Performance
-----------

Bootstage is used through all phases of U-Boot to keep accurate timimgs for
boot. Use 'bootstage report' in U-Boot to see the report, e.g.::

    Timer summary in microseconds (16 records):
           Mark    Elapsed  Stage
              0          0  reset
        155,325    155,325  TPL
        204,014     48,689  end TPL
        204,385        371  SPL
        738,633    534,248  end SPL
        739,161        528  board_init_f
        842,764    103,603  board_init_r
      1,166,233    323,469  main_loop
      1,166,283         50  id=175

    Accumulated time:
                        62  fast_spi
                       202  dm_r
                     7,779  dm_spl
                    15,555  dm_f
                   208,357  fsp-m
                   239,847  fsp-s
                   292,143  mmap_spi

CPU performance is about 3500 DMIPS::

    => dhry
    1000000 iterations in 161 ms: 6211180/s, 3535 DMIPS


Partial memory map
------------------

::

    ffffffff       Top of ROM (and last byte of 32-bit address space)
    ffff8000       TPL loaded here (from IFWI)
    ff000000       Bottom of ROM
    fefc0000       Top of CAR region
    fef96000       Stack for FSP-M
    fef40000 59000 FSP-M
    fef11000       SPL loaded here
    fef10000       CONFIG_BLOBLIST_ADDR
    fef10000       Stack top in TPL, SPL and U-Boot before relocation
    fef00000  1000 CONFIG_BOOTSTAGE_STASH_ADDR
    fef00000       Base of CAR region

       f0000       CONFIG_ROM_TABLE_ADDR
      120000       BSS (defined in u-boot-spl.lds)
      200000       FSP-S (which is run after U-Boot is relocated)
     1110000       CONFIG_SYS_TEXT_BASE


Supported peripherals
---------------------

- UART
- SPI flash
- Video
- MMC (dev 0) and micro-SD (dev 1)
- Chrome OS EC
- Keyboard
- USB


To do
-----

- Finish peripherals
   - left-side USB
   - USB-C
   - Cr50 (security chip: a basic driver is running but not included here)
   - Sound (Intel I2S support exists, but need da7219 driver)
   - Various minor features supported by LPC, etc.
- Booting Chrome OS, e.g. with verified boot
- Integrate with Chrome OS vboot
- Improvements to booting from coreboot (i.e. as a coreboot target)
- Use FSP-T binary instead of our own CAR implementation
- Use the official FSP package instead of the coreboot one
- Enable all CPU cores
- Suspend / resume
- ACPI


Credits
-------

This is a spare-time project conducted slowly over a long period of time.

Much of the code for this port came from Coreboot, an open-source firmware
project similar to U-Boot's SPL in terms of features.

Also see [2] for information about the boot flow used by coreboot. It is
similar, but has an extra postcar stage. U-Boot doesn't need this since it
supports relocating itself in memory.


[2] Intel PDF https://www.coreboot.org/images/2/23/Apollolake_SoC.pdf
