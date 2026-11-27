.. SPDX-License-Identifier: GPL-2.0+

MTD virtual concatenation
=========================

Some boards have two or more flash devices, for example QSPI or OSPI flashes
on separate chip selects, and need a single filesystem or image to span them.
MTD virtual concatenation joins partitions on different flash devices into a
single MTD device, based on the ``part-concat-next`` property in the device
tree. The same property is used by the Linux driver, so a board can use one
device tree for both.

Configuration
-------------

Enable ``CONFIG_MTD_VIRT_CONCAT``. This needs ``CONFIG_MTD_PARTITIONS``, as
well as driver model and ``CONFIG_OF_CONTROL``. It does not need
``CONFIG_MTD_CONCAT``, which is for joining NAND and CFI flash chips, and which
this option does not enable.

Device tree
-----------

Add a ``part-concat-next`` property to the first partition of a group, listing
the phandles of the other partitions in order::

    flash@0 {
        partitions {
            compatible = "fixed-partitions";
            #address-cells = <1>;
            #size-cells = <1>;

            part0: partition@0 {
                label = "flash0-part";
                reg = <0x0 0x800000>;
                part-concat-next = <&part1>;
            };
        };
    };

    flash@1 {
        partitions {
            compatible = "fixed-partitions";
            #address-cells = <1>;
            #size-cells = <1>;

            part1: partition@0 {
                label = "flash1-part";
                reg = <0x0 0x800000>;
            };
        };
    };

The property is described in the partition binding, in
``dts/upstream/Bindings/mtd/partitions/partition.yaml``. It can list up to 16
partitions, so a group can have more than two partitions, and there can be
several groups. Each partition can only be in one group. Only partitions
under a ``fixed-partitions`` node are supported.

The concatenated device is named after its partitions, followed by
``concat``, e.g. ``flash0-part-flash1-part-concat`` above. Its size is the sum
of the sizes of its partitions. The partitions in the group are not registered
as MTD devices on their own, while any other partitions of the flash devices
are registered as usual.

Usage
-----

The groups are created when U-Boot scans the MTD partitions, which happens
when a command such as ``mtd`` or ``ubi`` first needs them. The concatenated
device can then be used like any other MTD device::

    => mtd list
    ...
    * flash0-part-flash1-part-concat
      - type: NOR flash
      - block size: 0x10000 bytes
      - min I/O: 0x1 bytes
      - 0x000000000000-0x000001000000 : "flash0-part-flash1-part-concat"
    => mtd erase flash0-part-flash1-part-concat 0x7f0000 0x20000
    => mtd write flash0-part-flash1-part-concat 0x1000000 0x7ff000 0x2000
    => mtd read flash0-part-flash1-part-concat 0x2000000 0x7ff000 0x2000

Here the write and read cross from the first flash to the second.

Interaction with mtdparts
-------------------------

The partitions of a flash device come either from the ``mtdparts`` environment
variable or from the device tree. If ``mtdparts`` defines partitions for a
flash device, its device tree partitions are not added, so none of them can be
part of a group. Partitions defined in ``mtdparts`` cannot be concatenated.

For example, with the device tree above and::

    => setenv mtdids nor0=flash@0
    => setenv mtdparts mtdparts=flash@0:8m(boot)

flash@0 has the ``boot`` partition from ``mtdparts``, while ``flash0-part`` is
not added. The group is then incomplete, so a warning is printed and
``flash1-part`` is registered on its own::

    partition@0: found 1 of 2 partitions, not concatenating

To use a group, leave its flash devices out of ``mtdparts``. Other flash
devices can still use ``mtdparts``.

A concatenated device which exists already is destroyed when ``mtdparts``
defines the partitions of one of its flash devices. If it is in use, e.g. by
UBI, ``mtdparts`` is not applied to that flash device and a message is printed,
as it is when one of the flash device's partitions is in use. It is applied by
a later scan, once the concatenated device is free.

Behaviour
---------

Rescanning
    The partitions are scanned again when ``mtdids`` or ``mtdparts`` changes,
    or when an MTD device is added or removed. A scan does not change a
    concatenated device, so it keeps its place in the list of MTD devices and
    can be used while the other partitions of its flash devices come and go. It
    is destroyed when one of its flash devices is removed, or when
    ``mtdparts`` defines the partitions of one of them, and a later scan
    creates it again if its group is complete.

Incomplete groups
    If a partition in a group is not found, e.g. because its flash device is
    not present or uses ``mtdparts`` (see above), or the partitions cannot be
    joined, a warning is printed and the partitions of the group which were
    found are registered individually.

Removing a flash device
    When a flash device is removed, for example when ``sf probe`` probes it
    again, every concatenated device which uses it is destroyed. A later scan
    creates it again. If one is still in use, e.g. by UBI, a warning is printed
    and it is kept, so stop using it before removing the flash device.

Testing
-------

The sandbox test device tree concatenates the ``nor-spare`` partition of
``spi.bin@0`` with the ``nor1-spare`` partition of ``spi.bin@1``, and the
``nor1-b1`` and ``nor1-b2`` partitions of ``spi.bin@1`` with each other. The
tests are in ``test/dm/mtd_concat.c``::

    ./u-boot -T -c "ut dm mtd_virt_concat*"
