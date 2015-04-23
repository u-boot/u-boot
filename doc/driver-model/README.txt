Driver Model
============

This README contains high-level information about driver model, a unified
way of declaring and accessing drivers in U-Boot. The original work was done
by:

   Marek Vasut <marex@denx.de>
   Pavel Herrmann <morpheus.ibis@gmail.com>
   Viktor Křivák <viktor.krivak@gmail.com>
   Tomas Hlavacek <tmshlvck@gmail.com>

This has been both simplified and extended into the current implementation
by:

   Simon Glass <sjg@chromium.org>


Terminology
-----------

Uclass - a group of devices which operate in the same way. A uclass provides
	a way of accessing individual devices within the group, but always
	using the same interface. For example a GPIO uclass provides
	operations for get/set value. An I2C uclass may have 10 I2C ports,
	4 with one driver, and 6 with another.

Driver - some code which talks to a peripheral and presents a higher-level
	interface to it.

Device - an instance of a driver, tied to a particular port or peripheral.


How to try it
-------------

Build U-Boot sandbox and run it:

   make sandbox_defconfig
   make
   ./u-boot -d u-boot.dtb

   (type 'reset' to exit U-Boot)


There is a uclass called 'demo'. This uclass handles
saying hello, and reporting its status. There are two drivers in this
uclass:

   - simple: Just prints a message for hello, doesn't implement status
   - shape: Prints shapes and reports number of characters printed as status

The demo class is pretty simple, but not trivial. The intention is that it
can be used for testing, so it will implement all driver model features and
provide good code coverage of them. It does have multiple drivers, it
handles parameter data and platdata (data which tells the driver how
to operate on a particular platform) and it uses private driver data.

To try it, see the example session below:

=>demo hello 1
Hello '@' from 07981110: red 4
=>demo status 2
Status: 0
=>demo hello 2
g
r@
e@@
e@@@
n@@@@
g@@@@@
=>demo status 2
Status: 21
=>demo hello 4 ^
  y^^^
 e^^^^^
l^^^^^^^
l^^^^^^^
 o^^^^^
  w^^^
=>demo status 4
Status: 36
=>


Running the tests
-----------------

The intent with driver model is that the core portion has 100% test coverage
in sandbox, and every uclass has its own test. As a move towards this, tests
are provided in test/dm. To run them, try:

   ./test/dm/test-dm.sh

You should see something like this:

    <...U-Boot banner...>
    Running 53 driver model tests
    Test: dm_test_autobind
    Test: dm_test_autoprobe
    Test: dm_test_bus_child_post_bind
    Test: dm_test_bus_child_post_bind_uclass
    Test: dm_test_bus_child_pre_probe_uclass
    Test: dm_test_bus_children
    Device 'c-test@0': seq 0 is in use by 'd-test'
    Device 'c-test@1': seq 1 is in use by 'f-test'
    Test: dm_test_bus_children_funcs
    Test: dm_test_bus_children_iterators
    Test: dm_test_bus_parent_data
    Test: dm_test_bus_parent_data_uclass
    Test: dm_test_bus_parent_ops
    Test: dm_test_bus_parent_platdata
    Test: dm_test_bus_parent_platdata_uclass
    Test: dm_test_children
    Test: dm_test_device_get_uclass_id
    Test: dm_test_eth
    Using eth@10002000 device
    Using eth@10003000 device
    Using eth@10004000 device
    Test: dm_test_eth_alias
    Using eth@10002000 device
    Using eth@10004000 device
    Using eth@10002000 device
    Using eth@10003000 device
    Test: dm_test_eth_prime
    Using eth@10003000 device
    Using eth@10002000 device
    Test: dm_test_eth_rotate

    Error: eth@10004000 address not set.

    Error: eth@10004000 address not set.
    Using eth@10002000 device

    Error: eth@10004000 address not set.

    Error: eth@10004000 address not set.
    Using eth@10004000 device
    Test: dm_test_fdt
    Test: dm_test_fdt_offset
    Test: dm_test_fdt_pre_reloc
    Test: dm_test_fdt_uclass_seq
    Test: dm_test_gpio
    extra-gpios: get_value: error: gpio b5 not reserved
    Test: dm_test_gpio_anon
    Test: dm_test_gpio_copy
    Test: dm_test_gpio_leak
    extra-gpios: get_value: error: gpio b5 not reserved
    Test: dm_test_gpio_phandles
    Test: dm_test_gpio_requestf
    Test: dm_test_i2c_bytewise
    Test: dm_test_i2c_find
    Test: dm_test_i2c_offset
    Test: dm_test_i2c_offset_len
    Test: dm_test_i2c_probe_empty
    Test: dm_test_i2c_read_write
    Test: dm_test_i2c_speed
    Test: dm_test_leak
    Test: dm_test_lifecycle
    Test: dm_test_net_retry
    Using eth@10004000 device
    Using eth@10002000 device
    Using eth@10004000 device
    Test: dm_test_operations
    Test: dm_test_ordering
    Test: dm_test_pci_base
    Test: dm_test_pci_swapcase
    Test: dm_test_platdata
    Test: dm_test_pre_reloc
    Test: dm_test_remove
    Test: dm_test_spi_find
    Invalid chip select 0:0 (err=-19)
    SF: Failed to get idcodes
    SF: Detected M25P16 with page size 256 Bytes, erase size 64 KiB, total 2 MiB
    Test: dm_test_spi_flash
    2097152 bytes written in 0 ms
    SF: Detected M25P16 with page size 256 Bytes, erase size 64 KiB, total 2 MiB
    SPI flash test:
    0 erase: 0 ticks, 65536000 KiB/s 524288.000 Mbps
    1 check: 0 ticks, 65536000 KiB/s 524288.000 Mbps
    2 write: 0 ticks, 65536000 KiB/s 524288.000 Mbps
    3 read: 0 ticks, 65536000 KiB/s 524288.000 Mbps
    Test passed
    0 erase: 0 ticks, 65536000 KiB/s 524288.000 Mbps
    1 check: 0 ticks, 65536000 KiB/s 524288.000 Mbps
    2 write: 0 ticks, 65536000 KiB/s 524288.000 Mbps
    3 read: 0 ticks, 65536000 KiB/s 524288.000 Mbps
    Test: dm_test_spi_xfer
    SF: Detected M25P16 with page size 256 Bytes, erase size 64 KiB, total 2 MiB
    Test: dm_test_uclass
    Test: dm_test_uclass_before_ready
    Test: dm_test_usb_base
    Test: dm_test_usb_flash
    USB-1:   scanning bus 1 for devices... 2 USB Device(s) found
    Failures: 0


What is going on?
-----------------

Let's start at the top. The demo command is in common/cmd_demo.c. It does
the usual command processing and then:

	struct udevice *demo_dev;

	ret = uclass_get_device(UCLASS_DEMO, devnum, &demo_dev);

UCLASS_DEMO means the class of devices which implement 'demo'. Other
classes might be MMC, or GPIO, hashing or serial. The idea is that the
devices in the class all share a particular way of working. The class
presents a unified view of all these devices to U-Boot.

This function looks up a device for the demo uclass. Given a device
number we can find the device because all devices have registered with
the UCLASS_DEMO uclass.

The device is automatically activated ready for use by uclass_get_device().

Now that we have the device we can do things like:

	return demo_hello(demo_dev, ch);

This function is in the demo uclass. It takes care of calling the 'hello'
method of the relevant driver. Bearing in mind that there are two drivers,
this particular device may use one or other of them.

The code for demo_hello() is in drivers/demo/demo-uclass.c:

int demo_hello(struct udevice *dev, int ch)
{
	const struct demo_ops *ops = device_get_ops(dev);

	if (!ops->hello)
		return -ENOSYS;

	return ops->hello(dev, ch);
}

As you can see it just calls the relevant driver method. One of these is
in drivers/demo/demo-simple.c:

static int simple_hello(struct udevice *dev, int ch)
{
	const struct dm_demo_pdata *pdata = dev_get_platdata(dev);

	printf("Hello from %08x: %s %d\n", map_to_sysmem(dev),
	       pdata->colour, pdata->sides);

	return 0;
}


So that is a trip from top (command execution) to bottom (driver action)
but it leaves a lot of topics to address.


Declaring Drivers
-----------------

A driver declaration looks something like this (see
drivers/demo/demo-shape.c):

static const struct demo_ops shape_ops = {
	.hello = shape_hello,
	.status = shape_status,
};

U_BOOT_DRIVER(demo_shape_drv) = {
	.name	= "demo_shape_drv",
	.id	= UCLASS_DEMO,
	.ops	= &shape_ops,
	.priv_data_size = sizeof(struct shape_data),
};


This driver has two methods (hello and status) and requires a bit of
private data (accessible through dev_get_priv(dev) once the driver has
been probed). It is a member of UCLASS_DEMO so will register itself
there.

In U_BOOT_DRIVER it is also possible to specify special methods for bind
and unbind, and these are called at appropriate times. For many drivers
it is hoped that only 'probe' and 'remove' will be needed.

The U_BOOT_DRIVER macro creates a data structure accessible from C,
so driver model can find the drivers that are available.

The methods a device can provide are documented in the device.h header.
Briefly, they are:

    bind - make the driver model aware of a device (bind it to its driver)
    unbind - make the driver model forget the device
    ofdata_to_platdata - convert device tree data to platdata - see later
    probe - make a device ready for use
    remove - remove a device so it cannot be used until probed again

The sequence to get a device to work is bind, ofdata_to_platdata (if using
device tree) and probe.


Platform Data
-------------

Platform data is like Linux platform data, if you are familiar with that.
It provides the board-specific information to start up a device.

Why is this information not just stored in the device driver itself? The
idea is that the device driver is generic, and can in principle operate on
any board that has that type of device. For example, with modern
highly-complex SoCs it is common for the IP to come from an IP vendor, and
therefore (for example) the MMC controller may be the same on chips from
different vendors. It makes no sense to write independent drivers for the
MMC controller on each vendor's SoC, when they are all almost the same.
Similarly, we may have 6 UARTs in an SoC, all of which are mostly the same,
but lie at different addresses in the address space.

Using the UART example, we have a single driver and it is instantiated 6
times by supplying 6 lots of platform data. Each lot of platform data
gives the driver name and a pointer to a structure containing information
about this instance - e.g. the address of the register space. It may be that
one of the UARTS supports RS-485 operation - this can be added as a flag in
the platform data, which is set for this one port and clear for the rest.

Think of your driver as a generic piece of code which knows how to talk to
a device, but needs to know where it is, any variant/option information and
so on. Platform data provides this link between the generic piece of code
and the specific way it is bound on a particular board.

Examples of platform data include:

   - The base address of the IP block's register space
   - Configuration options, like:
         - the SPI polarity and maximum speed for a SPI controller
         - the I2C speed to use for an I2C device
         - the number of GPIOs available in a GPIO device

Where does the platform data come from? It is either held in a structure
which is compiled into U-Boot, or it can be parsed from the Device Tree
(see 'Device Tree' below).

For an example of how it can be compiled in, see demo-pdata.c which
sets up a table of driver names and their associated platform data.
The data can be interpreted by the drivers however they like - it is
basically a communication scheme between the board-specific code and
the generic drivers, which are intended to work on any board.

Drivers can access their data via dev->info->platdata. Here is
the declaration for the platform data, which would normally appear
in the board file.

	static const struct dm_demo_cdata red_square = {
		.colour = "red",
		.sides = 4.
	};
	static const struct driver_info info[] = {
		{
			.name = "demo_shape_drv",
			.platdata = &red_square,
		},
	};

	demo1 = driver_bind(root, &info[0]);


Device Tree
-----------

While platdata is useful, a more flexible way of providing device data is
by using device tree. With device tree we replace the above code with the
following device tree fragment:

	red-square {
		compatible = "demo-shape";
		colour = "red";
		sides = <4>;
	};

This means that instead of having lots of U_BOOT_DEVICE() declarations in
the board file, we put these in the device tree. This approach allows a lot
more generality, since the same board file can support many types of boards
(e,g. with the same SoC) just by using different device trees. An added
benefit is that the Linux device tree can be used, thus further simplifying
the task of board-bring up either for U-Boot or Linux devs (whoever gets to
the board first!).

The easiest way to make this work it to add a few members to the driver:

	.platdata_auto_alloc_size = sizeof(struct dm_test_pdata),
	.ofdata_to_platdata = testfdt_ofdata_to_platdata,

The 'auto_alloc' feature allowed space for the platdata to be allocated
and zeroed before the driver's ofdata_to_platdata() method is called. The
ofdata_to_platdata() method, which the driver write supplies, should parse
the device tree node for this device and place it in dev->platdata. Thus
when the probe method is called later (to set up the device ready for use)
the platform data will be present.

Note that both methods are optional. If you provide an ofdata_to_platdata
method then it will be called first (during activation). If you provide a
probe method it will be called next. See Driver Lifecycle below for more
details.

If you don't want to have the platdata automatically allocated then you
can leave out platdata_auto_alloc_size. In this case you can use malloc
in your ofdata_to_platdata (or probe) method to allocate the required memory,
and you should free it in the remove method.

The driver model tree is intended to mirror that of the device tree. The
root driver is at device tree offset 0 (the root node, '/'), and its
children are the children of the root node.


Declaring Uclasses
------------------

The demo uclass is declared like this:

U_BOOT_CLASS(demo) = {
	.id		= UCLASS_DEMO,
};

It is also possible to specify special methods for probe, etc. The uclass
numbering comes from include/dm/uclass.h. To add a new uclass, add to the
end of the enum there, then declare your uclass as above.


Device Sequence Numbers
-----------------------

U-Boot numbers devices from 0 in many situations, such as in the command
line for I2C and SPI buses, and the device names for serial ports (serial0,
serial1, ...). Driver model supports this numbering and permits devices
to be locating by their 'sequence'. This numbering uniquely identifies a
device in its uclass, so no two devices within a particular uclass can have
the same sequence number.

Sequence numbers start from 0 but gaps are permitted. For example, a board
may have I2C buses 1, 4, 5 but no 0, 2 or 3. The choice of how devices are
numbered is up to a particular board, and may be set by the SoC in some
cases. While it might be tempting to automatically renumber the devices
where there are gaps in the sequence, this can lead to confusion and is
not the way that U-Boot works.

Each device can request a sequence number. If none is required then the
device will be automatically allocated the next available sequence number.

To specify the sequence number in the device tree an alias is typically
used. Make sure that the uclass has the DM_UC_FLAG_SEQ_ALIAS flag set.

aliases {
	serial2 = "/serial@22230000";
};

This indicates that in the uclass called "serial", the named node
("/serial@22230000") will be given sequence number 2. Any command or driver
which requests serial device 2 will obtain this device.

More commonly you can use node references, which expand to the full path:

aliases {
	serial2 = &serial_2;
};
...
serial_2: serial@22230000 {
...
};

The alias resolves to the same string in this case, but this version is
easier to read.

Device sequence numbers are resolved when a device is probed. Before then
the sequence number is only a request which may or may not be honoured,
depending on what other devices have been probed. However the numbering is
entirely under the control of the board author so a conflict is generally
an error.


Bus Drivers
-----------

A common use of driver model is to implement a bus, a device which provides
access to other devices. Example of buses include SPI and I2C. Typically
the bus provides some sort of transport or translation that makes it
possible to talk to the devices on the bus.

Driver model provides some useful features to help with implementing buses.
Firstly, a bus can request that its children store some 'parent data' which
can be used to keep track of child state. Secondly, the bus can define
methods which are called when a child is probed or removed. This is similar
to the methods the uclass driver provides. Thirdly, per-child platform data
can be provided to specify things like the child's address on the bus. This
persists across child probe()/remove() cycles.

For consistency and ease of implementation, the bus uclass can specify the
per-child platform data, so that it can be the same for all children of buses
in that uclass. There are also uclass methods which can be called when
children are bound and probed.

Here an explanation of how a bus fits with a uclass may be useful. Consider
a USB bus with several devices attached to it, each from a different (made
up) uclass:

   xhci_usb (UCLASS_USB)
      eth (UCLASS_ETHERNET)
      camera (UCLASS_CAMERA)
      flash (UCLASS_FLASH_STORAGE)

Each of the devices is connected to a different address on the USB bus.
The bus device wants to store this address and some other information such
as the bus speed for each device.

To achieve this, the bus device can use dev->parent_platdata in each of its
three children. This can be auto-allocated if the bus driver (or bus uclass)
has a non-zero value for per_child_platdata_auto_alloc_size. If not, then
the bus device or uclass can allocate the space itself before the child
device is probed.

Also the bus driver can define the child_pre_probe() and child_post_remove()
methods to allow it to do some processing before the child is activated or
after it is deactivated.

Similarly the bus uclass can define the child_post_bind() method to obtain
the per-child platform data from the device tree and set it up for the child.
The bus uclass can also provide a child_pre_probe() method. Very often it is
the bus uclass that controls these features, since it avoids each driver
having to do the same processing. Of course the driver can still tweak and
override these activities.

Note that the information that controls this behaviour is in the bus's
driver, not the child's. In fact it is possible that child has no knowledge
that it is connected to a bus. The same child device may even be used on two
different bus types. As an example. the 'flash' device shown above may also
be connected on a SATA bus or standalone with no bus:

   xhci_usb (UCLASS_USB)
      flash (UCLASS_FLASH_STORAGE)  - parent data/methods defined by USB bus

   sata (UCLASS_SATA)
      flash (UCLASS_FLASH_STORAGE)  - parent data/methods defined by SATA bus

   flash (UCLASS_FLASH_STORAGE)  - no parent data/methods (not on a bus)

Above you can see that the driver for xhci_usb/sata controls the child's
bus methods. In the third example the device is not on a bus, and therefore
will not have these methods at all. Consider the case where the flash
device defines child methods. These would be used for *its* children, and
would be quite separate from the methods defined by the driver for the bus
that the flash device is connetced to. The act of attaching a device to a
parent device which is a bus, causes the device to start behaving like a
bus device, regardless of its own views on the matter.

The uclass for the device can also contain data private to that uclass.
But note that each device on the bus may be a memeber of a different
uclass, and this data has nothing to do with the child data for each child
on the bus. It is the bus' uclass that controls the child with respect to
the bus.


Driver Lifecycle
----------------

Here are the stages that a device goes through in driver model. Note that all
methods mentioned here are optional - e.g. if there is no probe() method for
a device then it will not be called. A simple device may have very few
methods actually defined.

1. Bind stage

A device and its driver are bound using one of these two methods:

   - Scan the U_BOOT_DEVICE() definitions. U-Boot It looks up the
name specified by each, to find the appropriate driver. It then calls
device_bind() to create a new device and bind' it to its driver. This will
call the device's bind() method.

   - Scan through the device tree definitions. U-Boot looks at top-level
nodes in the the device tree. It looks at the compatible string in each node
and uses the of_match part of the U_BOOT_DRIVER() structure to find the
right driver for each node. It then calls device_bind() to bind the
newly-created device to its driver (thereby creating a device structure).
This will also call the device's bind() method.

At this point all the devices are known, and bound to their drivers. There
is a 'struct udevice' allocated for all devices. However, nothing has been
activated (except for the root device). Each bound device that was created
from a U_BOOT_DEVICE() declaration will hold the platdata pointer specified
in that declaration. For a bound device created from the device tree,
platdata will be NULL, but of_offset will be the offset of the device tree
node that caused the device to be created. The uclass is set correctly for
the device.

The device's bind() method is permitted to perform simple actions, but
should not scan the device tree node, not initialise hardware, nor set up
structures or allocate memory. All of these tasks should be left for
the probe() method.

Note that compared to Linux, U-Boot's driver model has a separate step of
probe/remove which is independent of bind/unbind. This is partly because in
U-Boot it may be expensive to probe devices and we don't want to do it until
they are needed, or perhaps until after relocation.

2. Activation/probe

When a device needs to be used, U-Boot activates it, by following these
steps (see device_probe()):

   a. If priv_auto_alloc_size is non-zero, then the device-private space
   is allocated for the device and zeroed. It will be accessible as
   dev->priv. The driver can put anything it likes in there, but should use
   it for run-time information, not platform data (which should be static
   and known before the device is probed).

   b. If platdata_auto_alloc_size is non-zero, then the platform data space
   is allocated. This is only useful for device tree operation, since
   otherwise you would have to specific the platform data in the
   U_BOOT_DEVICE() declaration. The space is allocated for the device and
   zeroed. It will be accessible as dev->platdata.

   c. If the device's uclass specifies a non-zero per_device_auto_alloc_size,
   then this space is allocated and zeroed also. It is allocated for and
   stored in the device, but it is uclass data. owned by the uclass driver.
   It is possible for the device to access it.

   d. If the device's immediate parent specifies a per_child_auto_alloc_size
   then this space is allocated. This is intended for use by the parent
   device to keep track of things related to the child. For example a USB
   flash stick attached to a USB host controller would likely use this
   space. The controller can hold information about the USB state of each
   of its children.

   e. All parent devices are probed. It is not possible to activate a device
   unless its predecessors (all the way up to the root device) are activated.
   This means (for example) that an I2C driver will require that its bus
   be activated.

   f. The device's sequence number is assigned, either the requested one
   (assuming no conflicts) or the next available one if there is a conflict
   or nothing particular is requested.

   g. If the driver provides an ofdata_to_platdata() method, then this is
   called to convert the device tree data into platform data. This should
   do various calls like fdtdec_get_int(gd->fdt_blob, dev->of_offset, ...)
   to access the node and store the resulting information into dev->platdata.
   After this point, the device works the same way whether it was bound
   using a device tree node or U_BOOT_DEVICE() structure. In either case,
   the platform data is now stored in the platdata structure. Typically you
   will use the platdata_auto_alloc_size feature to specify the size of the
   platform data structure, and U-Boot will automatically allocate and zero
   it for you before entry to ofdata_to_platdata(). But if not, you can
   allocate it yourself in ofdata_to_platdata(). Note that it is preferable
   to do all the device tree decoding in ofdata_to_platdata() rather than
   in probe(). (Apart from the ugliness of mixing configuration and run-time
   data, one day it is possible that U-Boot will cache platformat data for
   devices which are regularly de/activated).

   h. The device's probe() method is called. This should do anything that
   is required by the device to get it going. This could include checking
   that the hardware is actually present, setting up clocks for the
   hardware and setting up hardware registers to initial values. The code
   in probe() can access:

      - platform data in dev->platdata (for configuration)
      - private data in dev->priv (for run-time state)
      - uclass data in dev->uclass_priv (for things the uclass stores
        about this device)

   Note: If you don't use priv_auto_alloc_size then you will need to
   allocate the priv space here yourself. The same applies also to
   platdata_auto_alloc_size. Remember to free them in the remove() method.

   i. The device is marked 'activated'

   j. The uclass's post_probe() method is called, if one exists. This may
   cause the uclass to do some housekeeping to record the device as
   activated and 'known' by the uclass.

3. Running stage

The device is now activated and can be used. From now until it is removed
all of the above structures are accessible. The device appears in the
uclass's list of devices (so if the device is in UCLASS_GPIO it will appear
as a device in the GPIO uclass). This is the 'running' state of the device.

4. Removal stage

When the device is no-longer required, you can call device_remove() to
remove it. This performs the probe steps in reverse:

   a. The uclass's pre_remove() method is called, if one exists. This may
   cause the uclass to do some housekeeping to record the device as
   deactivated and no-longer 'known' by the uclass.

   b. All the device's children are removed. It is not permitted to have
   an active child device with a non-active parent. This means that
   device_remove() is called for all the children recursively at this point.

   c. The device's remove() method is called. At this stage nothing has been
   deallocated so platform data, private data and the uclass data will all
   still be present. This is where the hardware can be shut down. It is
   intended that the device be completely inactive at this point, For U-Boot
   to be sure that no hardware is running, it should be enough to remove
   all devices.

   d. The device memory is freed (platform data, private data, uclass data,
   parent data).

   Note: Because the platform data for a U_BOOT_DEVICE() is defined with a
   static pointer, it is not de-allocated during the remove() method. For
   a device instantiated using the device tree data, the platform data will
   be dynamically allocated, and thus needs to be deallocated during the
   remove() method, either:

      1. if the platdata_auto_alloc_size is non-zero, the deallocation
      happens automatically within the driver model core; or

      2. when platdata_auto_alloc_size is 0, both the allocation (in probe()
      or preferably ofdata_to_platdata()) and the deallocation in remove()
      are the responsibility of the driver author.

   e. The device sequence number is set to -1, meaning that it no longer
   has an allocated sequence. If the device is later reactivated and that
   sequence number is still free, it may well receive the name sequence
   number again. But from this point, the sequence number previously used
   by this device will no longer exist (think of SPI bus 2 being removed
   and bus 2 is no longer available for use).

   f. The device is marked inactive. Note that it is still bound, so the
   device structure itself is not freed at this point. Should the device be
   activated again, then the cycle starts again at step 2 above.

5. Unbind stage

The device is unbound. This is the step that actually destroys the device.
If a parent has children these will be destroyed first. After this point
the device does not exist and its memory has be deallocated.


Data Structures
---------------

Driver model uses a doubly-linked list as the basic data structure. Some
nodes have several lists running through them. Creating a more efficient
data structure might be worthwhile in some rare cases, once we understand
what the bottlenecks are.


Changes since v1
----------------

For the record, this implementation uses a very similar approach to the
original patches, but makes at least the following changes:

- Tried to aggressively remove boilerplate, so that for most drivers there
is little or no 'driver model' code to write.
- Moved some data from code into data structure - e.g. store a pointer to
the driver operations structure in the driver, rather than passing it
to the driver bind function.
- Rename some structures to make them more similar to Linux (struct udevice
instead of struct instance, struct platdata, etc.)
- Change the name 'core' to 'uclass', meaning U-Boot class. It seems that
this concept relates to a class of drivers (or a subsystem). We shouldn't
use 'class' since it is a C++ reserved word, so U-Boot class (uclass) seems
better than 'core'.
- Remove 'struct driver_instance' and just use a single 'struct udevice'.
This removes a level of indirection that doesn't seem necessary.
- Built in device tree support, to avoid the need for platdata
- Removed the concept of driver relocation, and just make it possible for
the new driver (created after relocation) to access the old driver data.
I feel that relocation is a very special case and will only apply to a few
drivers, many of which can/will just re-init anyway. So the overhead of
dealing with this might not be worth it.
- Implemented a GPIO system, trying to keep it simple


Pre-Relocation Support
----------------------

For pre-relocation we simply call the driver model init function. Only
drivers marked with DM_FLAG_PRE_RELOC or the device tree
'u-boot,dm-pre-reloc' flag are initialised prior to relocation. This helps
to reduce the driver model overhead.

Then post relocation we throw that away and re-init driver model again.
For drivers which require some sort of continuity between pre- and
post-relocation devices, we can provide access to the pre-relocation
device pointers, but this is not currently implemented (the root device
pointer is saved but not made available through the driver model API).


SPL Support
-----------

Driver model can operate in SPL. Its efficient implementation and small code
size provide for a small overhead which is acceptable for all but the most
constrained systems.

To enable driver model in SPL, define CONFIG_SPL_DM. You might want to
consider the following option also. See the main README for more details.

   - CONFIG_SYS_MALLOC_SIMPLE
   - CONFIG_DM_WARN
   - CONFIG_DM_DEVICE_REMOVE
   - CONFIG_DM_STDIO


Enabling Driver Model
---------------------

Driver model is being brought into U-Boot gradually. As each subsystems gets
support, a uclass is created and a CONFIG to enable use of driver model for
that subsystem.

For example CONFIG_DM_SERIAL enables driver model for serial. With that
defined, the old serial support is not enabled, and your serial driver must
conform to driver model. With that undefined, the old serial support is
enabled and driver model is not available for serial. This means that when
you convert a driver, you must either convert all its boards, or provide for
the driver to be compiled both with and without driver model (generally this
is not very hard).

See the main README for full details of the available driver model CONFIG
options.


Things to punt for later
------------------------

Uclasses are statically numbered at compile time. It would be possible to
change this to dynamic numbering, but then we would require some sort of
lookup service, perhaps searching by name. This is slightly less efficient
so has been left out for now. One small advantage of dynamic numbering might
be fewer merge conflicts in uclass-id.h.


Simon Glass
sjg@chromium.org
April 2013
Updated 7-May-13
Updated 14-Jun-13
Updated 18-Oct-13
Updated 5-Nov-13
