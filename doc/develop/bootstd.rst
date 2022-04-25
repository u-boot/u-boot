.. SPDX-License-Identifier: GPL-2.0+:

U-Boot Standard Boot
====================

Introduction
------------

Standard boot provides a built-in way for U-Boot to automatically boot
an Operating System without custom scripting and other customisation. It
introduces the following concepts:

   - bootdev  - a device which can hold or access a distro (e.g. MMC, Ethernet)
   - bootmeth - a method to scan a bootdev to find bootflows (e.g. distro boot)
   - bootflow - a description of how to boot (provided by the distro)

For Linux, the distro (Linux distribution, e.g. Debian, Fedora) is responsible
for creating a bootflow for each kernel combination that it wants to offer.
These bootflows are stored on media so they can be discovered by U-Boot. This
feature is typically called `distro boot` (see :doc:`distro`) because it is
a way for distributions to boot on any hardware.

Traditionally U-Boot has relied on scripts to implement this feature. See
disto_boodcmd_ for details. This is done because U-Boot has no native support
for scanning devices. While the scripts work remarkably well, they can be hard
to understand and extend, and the feature does not include tests. They are also
making it difficult to move away from ad-hoc CONFIGs, since they are implemented
using the environment and a lot of #defines.

Standard boot is a generalisation of distro boot. It provides a more built-in
way to boot with U-Boot. The feature is extensible to different Operating
Systems (such as Chromium OS) and devices (beyond just block and network
devices). It supports EFI boot and EFI bootmgr too.


Bootflow
--------

A bootflow is a file that describes how to boot a distro. Conceptually there can
be different formats for that file but at present U-Boot only supports the
BootLoaderSpec_ format. which looks something like this::

   menu autoboot Welcome to Fedora-Workstation-armhfp-31-1.9. Automatic boot in # second{,s}. Press a key for options.
   menu title Fedora-Workstation-armhfp-31-1.9 Boot Options.
   menu hidden

   label Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
       kernel /vmlinuz-5.3.7-301.fc31.armv7hl
       append ro root=UUID=9732b35b-4cd5-458b-9b91-80f7047e0b8a rhgb quiet LANG=en_US.UTF-8 cma=192MB cma=256MB
       fdtdir /dtb-5.3.7-301.fc31.armv7hl/
       initrd /initramfs-5.3.7-301.fc31.armv7hl.img

As you can see it specifies a kernel, a ramdisk (initrd) and a directory from
which to load devicetree files. The details are described in disto_boodcmd_.

The bootflow is provided by the distro. It is not part of U-Boot. U-Boot's job
is simply to interpret the file and carry out the instructions. This allows
distros to boot on essentially any device supported by U-Boot.

Typically the first available bootflow is selected and booted. If that fails,
then the next one is tried.


Bootdev
-------

Where does U-Boot find the media that holds the operating systems? That is the
job of bootdev. A bootdev is simply a layer on top of a media device (such as
MMC, NVMe). The bootdev accesses the device, including partitions and
filesystems that might contain things related to an operating system.

For example, an MMC bootdev provides access to the individual partitions on the
MMC device. It scans through these to find filesystems, then provides a list of
these for consideration.


Bootmeth
--------

Once the list of filesystems is provided, how does U-Boot find the bootflow
files in these filesystems. That is the job of bootmeth. Each boot method has
its own way of doing this.

For example, the distro bootmeth simply looks through the provided filesystem
for a file called `extlinux/extlinux.conf`. This files constitutes a bootflow.
If the distro bootmeth is used on multiple partitions it may produce multiple
bootflows.

Note: it is possible to have a bootmeth that uses a partition or a whole device
directly, but it is more common to use a filesystem.


Boot process
------------

U-Boot tries to use the 'lazy init' approach whereever possible and distro boot
is no exception. The algorithm is::

   while (get next bootdev)
      while (get next bootmeth)
          while (get next bootflow)
              try to boot it

So U-Boot works its way through the bootdevs, trying each bootmeth in turn to
obtain bootflows, until it either boots or exhausts the available options.

Instead of 500 lines of #defines and a 4KB boot script, all that is needed is
the following command::

   bootflow scan -lb

which scans for available bootflows, optionally listing each find it finds (-l)
and trying to boot it (-b).


Controlling ordering
--------------------

Several options are available to control the ordering of boot scanning:


boot_targets
~~~~~~~~~~~~

This environment variable can be used to control the list of bootdevs searched
and their ordering, for example::

   setenv boot_targets "mmc0 mmc1 usb pxe"

Entries may be removed or re-ordered in this list to affect the boot order. If
the variable is empty, the default ordering is used, based on the priority of
bootdevs and their sequence numbers.


bootmeths
~~~~~~~~~

This environment variable can be used to control the list of bootmeths used and
their ordering for example::

   setenv bootmeths "syslinux efi"

Entries may be removed or re-ordered in this list to affect the order the
bootmeths are tried on each bootdev. If the variable is empty, the default
ordering is used, based on the bootmeth sequence numbers, which can be
controlled by aliases.

The :ref:`usage/cmd/bootmeth:bootmeth command` (`bootmeth order`) operates in
the same way as setting this variable.


Bootdev uclass
--------------

The bootdev uclass provides an simple API call to obtain a bootflows from a
device::

   int bootdev_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
                            struct bootflow *bflow);

This takes a iterator which indicates the bootdev, partition and bootmeth to
use. It returns a bootflow. This is the core of the bootdev implementation. The
bootdev drivers that implement this differ depending on the media they are
reading from, but each is responsible for returning a valid bootflow if
available.

A helper called `bootdev_find_in_blk()` makes it fairly easy to implement this
function for each media device uclass, in a few lines of code.


Bootdev drivers
---------------

A bootdev driver is typically fairly simple. Here is one for mmc::

    static int mmc_get_bootflow(struct udevice *dev, struct bootflow_iter *iter,
                    struct bootflow *bflow)
    {
        struct udevice *mmc_dev = dev_get_parent(dev);
        struct udevice *blk;
        int ret;

        ret = mmc_get_blk(mmc_dev, &blk);
        /*
         * If there is no media, indicate that no more partitions should be
         * checked
         */
        if (ret == -EOPNOTSUPP)
            ret = -ESHUTDOWN;
        if (ret)
            return log_msg_ret("blk", ret);
        assert(blk);
        ret = bootdev_find_in_blk(dev, blk, iter, bflow);
        if (ret)
            return log_msg_ret("find", ret);

        return 0;
    }

    static int mmc_bootdev_bind(struct udevice *dev)
    {
        struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

        ucp->prio = BOOTDEVP_0_INTERNAL_FAST;

        return 0;
    }

    struct bootdev_ops mmc_bootdev_ops = {
        .get_bootflow    = mmc_get_bootflow,
    };

    static const struct udevice_id mmc_bootdev_ids[] = {
        { .compatible = "u-boot,bootdev-mmc" },
        { }
    };

    U_BOOT_DRIVER(mmc_bootdev) = {
        .name        = "mmc_bootdev",
        .id        = UCLASS_BOOTDEV,
        .ops        = &mmc_bootdev_ops,
        .bind        = mmc_bootdev_bind,
        .of_match    = mmc_bootdev_ids,
    };

The implementation of the `get_bootflow()` method is simply to obtain the
block device and call a bootdev helper function to do the rest. The
implementation of `bootdev_find_in_blk()` checks the partition table, and
attempts to read a file from a filesystem on the partition number given by the
`@iter->part` parameter.

Each bootdev has a priority, which indicates the order in which it is used.
Faster bootdevs are used first, since they are more likely to be able to boot
the device quickly.


Device hierarchy
----------------

A bootdev device is a child of the media device. In this example, you can see
that the bootdev is a sibling of the block device and both are children of
media device::

    mmc           0  [ + ]   bcm2835-sdhost        |   |-- mmc@7e202000
    blk           0  [ + ]   mmc_blk               |   |   |-- mmc@7e202000.blk
    bootdev       0  [   ]   mmc_bootdev           |   |   `-- mmc@7e202000.bootdev
    mmc           1  [ + ]   sdhci-bcm2835         |   |-- sdhci@7e300000
    blk           1  [   ]   mmc_blk               |   |   |-- sdhci@7e300000.blk
    bootdev       1  [   ]   mmc_bootdev           |   |   `-- sdhci@7e300000.bootdev

The bootdev device is typically created automatically in the media uclass'
`post_bind()` method by calling `bootdev_setup_for_dev()`. The code typically
something like this::

    ret = bootdev_setup_for_dev(dev, "eth_bootdev");
    if (ret)
        return log_msg_ret("bootdev", ret);

Here, `eth_bootdev` is the name of the Ethernet bootdev driver and `dev`
is the ethernet device. This function is safe to call even if standard boot is
not enabled, since it does nothing in that case. It can be added to all uclasses
which implement suitable media.


The bootstd device
------------------

Standard boot requires a single instance of the bootstd device to make things
work. This includes global information about the state of standard boot. See
`struct bootstd_priv` for this structure, accessed with `bootstd_get_priv()`.

Within the devicetree, if you add bootmeth devices or a system bootdev, they
should be children of the bootstd device. See `arch/sandbox/dts/test.dts` for
an example of this.


The system bootdev
------------------

Some bootmeths don't operate on individual bootdevs, but on the whole system.
For example, the EFI boot manager does its own device scanning and does not
make use of the bootdev devices. Such bootmeths can make use of the system
bootdev, typically considered last, after everything else has been tried.


.. _`Automatic Devices`:

Automatic devices
-----------------

It is possible to define all the required devices in the devicetree manually,
but it is not necessary. The bootstd uclass includes a `dm_scan_other()`
function which creates the bootstd device if not found. If no bootmeth devices
are found at all, it creates one for each available bootmeth driver as well as a
system bootdev.

If your devicetree has any bootmeth device it must have all of them that you
want to use, as well as the system bootdev if needed, since no bootmeth devices
will be created automatically in that case.


Using devicetree
----------------

If a bootdev is complicated or needs configuration information, it can be
added to the devicetree as a child of the media device. For example, imagine a
bootdev which reads a bootflow from SPI flash. The devicetree fragment might
look like this::

    spi@0 {
        flash@0 {
            reg = <0>;
            compatible = "spansion,m25p16", "jedec,spi-nor";
            spi-max-frequency = <40000000>;

            bootdev {
                compatible = "u-boot,sf-bootdev";
                offset = <0x2000>;
                size = <0x1000>;
            };
        };
    };

The `sf-bootdev` driver can implement a way to read from the SPI flash, using
the offset and size provided, and return that bootflow file back to the caller.
When distro boot wants to read the kernel it calls disto_getfile() which must
provide a way to read from the SPI flash. See `distro_boot()` at distro_boot_
for more details.

Of course this is all internal to U-Boot. All the distro sees is another way
to boot.


Configuration
-------------

Standard boot is enabled with `CONFIG_BOOTSTD`. Each bootmeth has its own CONFIG
option also. For example, `CONFIG_BOOTMETH_DISTRO` enables support for distro
boot from a disk.


Available bootmeth drivers
--------------------------

Bootmeth drivers are provided for:

   - distro boot from a disk (syslinux)
   - distro boot from a network (PXE)
   - EFI boot using bootefi
   - EFI boot using boot manager


Command interface
-----------------

Three commands are available:

`bootdev`
    Allows listing of available bootdevs, selecting a particular one and
    getting information about it. See :doc:`../usage/cmd/bootdev`

`bootflow`
    Allows scanning one or more bootdevs for bootflows, listing available
    bootflows, selecting one, obtaining information about it and booting it.
    See :doc:`../usage/cmd/bootflow`

`bootmeth`
    Allow listing of available bootmethds and setting the order in which they
    are tried. See :doc:`../usage/cmd/bootmeth`

.. _BootflowStates:

Bootflow states
---------------

Here is a list of states that a bootflow can be in:

=======  =======================================================================
State    Meaning
=======  =======================================================================
base     Starting-out state, indicates that no media/partition was found. For an
         SD card socket it may indicate that the card is not inserted.
media    Media was found (e.g. SD card is inserted) but no partition information
         was found. It might lack a partition table or have a read error.
part     Partition was found but a filesystem could not be read. This could be
         because the partition does not hold a filesystem or the filesystem is
         very corrupted.
fs       Filesystem was found but the file could not be read. It could be
         missing or in the wrong subdirectory.
file     File was found and its size detected, but it could not be read. This
         could indicate filesystem corruption.
ready    File was loaded and is ready for use. In this state the bootflow is
         ready to be booted.
=======  =======================================================================


Theory of operation
-------------------

This describes how standard boot progresses through to booting an operating
system.

To start. all the necessary devices must be bound, including bootstd, which
provides the top-level `struct bootstd_priv` containing optional configuration
information. The bootstd device is also holds the various lists used while
scanning. This step is normally handled automatically by driver model, as
described in `Automatic Devices`_.

Bootdevs are also required, to provide access to the media to use. These are not
useful by themselves: bootmeths are needed to provide the means of scanning
those bootdevs. So, all up, we need a single bootstd device, one or more bootdev
devices and one or more bootmeth devices.

Once these are ready, typically a `bootflow scan` command is issued. This kicks
of the iteration process, which involves looking through the bootdevs and their
partitions one by one to find bootflows.

Iteration is kicked off using `bootflow_scan_first()`, which calls
`bootflow_scan_bootdev()`.

The iterator is set up with `bootflow_iter_init()`. This simply creates an
empty one with the given flags. Flags are used to control whether each
iteration is displayed, whether to return iterations even if they did not result
in a valid bootflow, whether to iterate through just a single bootdev, etc.

Then the ordering of bootdevs is determined, by `bootdev_setup_iter_order()`. By
default, the bootdevs are used in the order specified by the `boot_targets`
environment variable (e.g. "mmc2 mmc0 usb"). If that is missing then their
sequence order is used, as determined by the `/aliases` node, or failing that
their order in the devicetree. For BOOTSTD_FULL, if there is a `bootdev-order`
property in the bootstd node, then this is used as a final fallback. In any
case, the iterator ends up with a `dev_order` array containing the bootdevs that
are going to be used, with `num_devs` set to the number of bootdevs and
`cur_dev` starting at 0.

Next, the ordering of bootdevs is determined, by `bootmeth_setup_iter_order()`.
By default the ordering is again by sequence number, i.e. the `/aliases` node,
or failing that the order in the devicetree. But the `bootmeth order` command
or `bootmeths` environment variable can be used to set up an ordering. If that
has been done, the ordering is in `struct bootstd_priv`, so that ordering is
simply copied into the iterator. Either way, the `method_order` array it set up,
along with `num_methods`. Then `cur_method` is set to 0.

At this point the iterator is ready to use, with the first bootdev and bootmeth
selected. All the other fields are 0. This means that the current partition is
0, which is taken to mean the whole device, since partition numbers start at 1.
It also means that `max_part` is 0, i.e. the maximum partition number we know
about is 0, meaning that, as far as we know, there is no partition table on this
bootdev.

With the iterator ready, `bootflow_scan_bootdev()` checks whether the current
settings produce a valid bootflow. This is handled by `bootflow_check()`, which
either returns 0 (if it got something) or an error if not (more on that later).
If the `BOOTFLOWF_ALL` iterator flag is set, even errors are returned as
incomplete bootflows, but normally an error results in moving onto the next
iteration.

The `bootflow_scan_next()` function handles moving onto the next iteration and
checking it. In fact it sits in a loop doing that repeatedly until it finds
something it wants to return.

The actual 'moving on' part is implemented in `iter_incr()`. This is a very
simple function. It increments the first counter. If that hits its maximum, it
sets it to zero and increments the second counter. You can think of all the
counters together as a number with three digits which increment in order, with
the least-sigificant digit on the right, counting like this:

   ========    =======    =======
   bootdev     part       method
   ========    =======    =======
   0           0          0
   0           0          1
   0           0          2
   0           1          0
   0           1          1
   0           1          1
   1           0          0
   1           0          1
   ========    =======    =======

The maximum value for `method` is `num_methods - 1` so when it exceeds that, it
goes back to 0 and the next `part` is considered. The maximum value for that is
`max_part`, which is initially zero for all bootdevs. If we find a partition
table on that bootdev, `max_part` can be updated during the iteration to a
higher value - see `bootdev_find_in_blk()` for that, described later. If that
exceeds its maximum, then the next bootdev is used. In this way, iter_incr()
works its way through all possibilities, moving forward one each time it is
called.

There is no expectation that iteration will actually finish. Quite often a
valid bootflow is found early on. With `bootflow scan -b`, that causes the
bootflow to be immediately booted. Assuming it is successful, the iteration never
completes.

Also note that the iterator hold the **current** combination being considered.
So when `iter_incr()` is called, it increments to the next one and returns it,
the new **current** combination.

Note also the `err` field in `struct bootflow_iter`. This is normally 0 and has
thus has no effect on `iter_inc()`. But if it is non-zero, signalling an error,
it indicates to the iterator what it should do when called. It can force moving
to the next partition, or bootdev, for example. The special values
`BF_NO_MORE_PARTS` and `BF_NO_MORE_DEVICES` handle this. When `iter_incr` sees
`BF_NO_MORE_PARTS` it knows that it should immediately move to the next bootdev.
When it sees `BF_NO_MORE_DEVICES` it knows that there is nothing more it can do
so it should immediately return. The caller of `iter_incr()` is responsible for
updating the `err` field, based on the return value it sees.

The above describes the iteration process at a high level. It is basically a
very simple increment function with a checker called `bootflow_check()` that
checks the result of each iteration generated, to determine whether it can
produce a bootflow.

So what happens inside of `bootflow_check()`? It simply calls the uclass
method `bootdev_get_bootflow()` to ask the bootdev to return a bootflow. It
passes the iterator to the bootdev method, so that function knows what we are
talking about. At first, the bootflow is set up in the state `BOOTFLOWST_BASE`,
with just the `method` and `dev` intiialised. But the bootdev may fill in more,
e.g. updating the state, depending on what it finds.

Based on what the bootdev responds with, `bootflow_check()` either
returns a valid bootflow, or a partial one with an error. A partial bootflow
is one that has some fields set up, but did not reach the `BOOTFLOWST_READY`
state. As noted before, if the `BOOTFLOWF_ALL` iterator flag is set, then all
bootflows are returned, even partial ones. This can help with debugging.

So at this point you can see that total control over whether a bootflow can
be generated from a particular iteration, or not, rests with the bootdev.
Each one can adopt its own approach.

Going down a level, what does the bootdev do in its `get_bootflow()` method?
Let us consider the MMC bootdev. In that case the call to
`bootdev_get_bootflow()` ends up in `mmc_get_bootflow()`. It locates the parent
device of the bootdev, i.e. the `UCLASS_MMC` device itself, then finds the block
device associated with it. It then calls the helper function
`bootdev_find_in_blk()` to do all the work. This is common with just about any
bootdev that is based on a media device.

The `bootdev_find_in_blk()` helper is implemented in the bootdev uclass. It
names the bootflow and copies the partition number in from the iterator. Then it
calls the bootmeth device to check if it can support this device. This is
important since some bootmeths only work with network devices, for example. If
that check fails, it stops.

Assuming the bootmeth is happy, or at least indicates that it is willing to try
(by returning 0 from its `check()` method), the next step is to try the
partition. If that works it tries to detect a file system. If that works then it
calls the bootmeth device once more, this time to read the bootflow.

Note: At present a filesystem is needed for the bootmeth to be called on block
devices, simply because we don't have any examples where this is not the case.
This feature can be added as needed.

If we take the example of the `bootmeth_distro` driver, this call ends up at
`distro_read_bootflow()`. It has the filesystem ready, so tries various
filenames to try to find the `extlinux.conf` file, reading it if possible. If
all goes well the bootflow ends up in the `BOOTFLOWST_READY` state.

At this point, we fall back from the bootmeth driver, to
`bootdev_find_in_blk()`, then back to `mmc_get_bootflow()`, then to
`bootdev_get_bootflow()`, then to `bootflow_check()` and finally to its caller,
either `bootflow_scan_bootdev()` or `bootflow_scan_next()`. In either case,
the bootflow is returned as the result of this iteration, assuming it made it to
the  `BOOTFLOWST_READY` state.

That is the basic operation of scanning for bootflows. The process of booting a
bootflow is handled by the bootmeth driver for that bootflow. In the case of
distro boot, this parses and processes the `extlinux.conf` file that was read.
See `distro_boot()` for how that works. The processing may involve reading
additional files, which is handled by the `read_file()` method, which is
`distro_read_file()` in this case. All bootmethds should support reading files,
since the bootflow is typically only the basic instructions and does not include
the operating system itself, ramdisk, device tree, etc.

The vast majority of the bootstd code is concerned with iterating through
partitions on bootdevs and using bootmethds to find bootflows.

How about bootdevs which are not block devices? They are handled by the same
methods as above, but with a different implementation. For example, the bootmeth
for PXE boot (over a network) uses `tftp` to read files rather than `fs_read()`.
But other than that it is very similar.


Tests
-----

Tests are located in `test/boot` and cover the core functionality as well as
the commands. All tests use sandbox so can be run on a standard Linux computer
and in U-Boot's CI.

For testing, a DOS-formatted disk image is used with a single FAT partition on
it. This is created in `setup_bootflow_image()`, with a canned one from the
source tree used if it cannot be created (e.g. in CI).


Bootflow internals
------------------

The bootstd device holds a linked list of scanned bootflows as well as the
currently selected bootdev and bootflow (for use by commands). This is in
`struct bootstd_priv`.

Each bootdev device has its own `struct bootdev_uc_plat` which holds a
list of scanned bootflows just for that device.

The bootflow itself is documented in bootflow_h_. It includes various bits of
information about the bootflow and a buffer to hold the file.


Future
------

Apart from the to-do items below, different types of bootflow files may be
implemented in future, e.g. Chromium OS support which is currently only
available as a script in chromebook_coral.


To do
-----

Some things that need to be done to completely replace the distro-boot scripts:

- add bootdev drivers for dhcp, sata, scsi, ide, virtio
- PXE boot for EFI
- support for loading U-Boot scripts

Other ideas:

- `bootflow prep` to load everything preparing for boot, so that `bootflow boot`
  can just do the boot.
- automatically load kernel, FDT, etc. to suitable addresses so the board does
  not need to specify things like `pxefile_addr_r`


.. _disto_boodcmd: https://github.com/u-boot/u-boot/blob/master/include/config_distro_bootcmd.h
.. _BootLoaderSpec: http://www.freedesktop.org/wiki/Specifications/BootLoaderSpec/
.. _distro_boot: https://github.com/u-boot/u-boot/blob/master/boot/distro.c
.. _bootflow_h: https://github.com/u-boot/u-boot/blob/master/include/bootflow.h
