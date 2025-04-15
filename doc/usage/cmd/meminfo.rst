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
which is free for use by images. In architectures that support it, it also prints
the mapped pages and their permissions. The latter is architecture specific.

The layout of memory is set up before relocation, within the init sequence in
``board_init_f()``, specifically the various ``reserve_...()`` functions. This
'reservation' of memory starts from the top of RAM and proceeds downwards,
ending with the stack. This results in the maximum possible amount of memory
being left free for image-loading.

The meminfo command writes the DRAM size. If the architecture also supports it,
page table entries will be shown next. Finally the rest of the outputs are
printed in 5 columns:

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

Aarch64 specific flags
----------------------

More information on the output can be found
Chapter D8 - The AArch64 Virtual Memory System Architecture at
https://developer.arm.com/documentation/ddi0487/latest/

In short, for a stage 1 translation regime the following apply:

* RWX: Pages mapped with Read, Write and Execute permissions
* RO:  Pages mapped with Read-Only permissions
* PXN: PXN (Privileged Execute Never) applies to execution at EL1 and above
* UXN: UXN (Unprivileged Execute Never) applies to EL0

Example
-------

This example shows output with both ``CONFIG_CMD_MEMINFO`` and
``CONFIG_CMD_MEMINFO_MAP`` enabled for aarch64 qemu::

    DRAM:  8 GiB
    Walking pagetable at 000000023ffe0000, va_bits: 40. Using 4 levels
    [0x0000023ffe1000]                          |  Table |            |               |
      [0x0000023ffe2000]                        |  Table |            |               |
        [0x00000000000000 - 0x00000008000000]   |  Block | RWX        | Normal        | Inner-shareable
        [0x00000008000000 - 0x00000040000000]   |  Block | PXN UXN    | Device-nGnRnE | Non-shareable
      [0x00000040000000 - 0x00000200000000]     |  Block | RWX        | Normal        | Inner-shareable
      [0x0000023ffea000]                        |  Table |            |               |
        [0x00000200000000 - 0x0000023f600000]   |  Block | RWX        | Normal        | Inner-shareable
        [0x0000023ffeb000]                      |  Table |            |               |
          [0x0000023f600000 - 0x0000023f68c000] |  Pages | RWX        | Normal        | Inner-shareable
          [0x0000023f68c000 - 0x0000023f74f000] |  Pages | RO         | Normal        | Inner-shareable
          [0x0000023f74f000 - 0x0000023f794000] |  Pages | PXN UXN RO | Normal        | Inner-shareable
          [0x0000023f794000 - 0x0000023f79d000] |  Pages | PXN UXN    | Normal        | Inner-shareable
          [0x0000023f79d000 - 0x0000023f800000] |  Pages | RWX        | Normal        | Inner-shareable
        [0x0000023f800000 - 0x00000240000000]   |  Block | RWX        | Normal        | Inner-shareable
      [0x00000240000000 - 0x00004000000000]     |  Block | RWX        | Normal        | Inner-shareable
      [0x0000023ffe3000]                        |  Table |            |               |
        [0x00004010000000 - 0x00004020000000]   |  Block | PXN UXN    | Device-nGnRnE | Non-shareable
    [0x0000023ffe4000]                          |  Table |            |               |
      [0x00008000000000 - 0x00010000000000]     |  Block | PXN UXN    | Device-nGnRnE | Non-shareable

    Region           Base     Size      End      Gap
    ------------------------------------------------
    video        23f7e0000   800000 23ffe0000
    code         23f68a000   156000 23f7e0000        0
    malloc       23e64a000  1040000 23f68a000        0
    board_info   23e649f80       78 23e649ff8        8
    global_data  23e649df0      188 23e649f78        8
    devicetree   23e549df0   100000 23e649df0        0
    bloblist     23e547000     2000 23e549000      df0
    stack        23d546ff0  1000000 23e546ff0       10
    lmb          23d546ff0        0 23d546ff0        0
    lmb          23d543000     3ff0 23d546ff0        0
    free         40000000 23d543000 27d543000 ffffffffc0000000

Return value
------------

The return value $? is always 0 (true).
