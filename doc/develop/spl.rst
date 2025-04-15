Generic xPL framework
=====================

Overview
--------

To unify all existing implementations for secondary/tertiary program loaders
(generically called xPL)
and to allow simply adding of new implementations this generic xPL framework
has been created. With this framework almost all source files for a board
can be reused. No code duplication or symlinking is necessary anymore.


How it works
------------

The object files for xPL are built separately and placed in a subdirectory
("spl", "tpl" or "vpl").
The final binaries which are generated for SPL are u-boot-spl, u-boot-spl.bin
and u-boot-spl.map

A config option named CONFIG_XPL_BUILD is enabled by Kconfig for xPL builds.
Source files can therefore be compiled for xPL with different settings.

For example::

   ifeq ($(CONFIG_XPL_BUILD),y)
   obj-y += board_spl.o
   else
   obj-y += board.o
   endif

   obj-$(CONFIG_XPL_BUILD) += foo.o

   if (IS_ENABLED(CONFIG_XPL_BUILD))
           foo();

   if (xpl_phase() == PHASE_TPL)
           bar();

The building of xPL images can be enabled by CONFIG_SPL (etc.) options in
Kconfig.

Because xPL images normally have a different text base, one has to be
configured by defining CONFIG_xPL_TEXT_BASE. The linker script has to be
defined with CONFIG_xPL_LDSCRIPT.

To support generic U-Boot libraries and drivers in the xPL binary one can
optionally define CONFIG_xPL_XXX_SUPPORT. Currently following options
are supported:

CONFIG_SPL_LIBCOMMON_SUPPORT (common/libcommon.o)
CONFIG_SPL_LIBDISK_SUPPORT (disk/libdisk.o)
CONFIG_SPL_I2C (drivers/i2c/libi2c.o)
CONFIG_SPL_GPIO (drivers/gpio/libgpio.o)
CONFIG_SPL_MMC (drivers/mmc/libmmc.o)
CONFIG_SPL_SERIAL (drivers/serial/libserial.o)
CONFIG_SPL_SPI_FLASH_SUPPORT (drivers/mtd/spi/libspi_flash.o)
CONFIG_SPL_SPI (drivers/spi/libspi.o)
CONFIG_SPL_FS_FAT (fs/fat/libfat.o)
CONFIG_SPL_FS_EXT4
CONFIG_SPL_LIBGENERIC_SUPPORT (lib/libgeneric.o)
CONFIG_SPL_POWER (drivers/power/libpower.o)
CONFIG_SPL_NAND_SUPPORT (drivers/mtd/nand/raw/libnand.o)
CONFIG_SPL_DRIVERS_MISC (drivers/misc)
CONFIG_SPL_DMA (drivers/dma/libdma.o)
CONFIG_SPL_POST_MEM_SUPPORT (post/drivers/memory.o)
CONFIG_SPL_NAND_LOAD (drivers/mtd/nand/raw/nand_spl_load.o)
CONFIG_SPL_SPI_LOAD (drivers/mtd/spi/spi_spl_load.o)
CONFIG_SPL_RAM_DEVICE (common/spl/spl.c)
CONFIG_SPL_WATCHDOG (drivers/watchdog/libwatchdog.o)
CONFIG_SPL_SYSCON (drivers/core/syscon-uclass.o)
CONFIG_SPL_GZIP (lib/gzip.o)
CONFIG_SPL_VIDEO (drivers/video/video-uclass.o drivers/video/vidconsole-uclass.o)
CONFIG_SPL_SPLASH_SCREEN (common/splash.o)
CONFIG_SPL_SPLASH_SOURCE (common/splash_source.o)
CONFIG_SPL_GPIO (drivers/gpio)
CONFIG_SPL_DM_GPIO (drivers/gpio/gpio-uclass.o)
CONFIG_SPL_BMP (drivers/video/bmp.o)
CONFIG_SPL_BLOBLIST (common/bloblist.o)

Adding xPL-specific code
------------------------

To check whether a feature is enabled, use CONFIG_IS_ENABLED()::

  if (CONFIG_IS_ENABLED(CLK))
      ...

This checks CONFIG_CLK for the main build, CONFIG_SPL_CLK for the SPL build,
CONFIG_TPL_CLK for the TPL build, etc.

U-Boot Boot Phases
------------------

U-Boot goes through the following boot phases where TPL, VPL, SPL are optional.
While many boards use SPL, fewer use TPL.

TPL
   Very early init, as tiny as possible. This loads SPL (or VPL if enabled).

VPL
   Optional verification step, which can select one of several SPL binaries,
   if A/B verified boot is enabled. Implementation of the VPL logic is
   work-in-progress. For now it just boots into SPL.

SPL
   Secondary program loader. Sets up SDRAM and loads U-Boot proper. It may also
   load other firmware components.

U-Boot
   U-Boot proper, containing the command line and boot logic.

Further usages of U-Boot SPL comprise:

* Launching BL31 of ARM Trusted Firmware which invokes main U-Boot as BL33
* launching EDK II
* launching Linux kernel
* launching RISC-V OpenSBI which invokes main U-Boot

Checking the boot phase
-----------------------

Use `xpl_phase()` to find the current U-Boot phase, e.g. `PHASE_SPL`. You can
also find the previous and next phase and get the phase name.


.. _fdtgrep_filter:

Device tree
-----------
The U-Boot device tree is filtered by the fdtgrep tools during the build
process to generate a much smaller device tree used in SPL (spl/u-boot-spl.dtb)
with:

- the mandatory nodes (/alias, /chosen, /config)
- the nodes with one pre-relocation property:
  'bootph-all' or 'bootph-pre-ram'

fdtgrep is also used to remove:

- the properties defined in CONFIG_OF_SPL_REMOVE_PROPS
- all the pre-relocation properties
  ('bootph-all', 'bootph-pre-ram' (SPL), 'bootph-pre-sram' (TPL) and
  'bootph-verify' (TPL))

All the nodes remaining in the SPL devicetree are bound
(see doc/driver-model/design.rst).

NOTE: U-Boot migrated to a new schema for the u-boot,dm-* tags in 2023. Please
update to use the new bootph-* tags as described in the
doc/device-tree-bindings/bootph.yaml binding file.

Debugging
---------

When building SPL with DEBUG set you may also need to set CONFIG_PANIC_HANG
as in most cases do_reset is not defined within SPL.


Estimating stack usage
----------------------

With gcc 4.6 (and later) and the use of GNU cflow it is possible to estimate
stack usage at various points in run sequence of SPL.  The -fstack-usage option
to gcc will produce '.su' files (such as arch/arm/cpu/armv7/syslib.su) that
will give stack usage information and cflow can construct program flow.

Must have gcc 4.6 or later, which supports -fstack-usage:

#. Build normally
#. Perform the following shell command to generate a list of C files used in
   SPL:
#. `find spl -name '*.su' | sed -e 's:^spl/::' -e 's:[.]su$:.c:' > used-spl.list`
#. Execute cflow:
   `$ cflow --main=board_init_r $(cat used-spl.list) 2>&1 | $PAGER`

cflow will spit out a number of warnings as it does not parse
the config files and picks functions based on #ifdef.  Parsing the '.i'
files instead introduces another set of headaches.  These warnings are
not usually important to understanding the flow, however.


Reserving memory in xPL
-----------------------

If memory needs to be reserved in RAM during an xPL phase with the requirement
that the xPL reserved memory remains preserved across further boot phases too
then it needs to be reserved mandatorily starting from end of RAM. This is to
ensure that further phases can simply skip this region before carrying out
further reservations or updating the relocation address.

Also out of these regions which are to be preserved across further phases of
boot, video framebuffer memory region must be reserved first starting from
end of RAM for which the helper function spl_reserve_video_from_ram_top() is
provided
which makes sure that video memory is placed at the top of reservation area with
further reservations below it.

The reservation information for these regions can be passed to the
further boot phases using a bloblist. For e.g. the information for the
framebuffer area reserved by xPL can be passed onto U-Boot using
BLOBLISTT_U_BOOT_VIDEO

The further boot phases need to parse each of the blobs passed from xPL phase
starting from video bloblist and skip this whole xPL reserved-memory area from
end of RAM as per the blobs received, before carrying out further
reservations or updating the relocation address. For e.g, U-Boot proper uses
function setup_relocaddr_from_bloblist() to parse the bloblist passed from
previous phase and skip the memory reserved from previous phase accordingly.
