.. SPDX-License-Identifier: GPL-2.0+

Logical Memory Blocks (LMB)
===========================

U-Boot has support for reserving chunks of memory which is primarily
used for loading images to the DRAM memory, before these are booted,
or written to non-volatile storage medium. This functionality is
provided through the Logical Memory Blocks (LMB) module.

Introduction
------------

The LMB module manages allocation requests for memory region not
occupied by the U-Boot image. Allocation requests that are made
through malloc() and similar functions result in memory getting
allocated from the heap region, which is part of the U-Boot
image. Typically, the heap memory is a few MiB in size. Loading an
image like the linux kernel might require lot more memory than what
the heap can provide. Such allocations are usually handled through the
LMB module.

The U-Boot image typically gets relocated to the top of the usable
DRAM memory region. A typical memory layout looks as follows::





                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
       ---            +--------------+  <--- U-Boot ram top
        |             |              |
        |             |    Text      |
        |             +--------------+
        |             |              |
        |             |    Data      |
        |             +--------------+
        |             |              |
        |             |    BSS       |
  U-Boot Image        +--------------+
        |             |              |
        |             |    Heap      |
        |             |              |
        |             +--------------+
        |             |              |
        |             |              |
        |             |    Stack     |
        |             |              |
        |             |              |
       ---            +--------------+
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      |              |
                      +--------------+  <--- ram start



The region of memory below the U-Boot image is the one controlled by
the LMB module.


Types of LMB Allocations
------------------------

There are two classes of allocation requests that get made to the LMB
module. One type of allocation requests are requesting memory of a
particular number of bytes. This type of allocation is similar to that
done using the malloc type of function calls. The other type of
allocations, are requests made for a specific memory address. The
second type of allocations are usually made for loading images to a
particular memory address.


LMB design Pre 2025.01
----------------------

The earlier versions of U-Boot (pre 2025.01 release)
had a local memory map based LMB implementation whereby it was
possible to declare the LMB map inside a function or a C file. This
design resulted in temporary, non-global LMB maps, which also allowed
for re-use of memory. This meant that it was possible to use a region
of memory to load some image, and subsequently the same region of
memory could be used for loading a different image. A typical example
of this usage would be loading an image to a memory address, followed
by writing that image to some non-volatile storage medium. Once this
is done, the same address can be used for loading a different image
and then writing it to it's non-volatile storage
destination. Typically, environment variables like `loadaddr`,
`kernel_addr_r`, `ramdisk_addr_r` are used for loading images to
memory regions.


Current LMB implementation
--------------------------

Changes were made in the 2025.01 release to make the LMB memory map
global and persistent. With this, the LMB memory map is the same
across all of U-Boot, and also persists as long as U-Boot is
active. Even with this change, there has been consistency as far as
re-use of memory is concerned to maintain backward compatibility. It
is allowed for re-requesting the same region of memory if the memory
region has a particular attribute (LMB_NONE).

As part of the platform boot, DRAM memory available for use in U-Boot
gets added to the LMB memory map. Any allocation requests made
subsequently will be made from this memory added as part of the board
init.


Allocation API
--------------

Any request for non-heap memory can be made through the LMB allocation
API.

.. code-block:: c

	int lmb_alloc_mem(enum lmb_mem_type type, u64 align,
			  phys_addr_t *addr, phys_size_t size,
			  u32 flags);

Correspondingly, the allocated memory can be free'd

.. code-block:: c

	long lmb_free(phys_addr_t base, phys_size_t size, u32 flags);

For a detailed API description, please refer to the header file.


UEFI allocations with LMB as the backend
----------------------------------------

The UEFI specification describes boot-time API's for allocation of
memory. These API's use the same memory that is being used by the LMB
module. Pre 2025.01 release, there wasn't any synchronisation between
the EFI sub-system and the LMB module about the memory that was
getting allocated by each of these modules. This was the primary
reason for making the LMB memory map global and persistent. With this
change, the EFI memory allocation API's have also been changed to use
the LMB module as the backend for the allocation requests. Any other
sub-system which might wish to use the same memory region for it's use
can then use the LMB as the backend for the memory allocations and
it's associated book-keeping.


API documentation
-----------------

.. kernel-doc:: include/lmb.h

