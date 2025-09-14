.. SPDX-License-Identifier: GPL-2.0+

Ext4 File System
================

Overview
--------

U-Boot supports access of both ext2 and ext4 filesystems, either in read-only
mode or in read-write mode.

Configuration
-------------

First, to enable support for both ext4 (and, automatically, ext2 as well),
but without selecting the corresponding commands, enable one of the following:

::

  CONFIG_FS_EXT4	(for read-only)
  CONFIG_EXT4_WRITE	(for read-write)

Next, to select the ext2-related commands:

  * ext2ls
  * ext2load

or ext4-related commands:

  * ext4size
  * ext4ls
  * ext4load

use one or both of:

::

	CONFIG_CMD_EXT2
	CONFIG_CMD_EXT4

Selecting either of the above automatically selects CONFIG_FS_EXT4 if it
wasn't enabled already.

In addition, to get the write access command ``ext4write``, enable:

::

	CONFIG_CMD_EXT4_WRITE

which automatically selects CONFIG_EXT4_WRITE if it wasn't defined
already.

Also relevant are the generic filesystem commands, selected by:

::

	CONFIG_CMD_FS_GENERIC

This does not automatically enable EXT4 support for you, you still need
to do that yourself.

Lastly, the current u-boot implementation for Ext4 write requires a lot
of memory to run successfully. The following enable support for
large Ext4 partitions:

::

	CONFIG_EXT4_MAX_JOURNAL_ENTRIES
	CONFIG_SYS_MALLOC_LEN

The number of journal entries and dynamic memory allocation are proportional
to the partition capacity. For example, an ext4 4TB HDD partition could
require approximately 500 entries and more than 128 MB heap space.

Examples
--------

Some sample commands to test ext4 support:

1. Check that the ext4 commands can be seen in the output of U-Boot help:

::

	=> help
	...
	ext4load- load binary file from a Ext4 file system
	ext4ls  - list files in a directory (default /)
	ext4size - determine a file's size
	ext4write- create a file in ext4 formatted partition
	...

2. The ``ext4ls`` command can be used to list the files in an ext4-formatted partition:

::

	ext4ls <interface> <dev[:part]> [directory]

For example, to list files in ext4-formatted partition directory /usr/lib:

::

	=> ext4ls mmc 0:5 /usr/lib

3. The ``ext4load`` command can be used to read and load a file from an
ext4-formatted partition to RAM:

::

	ext4load <interface> [<dev[:part]> [addr [filename [bytes [pos]]]]]

For example, to load file /uImage from an ext4-formatted partition:

::

	=> ext4load mmc 2:2 0x30007fc0 uImage

4. The ``ext4write`` command can be used to write to an ext4 partition:

::

	ext4write <interface> <dev[:part]> <addr> <absolute filename path> [sizebytes] [file offset]

For example, to write a file loaded at 0x8200000 of size 256 bytes to an
ext4-formatted partition with the filename ``/boot/sample_file.hex``:

::

	=> ext4write mmc 2:2 0x82000000 /boot/sample_file.hex 0x100
	256 bytes written in 138 ms (1000 Bytes/s)


References
----------

	* ext4 implementation in Linux Kernel
	* Uboot existing ext2 load and ls implementation
	* Journaling block device JBD2 implementation in linux Kernel
