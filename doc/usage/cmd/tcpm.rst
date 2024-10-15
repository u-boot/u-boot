.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: tcpm (command)

tcpm command
============

Synopsis
--------

::

    tcpm dev [devnum]
    tcpm info
    tcpm list

Description
-----------

The tcpm command is used to control USB-PD controllers, also known as TypeC Port Manager (TCPM).

The 'tcpm dev' command shows or set current TCPM device.

    devnum
        device number to change

The 'tcpm info' command displays the current state of the device

The 'tcpm list' command displays the list available devices.

Examples
--------

The 'tcpm info' command displays device's status:
::

    => tcpm info
    Orientation: normal
    PD Revision: rev3
    Power Role:  sink
    Data Role:   device
    Voltage:     20.000 V
    Current:      2.250 A

The current device can be shown or set via 'tcpm dev' command:
::

    => tcpm dev
    TCPM device is not set!
    => tcpm dev 0
    dev: 0 @ usb-typec@22
    => tcpm dev
    dev: 0 @ usb-typec@22

The list of available devices can be shown via 'tcpm list' command:
::

    => tcpm list
    | ID | Name                            | Parent name         | Parent uclass @ seq
    |  0 | usb-typec@22                    | i2c@feac0000        | i2c @ 4 | status: 0

Configuration
-------------

The tcpm command is only available if CONFIG_CMD_TCPM=y.
