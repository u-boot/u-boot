.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: meminfo (command)

meminfo command
===============

Synopsis
--------

::

    meminfo

Description
-----------

The meminfo command shows the amount of memory. If ``CONFIG_CMD_MEMINFO_MAP`` is
enabled, then it also shows the layout of memory used by U-Boot and the region
which is free for use by images.

The layout of memory is set up before relocation, within the init sequence in
``board_init_f()``, specifically the various ``reserve_...()`` functions. This
'reservation' of memory starts from the top of RAM and proceeds downwards,
ending with the stack. This results in the maximum possible amount of memory
being left free for image-loading.

The meminfo command writes the DRAM size, then the rest of its outputs in 5
columns:

Region
   Name of the region

Base
    Base address of the region, i.e. where it starts in memory

Size
    Size of the region, which may be a little smaller than the actual size
    reserved, e.g. due to alignment

End
    End of the region. The last byte of the region is one lower than the address
    shown here

Gap
    Gap between the end of this region and the base of the one above

Regions shown are:

video
    Memory reserved for video framebuffers. This reservation happens in the
    bind() methods of all video drivers which are present before relocation,
    so the size depends on that maximum amount of memory which all such drivers
    want to reserve. This may be significantly greater than the amount actually
    needed, if the display is ultimately set to a smaller resolution or colour
    depth than the maximum supported.

code
    U-Boot's code and Block-Starting Symbol (BSS) region. Before relocation,
    U-Boot copies its code to a high region and sets up a BSS immediately after
    that. The size of this region is generally therefore ``__bss_end`` -
    ``__image_copy_start``

malloc
    Contains the malloc() heap. The size of this is set by
    ``CONFIG_SYS_MALLOC_LEN``.

board_info
    Contains the ``bd_info`` structure, with some information about the current
    board.

global_data
    Contains the global-data structure, pointed to by ``gd``. This includes
    various pointers, values and flags which control U-Boot.

devicetree
    Contains the flatted devicetree blob (FDT) being used by U-Boot to configure
    itself and its devices.

bootstage
    Contains the bootstage records, which keep track of boot time as U-Boot
    executes. The size of this is determined by
    ``CONFIG_BOOTSTAGE_RECORD_COUNT``, with each record taking approximately
    32 bytes.

bloblist
    Contains the bloblist, which is a list of tables and other data created by
    U-Boot while executed. The size of this is determined by
    ``CONFIG_BLOBLIST_SIZE``.

stack
    Contains U-Boot's stack, growing downwards from the top. The nominal size of
    this region is set by ``CONFIG_STACK_SIZE`` but there is no actual limit
    enforced, so the stack can grow behind that. Images should be loaded lower
    in memory to avoid any conflict.

free
    Free memory, which is available for loading images. The base address of
    this is ``gd->ram_base`` which is generally set by ``CFG_SYS_SDRAM_BASE``.

Example
-------

This example shows output with both ``CONFIG_CMD_MEMINFO`` and
``CONFIG_CMD_MEMINFO_MAP`` enabled::

    => meminfo
    DRAM:  256 MiB

    Region           Base     Size      End      Gap
    ------------------------------------------------
    video         f000000  1000000 10000000
    code          ec3a000   3c5d28  efffd28      2d8
    malloc        8c38000  6002000  ec3a000        0
    board_info    8c37f90       68  8c37ff8        8
    global_data   8c37d80      208  8c37f88        8
    devicetree    8c33000     4d7d  8c37d7d        3
    bootstage     8c32c20      3c8  8c32fe8       18
    bloblist      8c32000      400  8c32400      820
    stack         7c31ff0  1000000  8c31ff0       10
    free                0  7c31ff0  7c31ff0        0


Return value
------------

The return value $? is always 0 (true).
