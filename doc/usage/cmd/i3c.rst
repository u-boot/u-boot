.. SPDX-License-Identifier: GPL-2.0

.. index::
   single: i3c (command)

i3c command
===========

Synopsis
--------

::

    i3c <i3c name>
    i3c current
    i3c list
    i3c device_list
    i3c write <data> <length> <device_number>
    i3c read <length> <device_number>


Description
-----------

The ``i3c`` command is used to probe i3c master controller and perform read
and write on the i3c devices.

i3c current
------------

* Display the current probed i3c controller.

i3c list
----------

List all the i3c controllers defined in the DT.

i3c device_list
----------------

List all the i3c devices' device number, dynamic address, PID, BCR, DCR,
Max Write Speed and Max Read Speed of the probed i3c controller.

i3c write
-----------

Perform write to the i3c device.

i3c read
-----------

Perform read from the i3c device.

i3c name
    i3c name defined in DT

length
    Size of the data.

device_number
    device number that connected to i3c controller.

data
    Bytes to be written to device.

Examples
--------

Probe i3c0::

    => i3c i3c0

Check cuurent i3c controller::

    => i3c current

Check the device number and PID of the connected devices::

    => i3c device_list

Perform write AA byte to a device 0::

    => i3c write AA 1 0

Perform read from a device 0::

    => i3c read 1 0

Configuration
-------------

The ``i3c`` command is only available if CONFIG_CMD_I3C=y.

Return value
------------

If the command succeeds, the return value ``$?`` is set to 0. If an error occurs, the
return value ``$?`` is set to 1.
