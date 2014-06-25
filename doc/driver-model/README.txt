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

   make sandbox_config
   make
   ./u-boot

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
    Running 12 driver model tests
    Test: dm_test_autobind
    Test: dm_test_autoprobe
    Test: dm_test_children
    Test: dm_test_fdt
    Test: dm_test_gpio
    sandbox_gpio: sb_gpio_get_value: error: offset 4 not reserved
    Test: dm_test_leak
    Warning: Please add '#define DEBUG' to the top of common/dlmalloc.c
    Warning: Please add '#define DEBUG' to the top of common/dlmalloc.c
    Test: dm_test_lifecycle
    Test: dm_test_operations
    Test: dm_test_ordering
    Test: dm_test_platdata
    Test: dm_test_remove
    Test: dm_test_uclass
    Failures: 0

(You can add '#define DEBUG' as suggested to check for memory leaks)


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


Declaring Uclasses
------------------

The demo uclass is declared like this:

U_BOOT_CLASS(demo) = {
	.id		= UCLASS_DEMO,
};

It is also possible to specify special methods for probe, etc. The uclass
numbering comes from include/dm/uclass.h. To add a new uclass, add to the
end of the enum there, then declare your uclass as above.


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

   d. All parent devices are probed. It is not possible to activate a device
   unless its predecessors (all the way up to the root device) are activated.
   This means (for example) that an I2C driver will require that its bus
   be activated.

   e. If the driver provides an ofdata_to_platdata() method, then this is
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

   f. The device's probe() method is called. This should do anything that
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

   g. The device is marked 'activated'

   h. The uclass's post_probe() method is called, if one exists. This may
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

   d. The device memory is freed (platform data, private data, uclass data).

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

   e. The device is marked inactive. Note that it is still bound, so the
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


Things to punt for later
------------------------

- SPL support - this will have to be present before many drivers can be
converted, but it seems like we can add it once we are happy with the
core implementation.
- Pre-relocation support - similar story

That is not to say that no thinking has gone into these - in fact there
is quite a lot there. However, getting these right is non-trivial and
there is a high cost associated with going down the wrong path.

For SPL, it may be possible to fit in a simplified driver model with only
bind and probe methods, to reduce size.

For pre-relocation we can simply call the driver model init function. Then
post relocation we throw that away and re-init driver model again. For drivers
which require some sort of continuity between pre- and post-relocation
devices, we can provide access to the pre-relocation device pointers.

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
