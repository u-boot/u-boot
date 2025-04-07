.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: mv (command)

mv command
==========

Synopsis
--------

::

    mv <interface> [<dev[:part]>] <old_path> <new_path>

Description
-----------

The mv command renames/moves a file or directory within a filesystem.

interface
    interface for accessing the block device (mmc, sata, scsi, usb, ....)

dev
    device number

part
    partition number, defaults to 0 (whole device)

old_path
   existing path to file/directory

new_path
   new path/name for the rename/move


Example
-------

    # Rename file 'foo' in directory 'dir' to 'bar'
    mv mmc 0:0 dir/foo dir/bar

    # Move file 'f' from directory 'foo' to existing directory 'bar' renaming
    # 'f' to 'g'
    mv mmc 0:0 foo/f bar/g

    # Move directory 'abc' in directory 'dir1' into existing directory 'dir2'
    mv mmc 0:0 dir1/abc dir2

Configuration
-------------

The mv command is only available if CONFIG_CMD_FS_GENERIC=y.

Return value
------------

The return value $? is set to 0 (true) if the file was successfully
renamed/moved.

If an error occurs, the return value $? is set to 1 (false).
