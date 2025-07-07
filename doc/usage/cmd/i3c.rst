.. SPDX-License-Identifier: GPL-2.0

.. index::
   single: i3c (command)

i3c command
===========

Synopsis
--------

::

    i3c <host_controller>
    i3c current
    i3c list
    i3c device_list
    i3c write <mem_addr> <length> <device_number>
    i3c read <mem_addr> <length> <device_number>

Description
-----------

The ``i3c`` command is used to probe the i3c host controller and perform
read and write operations on the connected i3c devices.

i3c current
------------

Display the currently selected i3c host controller.

i3c list
---------

List all the i3c hosts defined in the device-tree.

i3c device_list
----------------

List all the i3c devices' device number, static address, dynamic address,
PID, BCR, DCR, Max Write Speed, and Max Read Speed of the probed i3c host
controller.

i3c write
----------

Perform a write operation from memory to the connected i3c device. The data
is read from a specified memory address and written to the selected i3c
device, which is identified by its device number.

You need to provide the memory address (``mem_addr``), the length of data
to be written (``length``), and the device number (``device_number``). The
data in memory will be transferred to the device in the specified order.

i3c read
---------

Perform a read operation from the connected i3c device to memory. The data
is read from the selected i3c device and stored at the specified memory
address. You need to provide the memory address (``mem_addr``), the length
of data to be read (``length``), and the device number (``device_number``).

The device will send the requested data, which is then written to the memory
location you specified. This operation allows you to retrieve information
from the device and use it in your application. The data is read in the
order specified and can be multiple bytes.

host_controller
    The name of the i3c host controller defined in the device-tree.

length
    The size of the data to be read or written.

device_number
    The device number in the driver model of the device connected to the i3c
    host controller.

mem_addr
    The start address in memory from which to read or write the data.

Examples
--------

Probe the ``i3c0`` controller::

    => i3c i3c0

Display the current i3c host controller::

    => i3c current

Check the device number and PID of the connected devices::

    => i3c device_list

Perform write operations on the connected i3c device (device 0) from memory::

    => i3c write 0x1000 4 0

    This command reads 4 bytes of data from memory starting at address
    ``0x1000`` and writes them to device 0, which is identified by its device
    number in the driver model. Example data from memory could look like this:

    ```
    Data at 0x1000: 0xAA 0xBB 0xCC 0xDD
    ```

    The bytes `0xAA`, `0xBB`, `0xCC`, and `0xDD` will be written to device 0.

Perform a read operation from device 0 to memory (multiple bytes)::

    => i3c read 0x1000 4 0

    This command reads 4 bytes of data from device 0 and writes them to
    memory starting at address ``0x1000``.

    Example output after reading 4 bytes from device 0:

    ```
    i3c Read:
    00000000  AA BB CC DD
    ```

    The bytes `0xAA`, `0xBB`, `0xCC`, and `0xDD` are read from device 0
    and written to memory at address `0x1000`.

Configuration
-------------

The ``i3c`` command is only available if CONFIG_CMD_I3C=y.

Return value
------------

If the command succeeds, the return value ``$?`` is set to 0. If an error
occurs, the return value ``$?`` is set to 1.

Note
----

When specifying the data to be written to the i3c device (for example, with
the ``i3c write`` command), the data can be provided in either uppercase
or lowercase hexadecimal format. Both are valid and will be processed
correctly. Similarly, when reading data with ``i3c read``, the data will be
retrieved in the specified length and can include multiple bytes, all
formatted in the same way.