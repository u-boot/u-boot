.. SPDX-License-Identifier: GPL-2.0+

Board Initialisation Flow
-------------------------

This is the intended start-up flow for boards. This should apply for both
xPL and U-Boot proper (i.e. they both follow the same rules).

Note: "xPL" stands for "any Program Loader", including SPL (Secondary
Program Loader), TPL (Tertiary Program Loader) and VPL (Verifying Program
Loader). The boot sequence is TPL->VPL->SPL->U-Boot proper

At present, xPL mostly uses a separate code path, but the function names
and roles of each function are the same. Some boards or architectures
may not conform to this.  At least most ARM boards which use
CONFIG_xPL_FRAMEWORK conform to this.

Execution typically starts with an architecture-specific (and possibly
CPU-specific) start.S file, such as:

- arch/arm/cpu/armv7/start.S
- arch/powerpc/cpu/mpc83xx/start.S
- arch/mips/cpu/start.S

and so on. From there, three functions are called; the purpose and
limitations of each of these functions are described below.

lowlevel_init()
~~~~~~~~~~~~~~~

- purpose: essential init to permit execution to reach board_init_f()
- no global_data or BSS
- there is no stack (ARMv7 may have one but it will soon be removed)
- must not set up SDRAM or use console
- must only do the bare minimum to allow execution to continue to
  board_init_f()
- this is almost never needed
- return normally from this function

board_init_f()
~~~~~~~~~~~~~~

- purpose: set up the machine ready for running board_init_r():
  i.e. SDRAM and serial UART
- global_data is available
- stack is in SRAM
- BSS is not available, so you cannot use global/static variables,
  only stack variables and global_data

Non-xPL-specific notes:

    - dram_init() is called to set up DRAM. If already done in xPL this
      can do nothing

xPL-specific notes:

    - you can override the entire board_init_f() function with your own
      version as needed.
    - preloader_console_init() can be called here in extremis
    - should set up SDRAM, and anything needed to make the UART work
    - there is no need to clear BSS, it will be done by crt0.S
    - for specific scenarios on certain architectures an early BSS *can*
      be made available (via CONFIG_SPL_EARLY_BSS by moving the clearing
      of BSS prior to entering board_init_f()) but doing so is discouraged.
      Instead it is strongly recommended to architect any code changes
      or additions such to not depend on the availability of BSS during
      board_init_f() as indicated in other sections of this README to
      maintain compatibility and consistency across the entire code base.
    - must return normally from this function (don't call board_init_r()
      directly)

Here the BSS is cleared. For xPL, if CONFIG_xPL_STACK_R is defined, then at
this point the stack and global_data are relocated to below
CONFIG_xPL_STACK_R_ADDR. For non-xPL, U-Boot is relocated to run at the top of
memory.

board_init_r()
~~~~~~~~~~~~~~

    - purpose: main execution, common code
    - global_data is available
    - SDRAM is available
    - BSS is available, all static/global variables can be used
    - execution eventually continues to main_loop()

Non-xPL-specific notes:

    - U-Boot is relocated to the top of memory and is now running from
      there.

xPL-specific notes:

    - stack is optionally in SDRAM, if CONFIG_xPL_STACK_R is defined
