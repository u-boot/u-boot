.. SPDX-License-Identifier: GPL-2.0+

Arm Versatile Express
=====================

The vexpress_* board configuration supports the following platforms:

 * FVP_Base_RevC-2xAEMvA
 * FVP_BaseR_AEMv8R
 * Juno development board

Fixed Virtual Platforms
-----------------------

The Fixed Virtual Platforms (FVP) are complete simulations of an Arm system,
including processor, memory and peripherals. They are set out in a "programmer's
view", which gives a comprehensive model on which to build and test software.

The supported FVPs are available free of charge and can be downloaded from the
Arm developer site [1]_ (user registration might be required).

Supported features:

 * GICv3
 * Generic timer
 * PL011 UART
 * SMC91111 network interface

The default configuration assumes that U-Boot is boostrapped from the start of
the DRAM (address 0x80000000 for AEMvA; 0x00000000 for AEMv8R) using a suitable
bootloader. Alternatively, U-Boot can be launched directly by mapping the binary
to the same address (using the FVP's --data argument).

The FVPs can be debugged using Arm Development Studio [2]_.

FVP_BaseR
^^^^^^^^^

On Armv8r64 platforms (such as the FVP_BaseR), U-Boot runs at S-EL2, so
CONFIG_ARMV8_SWITCH_TO_EL1 is defined so that the next stage boots at S-EL1. If
S-EL2 is desired instead, the *armv8_switch_to_el1* environment variable is
available. This can be set to *n* to override the config flag and boot the next
stage at S-EL2 instead.

Juno
----

The Juno development board is an open, vendor-neutral Armv8-A development
platform that supports an out-of-the-box Linux software package. A range of
plug-in expansion options enables hardware and software applications to be
developed and debugged.

References
----------

.. [1] https://developer.arm.com/tools-and-software/simulation-models/fixed-virtual-platforms
.. [2] https://developer.arm.com/tools-and-software/embedded/arm-development-studio
