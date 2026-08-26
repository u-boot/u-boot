.. SPDX-License-Identifier: GPL-2.0+:

BLS Bootmeth
============

The `Boot Loader Specification
<https://uapi-group.org/specifications/specs/boot_loader_specification>`_ (BLS)
describes a drop-in directory of boot entries, so that a distribution can add
and remove kernels without rewriting a central configuration file. Type #1
entries are plain text files, one per bootable kernel; this bootmeth implements
those. Type #2 entries are EFI binaries and are out of scope here, since the
EFI bootmeth already covers them.

Entries live in ``<prefix>loader/entries/*.conf``, where ``<prefix>`` comes from
the bootstd list of prefixes (``{"/", "/boot/"}`` by default, settable with the
`filename-prefixes` property on the bootstd device). systemd's
``kernel-install`` writes them in this layout when ``loader=bls``.

The format is close enough to `extlinux.conf` that U-Boot reuses the same
parser. Two extra keywords are recognised for it: ``title`` (the human-readable
entry name, equivalent to extlinux's ``menu label``) and ``options`` (the kernel
command line, equivalent to ``append``). Some spec-envisaged keys such as
``version``, ``machine-id``, ``sort-key`` and ``architecture`` are not yet
implemented and are skipped.

Paths inside an entry are used as written: the spec has them "always relative
to the root directory of the partition they are referenced from", which is
what the bootmeth sees whichever prefix the entry itself was found under.
systemd's ``90-loaderentry.install`` agrees, since it strips the mount point
from the entry directory and so keeps the ``/boot`` component when $BOOT is
not a mount point of its own.

Entry selection
---------------

Only the highest-sorting entry on a partition is returned, because the bootstd
framework currently allows one bootflow per (bootmeth, partition) pair.

Ordering is by filename alone, which suits the common case of distros encoding
the kernel version into the filename. The spec envisages more than this, none
of which is implemented yet:

* entries should be ordered by ``sort-key`` first and ``version`` second,
  falling back to the filename only when those tie;

* an entry may carry a boot counter as a ``+TRIES_LEFT[-TRIES_DONE]`` suffix on
  its filename, to be decremented on each attempt and skipped once it reaches
  zero. For now that suffix is left intact in the entry name and otherwise
  ignored.

When the bootflow is booted, the bootmeth re-parses the entry it read during
the scan and hands the resulting label to ``label_boot()``, the same code path
extlinux uses.

The compatible string "u-boot,bls" is used for the driver. It is present if
``CONFIG_BOOTMETH_BLS`` is enabled.
