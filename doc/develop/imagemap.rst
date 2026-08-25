.. SPDX-License-Identifier: GPL-2.0+

On-Demand Image Loading from Storage (imagemap)
================================================

This document describes the ``imagemap`` framework that enables
loading FIT images directly from storage devices without first
copying the entire image into RAM.

Architecture Overview
---------------------

The framework is built around the ``UCLASS_IMAGEMAP`` device class
(``include/imagemap.h``), which provides:

``imagemap_create(dev, name, part)``
   Create an imagemap device over a partition of block device *dev*,
   selected by *name* or by index *part*.

``imagemap_map(dev, offset, size)``
   Return a pointer to a buffer holding the requested region.
   Consecutive calls that fall within the same region reuse the
   existing mapping, so sub-image header look-ups are essentially
   free.

``imagemap_map_to(dev, offset, size, dst_addr)``
   Like ``imagemap_map()`` but place the data at a specific physical
   address (``dst_addr``).  This is used by ``fit_image_load()`` to
   stream sub-images directly into their final load addresses.

Translation Table
~~~~~~~~~~~~~~~~~

Every ``imagemap`` device maintains an internal translation
table that records which byte ranges of the storage image have been
read into memory and where they reside.  When ``imagemap_map()`` is
called:

1. The table is checked for an existing mapping that covers the
   requested range.
2. If found, the existing buffer pointer (adjusted for the offset
   within the region) is returned immediately.
3. If not found, a buffer is allocated via the LMB allocator, the
   data is read from storage, and a new entry is added to the table.

Reuse of already-loaded regions (steps 1 and 2) is what lets a header
probe, signature verification and the final load of the same range share
a single read; the FIT flow relies on it to hand already-read data back
through ``fit_image_get_data()``.  The table also serves as the registry
of LMB allocations that ``imagemap_cleanup()`` frees.

``imagemap_cleanup()`` frees all allocated buffers and resets
the table.

Storage Access
--------------

``imagemap_create()`` reads from a partition on a block device (MMC,
SCSI, NVMe, ...), selected by name or by index.  Reads go through the
same ``struct spl_load_info`` load-to-mem abstraction that SPL uses, so
a byte range is fetched with native sector alignment via
``spl_load_region()``.

MTD partitions and UBI volumes are read the same way, by way of the
existing block layers that expose them:

- with ``CONFIG_MTD_BLOCK`` a non-NAND MTD (parallel NOR, SPI-NOR, ...)
  is automatically given a ``mtd_blk`` block device, and each of its
  partitions appears as a partition on it;
- with ``CONFIG_UBI_BLOCK`` a UBI volume appears as a partition on the
  ``ubi_blk`` block device (the block descriptor's ``hwpart`` selects
  the volume).

NAND bad-block handling and wear-levelling therefore stay with UBI, as
they do for the rest of U-Boot; on NOR a partition is read linearly and
end-to-end integrity is provided by the FIT hash or signature, so no
MTD-specific read path is needed in imagemap.

Boot Integration
----------------

The ``imagemap`` framework integrates with the boot pipeline through
``bootm_run_states()`` in ``boot/bootm.c``:

1. The imagemap device pointer is stored in ``struct bootm_info``
   and propagated to the global ``images`` structure.

2. ``boot_get_kernel()`` uses ``imagemap_map()`` to read the image
   header and FIT metadata without loading the full image into RAM.

3. ``fit_image_load()`` uses the on-demand storage path via
   ``images->imagemap`` to stream individual sub-images directly to
   their final load addresses.

4. Before jumping to the OS, ``imagemap_cleanup()`` releases all
   allocated resources.

The ``bootmeth_openwrt`` boot method is the primary consumer of this
framework, using it to boot OpenWrt-style FIT firmware images stored
directly on raw storage.
