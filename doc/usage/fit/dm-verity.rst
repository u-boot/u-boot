.. SPDX-License-Identifier: GPL-2.0+

FIT dm-verity Boot Parameters
=============================

Introduction
------------

Linux's dm-verity device-mapper target provides transparent integrity
checking of block devices using a Merkle tree. It is commonly used to
protect read-only root filesystems such as SquashFS images.

When a FIT image packages the root filesystem as a loadable sub-image of
type ``filesystem`` (``IH_TYPE_FILESYSTEM``), the verity metadata can be
stored alongside the image data in a ``dm-verity`` subnode. U-Boot reads
this metadata at boot time and generates the kernel command-line parameters
that Linux needs to activate the verity target, eliminating the need for
an initramfs or userspace helper to set up dm-verity.

This feature is enabled by ``CONFIG_FIT_VERITY`` (see ``boot/Kconfig``).

Prerequisites
-------------

* **Linux uImage.FIT block driver** – the kernel must include the FIT block
  driver that exposes loadable sub-images as ``/dev/fit0``, ``/dev/fit1``,
  etc. The driver assigns device numbers in the order loadables appear in
  the FIT configuration.

* **dm-verity support in the kernel** – ``CONFIG_DM_VERITY`` must be
  enabled so the kernel can process the ``dm-mod.create=`` parameter.

* **CONFIG_FIT_VERITY** enabled in U-Boot.

How it works
------------

The implementation is split into a **build** phase and an **apply** phase,
both of which run automatically within the ``bootm`` state machine. No boot
method needs to call verity functions explicitly.

**Build phase** (``BOOTM_STATE_FINDOTHER`` → ``boot_get_loadable()``)

1. After all loadable sub-images have been loaded,
   ``fit_verity_build_cmdline()`` iterates the configuration's
   ``loadables`` list.

2. For each loadable that is an ``IH_TYPE_FILESYSTEM`` image **and**
   contains a ``dm-verity`` child node, a dm-verity target specification is
   built by the helper ``fit_verity_build_target()``.

3. The dm-verity target references ``/dev/fitN``, where *N* is the
   zero-based index of the loadable in the configuration. This matches the
   numbering used by the Linux FIT block driver.

4. The resulting fragments are stored in ``struct bootm_headers``:

   ``images->dm_mod_create``
     The full dm-verity target table. Multiple targets are separated by ``;``.

   ``images->dm_mod_waitfor``
     Comma-separated list of ``/dev/fitN`` devices so the kernel waits for
     the underlying FIT block devices to appear before activating
     device-mapper.

**Apply phase** (``BOOTM_STATE_OS_PREP``)

5. Just before ``bootm_process_cmdline_env()`` processes the ``bootargs``
   environment variable, ``fit_verity_apply_bootargs()`` appends the
   ``dm-mod.create=`` and ``dm-mod.waitfor=`` parameters.

**Bootmeth integration**

  Because the fragments are stored in ``struct bootm_headers``, a boot
  method can check ``fit_verity_active(images)`` between bootm state
  invocations. A typical pattern splits ``bootm_run_states()`` into two
  calls -- one for ``START|FINDOS|FINDOTHER|LOADOS`` and one for
  ``OS_PREP|OS_GO`` -- and inspects ``fit_verity_active()`` in
  between to decide whether to add a ``root=`` parameter pointing at the
  dm-verity device.

FIT image source (.its) example
-------------------------------

Below is a minimal ``.its`` file showing a kernel and a dm-verity-protected
root filesystem packaged as a FIT. Only the three user-provided properties
(``algo``, ``data-block-size``, ``hash-block-size``) are included; ``mkimage``
computes and fills in ``digest``, ``salt``, ``num-data-blocks``, and
``hash-start-block`` automatically (see `Generating verity metadata`_ below)::

    /dts-v1/;

    / {
        description = "Kernel + dm-verity rootfs";
        #address-cells = <1>;

        images {
            kernel {
                description = "Linux kernel";
                data = /incbin/("./Image.gz");
                type = "kernel";
                arch = "arm64";
                os = "linux";
                compression = "gzip";
                load = <0x44000000>;
                entry = <0x44000000>;
                hash-1 {
                    algo = "sha256";
                };
            };

            fdt {
                description = "Device tree blob";
                data = /incbin/("./board.dtb");
                type = "flat_dt";
                arch = "arm64";
                compression = "none";
                hash-1 {
                    algo = "sha256";
                };
            };

            rootfs {
                description = "SquashFS root filesystem";
                data = /incbin/("./rootfs.squashfs");
                type = "filesystem";
                arch = "arm64";
                compression = "none";
                hash-1 {
                    algo = "sha256";
                };

                dm-verity {
                    algo = "sha256";
                    data-block-size = <4096>;
                    hash-block-size = <4096>;
                };
            };
        };

        configurations {
            default = "config-1";
            config-1 {
                description = "Boot with dm-verity rootfs";
                kernel = "kernel";
                fdt = "fdt";
                loadables = "rootfs";
            };
        };
    };

With this configuration U-Boot produces a kernel command line similar to::

    dm-mod.create="rootfs,,, ro,0 <data_sectors> verity 1 \
        /dev/fit0 /dev/fit0 4096 4096 3762 3762 sha256 \
        8e6791637f93cbb81fc45299e203cbe85ca2e47a38f5051bddeece92d7b1c9f9 \
        aa7b11f8db8fe2e5bfd4eca1d18a22b5de7ea39d2e1b93bb7272ce0c6ca3cc8e" \
    dm-mod.waitfor=/dev/fit0

dm-verity subnode properties
----------------------------

User-provided properties (required in the ``.its``):

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - Property
     - Type
     - Description
   * - ``algo``
     - string
     - Hash algorithm name, e.g. ``"sha256"``.
   * - ``data-block-size``
     - u32
     - Data block size in bytes (>= 512, typically 4096).
   * - ``hash-block-size``
     - u32
     - Hash block size in bytes (>= 512, typically 4096).

Computed properties (filled in by ``mkimage``):

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - Property
     - Type
     - Description
   * - ``num-data-blocks``
     - u32
     - Number of data blocks in the filesystem image (computed from the
       image size and ``data-block-size``).
   * - ``hash-start-block``
     - u32
     - Offset in ``hash-block-size``-sized blocks from the start of the
       sub-image to the root block of the hash tree.
   * - ``digest``
     - byte array
     - Root hash of the Merkle tree, stored as raw bytes. Length must match
       the output size of ``algo``.
   * - ``salt``
     - byte array
     - Salt used when computing the Merkle tree, stored as raw bytes.

These values are the same ones produced by ``veritysetup format`` and can
typically be obtained from its output.
The ``digest`` and ``salt`` byte arrays correspond to the hex-encoded
``Root hash`` and ``Salt`` printed by ``veritysetup format``.

Optional boolean properties (when present, they are collected and appended
as dm-verity optional parameters with hyphens converted to underscores):

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Property
     - Description
   * - ``restart-on-corruption``
     - Restart the system on data corruption.
   * - ``panic-on-corruption``
     - Panic the system on data corruption.
   * - ``restart-on-error``
     - Restart the system on I/O error.
   * - ``panic-on-error``
     - Panic the system on I/O error.
   * - ``check-at-most-once``
     - Verify data blocks only on first read.


Generating verity metadata
--------------------------

``mkimage`` automates the entire process. When it encounters a
``dm-verity`` subnode, it:

1. Writes the embedded image data to a temporary file.
2. Runs ``veritysetup format`` with the user-supplied algorithm and
   block sizes.
3. Parses ``Root hash`` and ``Salt`` from ``veritysetup`` stdout.
4. Reads the expanded content (original data + Merkle hash tree) back
   into an in-memory buffer and removes the temporary file.  The
   external-data section written to the .itb file uses this buffer in
   place of the original ``data`` property.
5. Writes the computed ``digest``, ``salt``, ``num-data-blocks``, and
   ``hash-start-block`` properties into the ``dm-verity`` subnode.

Images with ``dm-verity`` subnodes **must** use external data layout
(``mkimage -E``). ``mkimage`` will abort with an error if ``-E`` is
not specified.

Usage::

    # Create the filesystem image
    mksquashfs rootfs/ rootfs.squashfs -comp xz

    # Build the FIT (dm-verity is computed automatically); align each
    # external-data section to the block size of the underlying storage
    # (see the alignment note below).
    mkimage -E -B 0x1000 -f image.its image.itb

``veritysetup`` (from the cryptsetup_ package) must be installed on
the build host.

.. _cryptsetup: https://gitlab.com/cryptsetup/cryptsetup

.. note::

   ``veritysetup format`` is invoked with ``--no-superblock``, so no
   on-disk superblock is written between the data and hash regions.
   The Merkle hash tree is appended directly to the image data within
   the FIT external data section. ``hash-start-block`` is therefore
   computed as ``data_size / hash-block-size`` (the offset of the hash
   region in units of ``hash-block-size``). When ``data-block-size``
   equals ``hash-block-size`` this happens to equal ``num-data-blocks``.

.. note::

   The Linux ``fitblk`` driver currently requires each ``filesystem``
   sub-image to start and end on block boundaries of the underlying
   block device (typically 512 bytes, sometimes 4 KiB for eMMC or NVMe
   with 4 KiB native sectors). Use ``mkimage -B <align>`` to pad
   external-data sections to that boundary; ``-B 0x1000`` is a safe
   default for the storage in common use.

   This alignment requirement comes from the kernel-side ``fitblk``
   driver to avoid unaligned-access fix-up overhead in block I/O, and
   is **independent** of the dm-verity ``data-block-size`` and
   ``hash-block-size`` properties -- those describe the block sizes
   used by the device-mapper verity target itself, not storage
   alignment.

Kconfig
-------

``CONFIG_FIT_VERITY``
  Depends on ``CONFIG_FIT`` and ``CONFIG_OF_LIBFDT``.
  When enabled, ``fit_verity_build_cmdline()`` and
  ``fit_verity_apply_bootargs()`` are compiled into the boot path.
  When disabled, the functions are static inlines returning 0, so there
  is no code-size impact. Works with both the ``bootm`` command and
  BOOTSTD boot methods.
