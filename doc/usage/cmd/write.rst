.. SPDX-License-Identifier: GPL-2.0-or-later

.. index::
   single: write (command)

write command
=============

Synopsis
--------

::

    write <interface> <dev[:part|#partname]> <addr> <blk#> <cnt>

Description
-----------

The write command can be used for raw writing data to a block device
(or partition therein), i.e. without going through a file system.

The block device is specified using the <interface> (e.g. "mmc") and
<dev> parameters. If the block device has a partition table, one can
optionally specify a partition number (using the :part syntax) or
partition name (using the #partname syntax). The command then reads
the <cnt> blocks of data starting at block number <blk#> of the given
device/partition to the memory address <addr>.

Examples
--------

.. code-block:: bash

    # Write to the third sector of the partition named 'bootdata' of mmc device 0
    write mmc 0#bootdata $loadaddr 2 1

Configuration
-------------

The write command is only available if CONFIG_CMD_WRITE=y.

Return value
------------

The command sets the return value $? to 0 (true) on success and to 1 (false) in
case of an error.
