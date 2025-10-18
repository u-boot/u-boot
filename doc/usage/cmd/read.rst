.. SPDX-License-Identifier: GPL-2.0-or-later

.. index::
   single: read (command)

read command
============

Synopsis
--------

::

    read <interface> <dev[:part|#partname]> <addr> <blk#> <cnt>

Description
-----------

The read command can be used for raw reading data from a block device
(or a partition therein), i.e. without going through a file system.

The block device is specified using the <interface> (e.g. "mmc") and
<dev> parameters. If the block device has a partition table, one can
optionally specify a partition number (using the :part syntax) or
partition name (using the #partname syntax). The command then reads
the <cnt> blocks of data starting at block number <blk#> of the given
device/partition to the memory address <addr>.

Examples
--------

.. code-block:: bash

    # Read 2 MiB from partition 3 of mmc device 2 to $loadaddr
    read mmc 2.3 $loadaddr 0 0x1000

    # Read 16 MiB from the partition named 'kernel' of mmc device 1 to $loadaddr
    read mmc 1#kernel $loadaddr 0 0x8000

Configuration
-------------

The read command is only available if CONFIG_CMD_READ=y.

Return value
------------

The command sets the return value $? to 0 (true) on success or to 1 (false) in
case of an error.
