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
	a way of accessing invidual devices within the group, but always
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
the usual command procesing and then:

	struct device *demo_dev;

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

int demo_hello(struct device *dev, int ch)
{
	const struct demo_ops *ops = device_get_ops(dev);

	if (!ops->hello)
		return -ENOSYS;

	return ops->hello(dev, ch);
}

As you can see it just calls the relevant driver method. One of these is
in drivers/demo/demo-simple.c:

static int simple_hello(struct device *dev, int ch)
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

Where does the platform data come from? See demo-pdata.c which
sets up a table of driver names and their associated platform data.
The data can be interpreted by the drivers however they like - it is
basically a communication scheme between the board-specific code and
the generic drivers, which are intended to work on any board.

Drivers can acceess their data via dev->info->platdata. Here is
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


The easiest way to make this work it to add a few members to the driver:

	.platdata_auto_alloc_size = sizeof(struct dm_test_pdata),
	.ofdata_to_platdata = testfdt_ofdata_to_platdata,
	.probe	= testfdt_drv_probe,

The 'auto_alloc' feature allowed space for the platdata to be allocated
and zeroed before the driver's ofdata_to_platdata method is called. This
method reads the information out of the device tree and puts it in
dev->platdata. Then the probe method is called to set up the device.

Note that both methods are optional. If you provide an ofdata_to_platdata
method then it wlil be called first (after bind). If you provide a probe
method it will be called next.

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

- Tried to agressively remove boilerplate, so that for most drivers there
is little or no 'driver model' code to write.
- Moved some data from code into data structure - e.g. store a pointer to
the driver operations structure in the driver, rather than passing it
to the driver bind function.
- Rename some structures to make them more similar to Linux (struct device
instead of struct instance, struct platdata, etc.)
- Change the name 'core' to 'uclass', meaning U-Boot class. It seems that
this concept relates to a class of drivers (or a subsystem). We shouldn't
use 'class' since it is a C++ reserved word, so U-Boot class (uclass) seems
better than 'core'.
- Remove 'struct driver_instance' and just use a single 'struct device'.
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
