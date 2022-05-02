Generic SPL framework
=====================

Overview
--------

To unify all existing implementations for a secondary program loader (SPL)
and to allow simply adding of new implementations this generic SPL framework
has been created. With this framework almost all source files for a board
can be reused. No code duplication or symlinking is necessary anymore.


How it works
------------

The object files for SPL are built separately and placed in the "spl" directory.
The final binaries which are generated are u-boot-spl, u-boot-spl.bin and
u-boot-spl.map.

A config option named CONFIG_SPL_BUILD is enabled by Kconfig for SPL.
Source files can therefore be compiled for SPL with different settings.

For example::

   ifeq ($(CONFIG_SPL_BUILD),y)
   obj-y += board_spl.o
   else
   obj-y += board.o
   endif

   obj-$(CONFIG_SPL_BUILD) += foo.o

   #ifdef CONFIG_SPL_BUILD
           foo();
   #endif


The building of SPL images can be enabled by CONFIG_SPL option in Kconfig.

Because SPL images normally have a different text base, one has to be
configured by defining CONFIG_SPL_TEXT_BASE. The linker script has to be
defined with CONFIG_SPL_LDSCRIPT.

To support generic U-Boot libraries and drivers in the SPL binary one can
optionally define CONFIG_SPL_XXX_SUPPORT. Currently following options
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

Adding SPL-specific code
------------------------

To check whether a feature is enabled, use CONFIG_IS_ENABLED()::

  if (CONFIG_IS_ENABLED(CLK))
      ...

This checks CONFIG_CLK for the main build, CONFIG_SPL_CLK for the SPL build,
CONFIG_TPL_CLK for the TPL build, etc.

U-Boot Phases
-------------

U-Boot boots through the following phases:

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


Checking the boot phase
-----------------------

Use `spl_phase()` to find the current U-Boot phase, e.g. `PHASE_SPL`. You can
also find the previous and next phase and get the phase name.


Device tree
-----------
The U-Boot device tree is filtered by the fdtgrep tools during the build
process to generate a much smaller device tree used in SPL (spl/u-boot-spl.dtb)
with:

- the mandatory nodes (/alias, /chosen, /config)
- the nodes with one pre-relocation property:
  'u-boot,dm-pre-reloc' or 'u-boot,dm-spl'

fdtgrep is also used to remove:

- the properties defined in CONFIG_OF_SPL_REMOVE_PROPS
- all the pre-relocation properties
  ('u-boot,dm-pre-reloc', 'u-boot,dm-spl' and 'u-boot,dm-tpl')

All the nodes remaining in the SPL devicetree are bound
(see doc/driver-model/design.rst).

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
