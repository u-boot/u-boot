menu "TPL configuration options"
	depends on TPL

config TPL_SIZE_LIMIT
	hex "Maximum size of TPL image"
	default 0x0
	help
	  Specifies the maximum length of the U-Boot TPL image.
	  If this value is zero, it is ignored.

config TPL_BINMAN_SYMBOLS
	bool "Support binman symbols in TPL"
	depends on TPL_FRAMEWORK && BINMAN
	default y
	help
	  This enables use of symbols in TPL which refer to other entries in
	  the same binman image as the TPL. These can be declared with the
	  binman_sym_declare(type, entry, prop) macro and accessed by the
	  binman_sym(type, entry, prop) macro defined in binman_sym.h.

	  See tools/binman/binman.rst for a detailed explanation.

config TPL_BINMAN_UBOOT_SYMBOLS
	bool "Declare binman symbols for U-Boot phases in TPL"
	depends on TPL_BINMAN_SYMBOLS
	default n if ARCH_IMX8M
	default y
	help
	  This enables use of symbols in TPL which refer to U-Boot phases,
	  enabling TPL to obtain the location and size of its next phase simply
	  by calling spl_get_image_pos() and spl_get_image_size().

	  For this to work, you must have all U-Boot phases in the same binman
	  image, so binman can update TPL with the locations of everything.

config TPL_FRAMEWORK
	bool "Support TPL based upon the common SPL framework"
	default y if SPL_FRAMEWORK
	help
	  Enable the SPL framework under common/spl/ for TPL builds.
	  This framework supports MMC, NAND and YMODEM and other methods
	  loading of U-Boot's SPL stage. If unsure, say Y.

config TPL_BANNER_PRINT
	bool "Enable output of the TPL banner 'U-Boot TPL ...'"
	default y
	help
	  If this option is enabled, TPL will print the banner with version
	  info. Disabling this option could be useful to reduce TPL boot time
	  (e.g. approx. 6 ms faster, when output on i.MX6 with 115200 baud).

config TPL_HANDOFF
	bool "Pass hand-off information from TPL to SPL and U-Boot proper"
	depends on HANDOFF && TPL_BLOBLIST
	default y
	help
	  This option enables TPL to write handoff information. This can be
	  used to pass information like the size of SDRAM from TPL to U-Boot
	  proper. The information is also available to SPL if it is useful
	  there.

config TPL_BOARD_INIT
	bool "Call board-specific initialization in TPL"
	help
	  If this option is enabled, U-Boot will call the function
	  spl_board_init() from board_init_r(). This function should be
	  provided by the board.

config TPL_BOOTCOUNT_LIMIT
	bool "Support bootcount in TPL"
	depends on TPL_ENV_SUPPORT
	help
	  If this option is enabled, the TPL will support bootcount.
	  For example, it may be useful to choose the device to boot.

config TPL_SYS_MALLOC_SIMPLE
	bool
	prompt "Only use malloc_simple functions in the TPL"
	help
	  Say Y here to only use the *_simple malloc functions from
	  malloc_simple.c, rather then using the versions from dlmalloc.c;
	  this will make the TPL binary smaller at the cost of more heap
	  usage as the *_simple malloc functions do not re-use free-ed mem.

config TPL_SEPARATE_BSS
	bool "BSS section is in a different memory region from text"
	default y if SPL_SEPARATE_BSS
	help
	  Some platforms need a large BSS region in TPL and can provide this
	  because RAM is already set up. In this case BSS can be moved to RAM.
	  This option should then be enabled so that the correct device tree
	  location is used. Normally we put the device tree at the end of BSS
	  but with this option enabled, it goes at _image_binary_end.

config TPL_LDSCRIPT
	string "Linker script for the TPL stage"
	default "arch/arm/cpu/armv8/u-boot-spl.lds" if ARM64
	default "arch/\$(ARCH)/cpu/u-boot-spl.lds"
	help
	  The TPL stage will usually require a different linker-script
	  (as it runs from a different memory region) than the regular
	  U-Boot stage.  Set this to the path of the linker-script to
	  be used for TPL.

	  May be left empty to trigger the Makefile infrastructure to
	  fall back to the linker-script used for the SPL stage.

config TPL_NEEDS_SEPARATE_STACK
	bool "TPL needs a separate initial stack-pointer"
	help
	  Enable, if the TPL stage should not inherit its initial
	  stack-pointer from the settings for the SPL stage.

config TPL_POWER
	bool "Support power drivers"
	help
	  Enable support for power control in TPL. This includes support
	  for PMICs (Power-management Integrated Circuits) and some of the
	  features provided by PMICs. In particular, voltage regulators can
	  be used to enable/disable power and vary its voltage. That can be
	  useful in TPL to turn on boot peripherals and adjust CPU voltage
	  so that the clock speed can be increased. This enables the drivers
	  in drivers/power, drivers/power/pmic and drivers/power/regulator
	  as part of an TPL build.

config TPL_TEXT_BASE
	hex "Base address for the .text section of the TPL stage"
	default 0
	help
	  The base address for the .text section of the TPL stage.

config TPL_MAX_SIZE
	hex "Maximum size (in bytes) for the TPL stage"
	default 0x2e000 if ROCKCHIP_RK3399
	default 0x8000 if ROCKCHIP_RK3288
	default 0x7000 if ROCKCHIP_RK322X || ROCKCHIP_RK3328 || ROCKCHIP_RK3368
	default 0x2800 if ROCKCHIP_PX30
	default 0x0
	help
	  The maximum size (in bytes) of the TPL stage.

config TPL_STACK
	hex "Address of the initial stack-pointer for the TPL stage"
	depends on TPL_NEEDS_SEPARATE_STACK
	help
	  The address of the initial stack-pointer for the TPL stage.
	  Usually this will be the (aligned) top-of-stack.

config TPL_READ_ONLY
	bool
	depends on TPL_OF_PLATDATA
	select TPL_OF_PLATDATA_NO_BIND
	select TPL_OF_PLATDATA_RT
	help
	  Some platforms (e.g. x86 Apollo Lake) load SPL into a read-only
	  section of memory. This means that of-platdata must make a copy (in
	  writeable memory) of anything it wants to modify, such as
	  device-private data.

config TPL_BOOTROM_SUPPORT
	bool "Support returning to the BOOTROM (from TPL)"
	help
	  Some platforms (e.g. the Rockchip RK3368) provide support in their
	  ROM for loading the next boot-stage after performing basic setup
	  from the TPL stage.

	  Enable this option, to return to the BOOTROM through the
	  BOOT_DEVICE_BOOTROM (or fall-through to the next boot device in the
	  boot device list, if not implemented for a given board)

config TPL_CRC32
	bool "Support CRC32 in TPL"
	default y if TPL_ENV_SUPPORT || TPL_BLOBLIST
	help
	  Enable this to support CRC32 in uImages or FIT images within SPL.
	  This is a 32-bit checksum value that can be used to verify images.
	  For FIT images, this is the least secure type of checksum, suitable
	  for detected accidental image corruption. For secure applications you
	  should consider SHA1 or SHA256.

config TPL_DRIVERS_MISC
	bool "Support misc drivers in TPL"
	help
	  Enable miscellaneous drivers in TPL. These drivers perform various
	  tasks that don't fall nicely into other categories, Enable this
	  option to build the drivers in drivers/misc as part of an TPL
	  build, for those that support building in TPL (not all drivers do).

config TPL_ENV_SUPPORT
	bool "Support an environment"
	help
	  Enable environment support in TPL. See SPL_ENV_SUPPORT for details.

config TPL_GPIO
	bool "Support GPIO in TPL"
	help
	  Enable support for GPIOs (General-purpose Input/Output) in TPL.
	  GPIOs allow U-Boot to read the state of an input line (high or
	  low) and set the state of an output line. This can be used to
	  drive LEDs, control power to various system parts and read user
	  input. GPIOs can be useful in TPL to enable a 'sign-of-life' LED,
	  for example. Enable this option to build the drivers in
	  drivers/gpio as part of an TPL build.

config TPL_I2C
	bool "Support I2C"
	help
	  Enable support for the I2C bus in TPL. See SPL_I2C for
	  details.

config TPL_LIBCOMMON_SUPPORT
	bool "Support common libraries"
	help
	  Enable support for common U-Boot libraries within TPL. See
	  SPL_LIBCOMMON_SUPPORT for details.

config TPL_LIBGENERIC_SUPPORT
	bool "Support generic libraries"
	help
	  Enable support for generic U-Boot libraries within TPL. See
	  SPL_LIBGENERIC_SUPPORT for details.

config TPL_MPC8XXX_INIT_DDR
	bool "Support MPC8XXX DDR init"
	help
	  Enable support for DDR-SDRAM on the MPC8XXX family within TPL. See
	  SPL_MPC8XXX_INIT_DDR for details.

config TPL_MMC
	bool "Support MMC"
	depends on MMC
	help
	  Enable support for MMC within TPL. See SPL_MMC for details.

config TPL_NAND_SUPPORT
	bool "Support NAND flash"
	help
	  Enable support for NAND in TPL. See SPL_NAND_SUPPORT for details.

config TPL_PCI
	bool "Support PCI drivers"
	help
	  Enable support for PCI in TPL. For platforms that need PCI to boot,
	  or must perform some init using PCI in SPL, this provides the
	  necessary driver support. This enables the drivers in drivers/pci
	  as part of a TPL build.

config TPL_PCH
	bool "Support PCH drivers"
	help
	  Enable support for PCH (Platform Controller Hub) devices in TPL.
	  These are used to set up GPIOs and the SPI peripheral early in
	  boot. This enables the drivers in drivers/pch as part of a TPL
	  build.

config TPL_RAM_SUPPORT
	bool "Support booting from RAM"
	help
	  Enable booting of an image in RAM. The image can be preloaded or
	  it can be loaded by TPL directly into RAM (e.g. using USB).

config TPL_RAM_DEVICE
	bool "Support booting from preloaded image in RAM"
	depends on TPL_RAM_SUPPORT
	help
	  Enable booting of an image already loaded in RAM. The image has to
	  be already in memory when TPL takes over, e.g. loaded by the boot
	  ROM.

config TPL_RTC
	bool "Support RTC drivers"
	help
	  Enable RTC (Real-time Clock) support in TPL. This includes support
	  for reading and setting the time. Some RTC devices also have some
	  non-volatile (battery-backed) memory which is accessible if
	  needed. This enables the drivers in drivers/rtc as part of an TPL
	  build.

config TPL_SERIAL
	bool "Support serial"
	select TPL_PRINTF
	select TPL_STRTO
	help
	  Enable support for serial in TPL. See SPL_SERIAL for
	  details.

config TPL_SPI_FLASH_SUPPORT
	bool "Support SPI flash drivers"
	help
	  Enable support for using SPI flash in TPL. See SPL_SPI_FLASH_SUPPORT
	  for details.

config TPL_SPI_FLASH_TINY
	bool "Enable low footprint TPL SPI Flash support"
	depends on TPL_SPI_FLASH_SUPPORT && !SPI_FLASH_BAR
	default y if SPI_FLASH
	help
	 Enable lightweight TPL SPI Flash support that supports just reading
	 data/images from flash. No support to write/erase flash. Enable
	 this if you have TPL size limitations and don't need full-fledged
	 SPI flash support.

config TPL_SPI_LOAD
	bool "Support loading from SPI flash"
	depends on TPL_SPI_FLASH_SUPPORT
	help
	  Enable support for loading next stage, U-Boot or otherwise, from
	  SPI NOR in U-Boot TPL.

config TPL_SPI
	bool "Support SPI drivers"
	help
	  Enable support for using SPI in TPL. See SPL_SPI for
	  details.

config TPL_DM_SPI
	bool "Support SPI DM drivers in TPL"
	help
	  Enable support for SPI DM drivers in TPL.

config TPL_DM_SPI_FLASH
	bool "Support SPI DM FLASH drivers in TPL"
	help
	  Enable support for SPI DM flash drivers in TPL.

config TPL_YMODEM_SUPPORT
	bool "Support loading using Ymodem"
	depends on TPL_SERIAL
	help
	  While loading from serial is slow it can be a useful backup when
	  there is no other option. The Ymodem protocol provides a reliable
	  means of transmitting U-Boot over a serial line for using in TPL,
	  with a checksum to ensure correctness.

endmenu
