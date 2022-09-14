.. SPDX-License-Identifier: GPL-2.0+
.. (C) Copyright 2022, Masahisa Kojima <masahisa.kojima@linaro.org>

eficonfig command
=================

Synopsis
--------
::

    eficonfig

Description
-----------

The "eficonfig" command uses U-Boot menu interface and provides
a menu-driven UEFI variable maintenance feature.
The "eficonfig" has the following menu entries.

Add Boot Option
    Add new UEFI Boot Option.
    User can edit description, file path, and optional_data.

Edit Boot Option
    Edit the existing UEFI Boot Option
    User can edit description, file path, and optional_data.

Change Boot Order
    Change the order of UEFI BootOrder variable.

Delete Boot Option
    Delete the UEFI Boot Option

Configuration
-------------

The "eficonfig" command is enabled by::

    CONFIG_CMD_EFICONFIG=y

If CONFIG_BOOTMENU_DISABLE_UBOOT_CONSOLE is enabled, user can not enter
U-Boot console. In this case, bootmenu can be used to invoke "eficonfig"::

    CONFIG_USE_PREBOOT=y
    CONFIG_PREBOOT="setenv bootmenu_0 UEFI Maintenance Menu=eficonfig"

How to boot the system with newly added UEFI Boot Option
''''''''''''''''''''''''''''''''''''''''''''''''''''''''

"eficonfig" command is responsible for configuring the UEFI variables,
not directly handle the system boot.
The new Boot Option added by "eficonfig" is appended at the last entry
of UEFI BootOrder variable, user may want to change the boot order
through "Change Boot Order".
If the bootmenu is enabled, CONFIG_BOOTMENU_DISABLE_UBOOT_CONSOLE is enabled,
and "eficonfig" is configured as preboot command, the newly added Boot Options
are enumerated in the bootmenu when user exits from the eficonfig menu.
User may select the entry in the bootmenu to boot the system, or follow
the U-Boot configuration the system already has.

Auto boot with the UEFI Boot Option
'''''''''''''''''''''''''''''''''''

To do auto boot according to the UEFI BootOrder variable,
add "bootefi bootmgr" entry as a default or first bootmenu entry::

    CONFIG_PREBOOT="setenv bootmenu_0 UEFI Boot Manager=bootefi bootmgr; setenv bootmenu_1 UEFI Maintenance Menu=eficonfig"

See also
--------
* :doc:`bootmenu<bootmenu>` provides a simple mechanism for creating menus with different boot items
