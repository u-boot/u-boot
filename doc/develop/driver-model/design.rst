.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Design Details
==============

This README contains high-level information about driver model, a unified
way of declaring and accessing drivers in U-Boot. The original work was done
by:

   * Marek Vasut <marex@denx.de>
   * Pavel Herrmann <morpheus.ibis@gmail.com>
   * Viktor Křivák <viktor.krivak@gmail.com>
   * Tomas Hlavacek <tmshlvck@gmail.com>

This has been both simplified and extended into the current implementation
by:

   * Simon Glass <sjg@chromium.org>


Terminology
-----------

Uclass
  a group of devices which operate in the same way. A uclass provides
  a way of accessing individual devices within the group, but always
  using the same interface. For example a GPIO uclass provides
  operations for get/set value. An I2C uclass may have 10 I2C ports,
  4 with one driver, and 6 with another.

Driver
  some code which talks to a peripheral and presents a higher-level
  interface to it.

Device
  an instance of a driver, tied to a particular port or peripheral.


How to try it
-------------

Build U-Boot sandbox and run it::

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
handles parameter data and plat (data which tells the driver how
to operate on a particular platform) and it uses private driver data.

To try it, see the example session below::

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
are provided in test/dm. To run them, try::

   ./test/py/test.py --bd sandbox --build -k ut_dm -v

You should see something like this::

   (venv)$ ./test/py/test.py --bd sandbox --build -k ut_dm -v
   +make O=/root/u-boot/build-sandbox -s sandbox_defconfig
   +make O=/root/u-boot/build-sandbox -s -j8
   ============================= test session starts ==============================
   platform linux2 -- Python 2.7.5, pytest-2.9.0, py-1.4.31, pluggy-0.3.1 -- /root/u-boot/venv/bin/python
   cachedir: .cache
   rootdir: /root/u-boot, inifile:
   collected 199 items

   test/py/tests/test_ut.py::test_ut_dm_init PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_adc_bind] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_adc_multi_channel_conversion] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_adc_multi_channel_shot] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_adc_single_channel_conversion] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_adc_single_channel_shot] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_adc_supply] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_adc_wrong_channel_selection] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_autobind] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_autobind_uclass_pdata_alloc] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_autobind_uclass_pdata_valid] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_autoprobe] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_child_post_bind] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_child_post_bind_uclass] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_child_pre_probe_uclass] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_children] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_children_funcs] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_children_iterators] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_parent_data] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_parent_data_uclass] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_parent_ops] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_parent_platdata] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_bus_parent_platdata_uclass] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_children] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_clk_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_clk_periph] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_device_get_uclass_id] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_eth] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_eth_act] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_eth_alias] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_eth_prime] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_eth_rotate] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_fdt] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_fdt_offset] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_fdt_pre_reloc] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_fdt_uclass_seq] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_gpio] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_gpio_anon] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_gpio_copy] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_gpio_leak] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_gpio_phandles] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_gpio_requestf] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_i2c_bytewise] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_i2c_find] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_i2c_offset] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_i2c_offset_len] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_i2c_probe_empty] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_i2c_read_write] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_i2c_speed] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_leak] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_led_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_led_gpio] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_led_label] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_lifecycle] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_mmc_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_net_retry] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_operations] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_ordering] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_pci_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_pci_busnum] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_pci_swapcase] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_platdata] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_power_pmic_get] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_power_pmic_io] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_power_regulator_autoset] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_power_regulator_autoset_list] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_power_regulator_get] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_power_regulator_set_get_current] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_power_regulator_set_get_enable] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_power_regulator_set_get_mode] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_power_regulator_set_get_voltage] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_pre_reloc] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_ram_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_regmap_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_regmap_syscon] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_remoteproc_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_remove] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_reset_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_reset_walk] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_rtc_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_rtc_dual] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_rtc_reset] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_rtc_set_get] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_spi_find] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_spi_flash] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_spi_xfer] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_syscon_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_syscon_by_driver_data] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_timer_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_uclass] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_uclass_before_ready] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_uclass_devices_find] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_uclass_devices_find_by_name] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_uclass_devices_get] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_uclass_devices_get_by_name] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_usb_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_usb_flash] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_usb_keyb] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_usb_multi] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_usb_remove] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_usb_tree] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_usb_tree_remove] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_usb_tree_reorder] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_base] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_bmp] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_bmp_comp] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_chars] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_context] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_rotation1] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_rotation2] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_rotation3] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_text] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_truetype] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_truetype_bs] PASSED
   test/py/tests/test_ut.py::test_ut[ut_dm_video_truetype_scroll] PASSED

   ======================= 84 tests deselected by '-kut_dm' =======================
   ================== 115 passed, 84 deselected in 3.77 seconds ===================

What is going on?
-----------------

Let's start at the top. The demo command is in cmd/demo.c. It does
the usual command processing and then:

.. code-block:: c

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

.. code-block:: c

	return demo_hello(demo_dev, ch);

This function is in the demo uclass. It takes care of calling the 'hello'
method of the relevant driver. Bearing in mind that there are two drivers,
this particular device may use one or other of them.

The code for demo_hello() is in drivers/demo/demo-uclass.c:

.. code-block:: c

	int demo_hello(struct udevice *dev, int ch)
	{
		const struct demo_ops *ops = device_get_ops(dev);

		if (!ops->hello)
			return -ENOSYS;

		return ops->hello(dev, ch);
	}

As you can see it just calls the relevant driver method. One of these is
in drivers/demo/demo-simple.c:

.. code-block:: c

	static int simple_hello(struct udevice *dev, int ch)
	{
		const struct dm_demo_pdata *pdata = dev_get_plat(dev);

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

.. code-block:: c

	static const struct demo_ops shape_ops = {
		.hello = shape_hello,
		.status = shape_status,
	};

	U_BOOT_DRIVER(demo_shape_drv) = {
		.name	= "demo_shape_drv",
		.id	= UCLASS_DEMO,
		.ops	= &shape_ops,
		.priv_auto = sizeof(struct shape_data),
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

   * bind - make the driver model aware of a device (bind it to its driver)
   * unbind - make the driver model forget the device
   * of_to_plat - convert device tree data to plat - see later
   * probe - make a device ready for use
   * remove - remove a device so it cannot be used until probed again

The sequence to get a device to work is bind, of_to_plat (if using
device tree) and probe.


Platform Data
-------------

Note: platform data is the old way of doing things. It is
basically a C structure which is passed to drivers to tell them about
platform-specific settings like the address of its registers, bus
speed, etc. Device tree is now the preferred way of handling this.
Unless you have a good reason not to use device tree (the main one
being you need serial support in SPL and don't have enough SRAM for
the cut-down device tree and libfdt libraries) you should stay away
from platform data.

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

Drivers can access their data via dev->info->plat. Here is
the declaration for the platform data, which would normally appear
in the board file.

.. code-block:: c

	static const struct dm_demo_pdata red_square = {
		.colour = "red",
		.sides = 4.
	};

	static const struct driver_info info[] = {
		{
			.name = "demo_shape_drv",
			.plat = &red_square,
		},
	};

	demo1 = driver_bind(root, &info[0]);


Device Tree
-----------

While plat is useful, a more flexible way of providing device data is
by using device tree. In U-Boot you should use this where possible. Avoid
sending patches which make use of the U_BOOT_DRVINFO() macro unless strictly
necessary.

With device tree we replace the above code with the following device tree
fragment:

.. code-block:: c

	red-square {
		compatible = "demo-shape";
		colour = "red";
		sides = <4>;
	};

This means that instead of having lots of U_BOOT_DRVINFO() declarations in
the board file, we put these in the device tree. This approach allows a lot
more generality, since the same board file can support many types of boards
(e,g. with the same SoC) just by using different device trees. An added
benefit is that the Linux device tree can be used, thus further simplifying
the task of board-bring up either for U-Boot or Linux devs (whoever gets to
the board first!).

The easiest way to make this work it to add a few members to the driver:

.. code-block:: c

	.plat_auto = sizeof(struct dm_test_pdata),
	.of_to_plat = testfdt_of_to_plat,

The 'auto' feature allowed space for the plat to be allocated
and zeroed before the driver's of_to_plat() method is called. The
of_to_plat() method, which the driver write supplies, should parse
the device tree node for this device and place it in dev->plat. Thus
when the probe method is called later (to set up the device ready for use)
the platform data will be present.

Note that both methods are optional. If you provide an of_to_plat
method then it will be called first (during activation). If you provide a
probe method it will be called next. See Driver Lifecycle below for more
details.

If you don't want to have the plat automatically allocated then you
can leave out plat_auto. In this case you can use malloc
in your of_to_plat (or probe) method to allocate the required memory,
and you should free it in the remove method.

The driver model tree is intended to mirror that of the device tree. The
root driver is at device tree offset 0 (the root node, '/'), and its
children are the children of the root node.

In order for a device tree to be valid, the content must be correct with
respect to either device tree specification
(https://www.devicetree.org/specifications/) or the device tree bindings that
are found in the doc/device-tree-bindings directory.  When not U-Boot specific
the bindings in this directory tend to come from the Linux Kernel.  As such
certain design decisions may have been made already for us in terms of how
specific devices are described and bound.  In most circumstances we wish to
retain compatibility without additional changes being made to the device tree
source files.

Declaring Uclasses
------------------

The demo uclass is declared like this:

.. code-block:: c

	UCLASS_DRIVER(demo) = {
		.id		= UCLASS_DEMO,
	};

It is also possible to specify special methods for probe, etc. The uclass
numbering comes from include/dm/uclass-id.h. To add a new uclass, add to the
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

Where a device gets its sequence number is controlled by the DM_SEQ_ALIAS
Kconfig option, which can have a different value in U-Boot proper and SPL.
If this option is not set, aliases are ignored.

Even if CONFIG_DM_SEQ_ALIAS is enabled, the uclass must still have the
DM_UC_FLAG_SEQ_ALIAS flag set, for its devices to be sequenced by aliases.

With those options set, devices with an alias (e.g. "serial2") will get that
sequence number (e.g. 2). Other devices get the next available number after all
aliases and all existing numbers. This means that if there is just a single
alias "serial2", unaliased serial devices will be assigned 3 or more, with 0 and
1 being unused.

If CONFIG_DM_SEQ_ALIAS or DM_UC_FLAG_SEQ_ALIAS are not set, all devices will get
sequence numbers in a simple ordering starting from 0. To find the next number
to allocate, driver model scans through to find the maximum existing number,
then uses the next one. It does not attempt to fill in gaps.

.. code-block:: none

	aliases {
		serial2 = "/serial@22230000";
	};

This indicates that in the uclass called "serial", the named node
("/serial@22230000") will be given sequence number 2. Any command or driver
which requests serial device 2 will obtain this device.

More commonly you can use node references, which expand to the full path:

.. code-block:: none

	aliases {
		serial2 = &serial_2;
	};
	...
	serial_2: serial@22230000 {
	...
	};

The alias resolves to the same string in this case, but this version is
easier to read.

Device sequence numbers are resolved when a device is bound and the number does
not change for the life of the device.

There are some situations where the uclass must allocate sequence numbers,
since a strictly increase sequence (with devicetree nodes bound first) is not
suitable. An example of this is the PCI bus. In this case, you can set the
uclass DM_UC_FLAG_NO_AUTO_SEQ flag. With this flag set, only devices with an
alias will be assigned a number by driver model. The rest is left to the uclass
to sort out, e.g. when enumerating the bus.

Note that changing the sequence number for a device (e.g. in a driver) is not
permitted. If it is felt to be necessary, ask on the mailing list.

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
up) uclass::

   xhci_usb (UCLASS_USB)
      eth (UCLASS_ETH)
      camera (UCLASS_CAMERA)
      flash (UCLASS_FLASH_STORAGE)

Each of the devices is connected to a different address on the USB bus.
The bus device wants to store this address and some other information such
as the bus speed for each device.

To achieve this, the bus device can use dev->parent_plat in each of its
three children. This can be auto-allocated if the bus driver (or bus uclass)
has a non-zero value for per_child_plat_auto. If not, then
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
be connected on a SATA bus or standalone with no bus::

   xhci_usb (UCLASS_USB)
      flash (UCLASS_FLASH_STORAGE)  - parent data/methods defined by USB bus

   sata (UCLASS_AHCI)
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
But note that each device on the bus may be a member of a different
uclass, and this data has nothing to do with the child data for each child
on the bus. It is the bus' uclass that controls the child with respect to
the bus.


Driver Lifecycle
----------------

Here are the stages that a device goes through in driver model. Note that all
methods mentioned here are optional - e.g. if there is no probe() method for
a device then it will not be called. A simple device may have very few
methods actually defined.

Bind stage
^^^^^^^^^^

U-Boot discovers devices using one of these two methods:

- Scan the U_BOOT_DRVINFO() definitions. U-Boot looks up the name specified
  by each, to find the appropriate U_BOOT_DRIVER() definition. In this case,
  there is no path by which driver_data may be provided, but the U_BOOT_DRVINFO()
  may provide plat.

- Scan through the device tree definitions. U-Boot looks at top-level
  nodes in the the device tree. It looks at the compatible string in each node
  and uses the of_match table of the U_BOOT_DRIVER() structure to find the
  right driver for each node. In this case, the of_match table may provide a
  driver_data value, but plat cannot be provided until later.

For each device that is discovered, U-Boot then calls device_bind() to create a
new device, initializes various core fields of the device object such as name,
uclass & driver, initializes any optional fields of the device object that are
applicable such as of_offset, driver_data & plat, and finally calls the
driver's bind() method if one is defined.

At this point all the devices are known, and bound to their drivers. There
is a 'struct udevice' allocated for all devices. However, nothing has been
activated (except for the root device). Each bound device that was created
from a U_BOOT_DRVINFO() declaration will hold the plat pointer specified
in that declaration. For a bound device created from the device tree,
plat will be NULL, but of_offset will be the offset of the device tree
node that caused the device to be created. The uclass is set correctly for
the device.

The device's sequence number is assigned, either the requested one or the next
available one (after all aliases are processed) if nothing particular is
requested.

The device's bind() method is permitted to perform simple actions, but
should not scan the device tree node, not initialise hardware, nor set up
structures or allocate memory. All of these tasks should be left for
the probe() method.

Note that compared to Linux, U-Boot's driver model has a separate step of
probe/remove which is independent of bind/unbind. This is partly because in
U-Boot it may be expensive to probe devices and we don't want to do it until
they are needed, or perhaps until after relocation.

Reading ofdata
^^^^^^^^^^^^^^

Most devices have data in the device tree which they can read to find out the
base address of hardware registers and parameters relating to driver
operation. This is called 'ofdata' (Open-Firmware data).

The device's of_to_plat() implemnents allocation and reading of
plat. A parent's ofdata is always read before a child.

The steps are:

   1. If priv_auto is non-zero, then the device-private space
   is allocated for the device and zeroed. It will be accessible as
   dev->priv. The driver can put anything it likes in there, but should use
   it for run-time information, not platform data (which should be static
   and known before the device is probed).

   2. If plat_auto is non-zero, then the platform data space
   is allocated. This is only useful for device tree operation, since
   otherwise you would have to specify the platform data in the
   U_BOOT_DRVINFO() declaration. The space is allocated for the device and
   zeroed. It will be accessible as dev->plat.

   3. If the device's uclass specifies a non-zero per_device_auto,
   then this space is allocated and zeroed also. It is allocated for and
   stored in the device, but it is uclass data. owned by the uclass driver.
   It is possible for the device to access it.

   4. If the device's immediate parent specifies a per_child_auto
   then this space is allocated. This is intended for use by the parent
   device to keep track of things related to the child. For example a USB
   flash stick attached to a USB host controller would likely use this
   space. The controller can hold information about the USB state of each
   of its children.

   5. If the driver provides an of_to_plat() method, then this is
   called to convert the device tree data into platform data. This should
   do various calls like dev_read_u32(dev, ...) to access the node and store
   the resulting information into dev->plat. After this point, the device
   works the same way whether it was bound using a device tree node or
   U_BOOT_DRVINFO() structure. In either case, the platform data is now stored
   in the plat structure. Typically you will use the
   plat_auto feature to specify the size of the platform data
   structure, and U-Boot will automatically allocate and zero it for you before
   entry to of_to_plat(). But if not, you can allocate it yourself in
   of_to_plat(). Note that it is preferable to do all the device tree
   decoding in of_to_plat() rather than in probe(). (Apart from the
   ugliness of mixing configuration and run-time data, one day it is possible
   that U-Boot will cache platform data for devices which are regularly
   de/activated).

   6. The device is marked 'plat valid'.

Note that ofdata reading is always done (for a child and all its parents)
before probing starts. Thus devices go through two distinct states when
probing: reading platform data and actually touching the hardware to bring
the device up.

Having probing separate from ofdata-reading helps deal with of-platdata, where
the probe() method is common to both DT/of-platdata operation, but the
of_to_plat() method is implemented differently.

Another case has come up where this separate is useful. Generation of ACPI
tables uses the of-platdata but does not want to probe the device. Probing
would cause U-Boot to violate one of its design principles, viz that it
should only probe devices that are used. For ACPI we want to generate a
table for each device, even if U-Boot does not use it. In fact it may not
even be possible to probe the device - e.g. an SD card which is not
present will cause an error on probe, yet we still must tell Linux about
the SD card connector in case it is used while Linux is running.

It is important that the of_to_plat() method does not actually probe
the device itself. However there are cases where other devices must be probed
in the of_to_plat() method. An example is where a device requires a
GPIO for it to operate. To select a GPIO obviously requires that the GPIO
device is probed. This is OK when used by common, core devices such as GPIO,
clock, interrupts, reset and the like.

If your device relies on its parent setting up a suitable address space, so
that dev_read_addr() works correctly, then make sure that the parent device
has its setup code in of_to_plat(). If it has it in the probe method,
then you cannot call dev_read_addr() from the child device's
of_to_plat() method. Move it to probe() instead. Buses like PCI can
fall afoul of this rule.

Activation/probe
^^^^^^^^^^^^^^^^

To save resources devices in U-Boot are probed lazily. U-Boot is a bootloader,
not an operating system. Many devices are never used during an U-Boot run, and
probing them takes time, requires memory, may add delays to the main loop, etc.

The device should be probed by the uclass code or generic device code (e.g.
device_find_global_by_ofnode()). Uclasses differ but two common use cases can be
seen:

   1. The uclass is asked to look up a specific device, such as SPI bus 0,
      first chip select - in this case the returned device should be
      activated.

   2. The uclass is asked to perform a specific function on any device that
      supports it, eg. reset the board using any sysreset that can be found -
      for this case the core uclass code provides iterators that activate
      each device before returning it, and the uclass typically implements a
      walk function that iterates over all devices of the uclass and tries
      to perform the requested function on each in turn until succesful.

To activate a device U-Boot first reads ofdata as above and then follows these
steps (see device_probe()):

   1. All parent devices are probed. It is not possible to activate a device
      unless its predecessors (all the way up to the root device) are activated.
      This means (for example) that an I2C driver will require that its bus
      be activated.

   2. The device's probe() method is called. This should do anything that
      is required by the device to get it going. This could include checking
      that the hardware is actually present, setting up clocks for the
      hardware and setting up hardware registers to initial values. The code
      in probe() can access:

      - platform data in dev->plat (for configuration)
      - private data in dev->priv (for run-time state)
      - uclass data in dev->uclass_priv (for things the uclass stores
        about this device)

      Note: If you don't use priv_auto then you will need to
      allocate the priv space here yourself. The same applies also to
      plat_auto. Remember to free them in the remove() method.

   3. The device is marked 'activated'

   4. The uclass's post_probe() method is called, if one exists. This may
      cause the uclass to do some housekeeping to record the device as
      activated and 'known' by the uclass.

For some platforms, certain devices must be probed to get the platform into
a working state. To help with this, devices marked with DM_FLAG_PROBE_AFTER_BIND
will be probed immediately after all devices are bound. This flag must be set
on the device in its ``bind()`` function with
``dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND)``. For now, this happens in
SPL, before relocation and after relocation. See the call to ``dm_autoprobe()``
for where this is done.

The auto-probe feature is tricky because it bypasses the normal ordering of
probing. General, if device A (e.g. video) needs device B (e.g. clock), then
A's probe() method uses ``clk_get_by_index()`` and B is probed before A. But
A is only probed when it is used. Therefore care should be taken when using
auto-probe, limiting it to devices which truly are essential, such as power
domains or critical clocks.

See here for more discussion of this feature:

:Link: https://patchwork.ozlabs.org/project/uboot/patch/20240626235717.272219-1-marex@denx.de/

Running stage
^^^^^^^^^^^^^

The device is now activated and can be used. From now until it is removed
all of the above structures are accessible. The device appears in the
uclass's list of devices (so if the device is in UCLASS_GPIO it will appear
as a device in the GPIO uclass). This is the 'running' state of the device.

Removal stage
^^^^^^^^^^^^^

When the device is no-longer required, you can call device_remove() to
remove it. This performs the probe steps in reverse:

   1. The uclass's pre_remove() method is called, if one exists. This may
   cause the uclass to do some housekeeping to record the device as
   deactivated and no-longer 'known' by the uclass.

   2. All the device's children are removed. It is not permitted to have
   an active child device with a non-active parent. This means that
   device_remove() is called for all the children recursively at this point.

   3. The device's remove() method is called. At this stage nothing has been
   deallocated so platform data, private data and the uclass data will all
   still be present. This is where the hardware can be shut down. It is
   intended that the device be completely inactive at this point, For U-Boot
   to be sure that no hardware is running, it should be enough to remove
   all devices.

   4. The device memory is freed (platform data, private data, uclass data,
   parent data).

   Note: Because the platform data for a U_BOOT_DRVINFO() is defined with a
   static pointer, it is not de-allocated during the remove() method. For
   a device instantiated using the device tree data, the platform data will
   be dynamically allocated, and thus needs to be deallocated during the
   remove() method, either:

      - if the plat_auto is non-zero, the deallocation happens automatically
        within the driver model core in the unbind stage; or

      - when plat_auto is 0, both the allocation (in probe()
        or preferably of_to_plat()) and the deallocation in remove()
        are the responsibility of the driver author.

   5. The device is marked inactive. Note that it is still bound, so the
   device structure itself is not freed at this point. Should the device be
   activated again, then the cycle starts again at step 2 above.

Unbind stage
^^^^^^^^^^^^

The device is unbound. This is the step that actually destroys the device.
If a parent has children these will be destroyed first. After this point
the device does not exist and its memory has be deallocated.


Special cases for removal
-------------------------

Some devices need to do clean-up before the OS is called. For example, a USB
driver may want to stop the bus. This can be done in the remove() method.
Some special flags are used to determine whether to remove the device:

   DM_FLAG_OS_PREPARE - indicates that the device needs to get ready for OS
          boot. The device will be removed just before the OS is booted
   DM_REMOVE_ACTIVE_DMA - indicates that the device uses DMA. This is
          effectively the same as DM_FLAG_OS_PREPARE, so the device is removed
          before the OS is booted
   DM_FLAG_VITAL - indicates that the device is 'vital' to the operation of
          other devices. It is possible to remove this device after all regular
          devices are removed. This is useful e.g. for a clock, which need to
          be active during the device-removal phase.

The dm_remove_devices_flags() function can be used to remove devices based on
their driver flags.


Error codes
-----------

Driver model tries to use errors codes in a consistent way, as follows:

\-EAGAIN
   Try later, e.g. dependencies not ready

\-EINVAL
   Invalid argument, such as `dev_read_...()` failed or any other
   devicetree-related access. Also used when a driver method is passed an
   argument it considers invalid or does not support.

\-EIO
   Failed to perform an I/O operation. This is used when a local device
   (i.e. part of the SOC) does not work as expected. Use -EREMOTEIO for
   failures to talk to a separate device, e.g. over an I2C or SPI
   channel.

\-ENODEV
   Do not bind the device. This should not be used to indicate an
   error probing the device or for any other purpose, lest driver model get
   confused. Using `-ENODEV` inside a driver method makes no sense, since
   clearly there is a device.

\-ENOENT
   Entry or object not found. This is used when a device, file or directory
   cannot be found (e.g. when looked up by name), It can also indicate a
   missing devicetree subnode.

\-ENOMEM
   Out of memory

\-ENOSPC
   Ran out of space (e.g. in a buffer or limited-size array)

\-ENOSYS
   Function not implemented. This is returned by uclasses where the driver does
   not implement a particular method. It can also be returned by drivers when
   a particular sub-method is not implemented. This is widely checked in the
   wider code base, where a feature may or may not be compiled into U-Boot. It
   indicates that the feature is not available, but this is often just normal
   operation. Please do not use -ENOSUPP. If an incorrect or unknown argument
   is provided to a method (e.g. an unknown clock ID), return -EINVAL.

\-ENXIO
   Couldn't find device/address. This is used when a device or address
   could not be obtained or is not valid. It is often used to indicate a
   different type of problem, if -ENOENT is already used for something else in
   the driver.

\-EPERM
   This is -1 so some older code may use it as a generic error. This indicates
   that an operation is not permitted, e.g. a security violation or policy
   constraint. It is returned internally when binding devices before relocation,
   if the device is not marked for pre-relocation use.

\-EPFNOSUPPORT
   Missing uclass. This is deliberately an uncommon error code so that it can
   easily be distinguished. If you see this very early in U-Boot, it means that
   a device exists with a particular uclass but the uclass does not (mostly
   likely because it is not compiled in). Enable DEBUG in uclass.c or lists.c
   to see which uclass ID or driver is causing the problem.

\-EREMOTEIO
   This indicates an error in talking to a peripheral over a comms link, such
   as I2C or SPI. It might indicate that the device is not present or is not
   responding as expected.

\-ETIMEDOUT
   Hardware access or some other operation has timed out. This is used where
   there is an expected time of response and that was exceeded by enough of
   a margin that there is probably something wrong.


Less common ones:

\-ECOMM
   Not widely used, but similar to -EREMOTEIO. Can be useful as a secondary
   error to distinguish the problem from -EREMOTEIO.

\-EKEYREJECTED
   Attempt to remove a device which does not match the removal flags. See
   device_remove().

\-EILSEQ
   Devicetree read failure, specifically trying to read a string index which
   does not exist, in a string-listg property

\-ENOEXEC
   Attempt to use a uclass method on a device not in that uclass. This is
   seldom checked at present, since it is generally a programming error and a
   waste of code space. A DEBUG-only check would be useful here.

\-ENODATA
   Devicetree read error, where a property exists but has no data associated
   with it

\-EOVERFLOW
   Devicetree read error, where the property is longer than expected

\-EPROBE_DEFER
   Attempt to remove a non-vital device when the removal flags indicate that
   only vital devices should be removed

\-ERANGE
   Returned by regmap functions when arguments are out of range. This can be
   useful for disinguishing regmap errors from other errors obtained while
   probing devices.

Drivers should use the same conventions so that things function as expected.
In particular, if a driver fails to probe, or a uclass operation fails, the
error code is the primary way to indicate what actually happened.

Printing error messages in drivers is discouraged due to code size bloat and
since it can result in messages appearing in normal operation. For example, if
a command tries two different devices and uses whichever one probes correctly,
we don't want an error message displayed, even if the command itself might show
a warning or informational message. Ideally, messages in drivers should only be
displayed when debugging, e.g. by using log_debug() although in extreme cases
log_warning() or log_error() may be used.

Error messages can be logged using `log_msg_ret()`, so that enabling
`CONFIG_LOG` and `CONFIG_LOG_ERROR_RETURN` shows a trace of error codes returned
through the call stack. That can be a handy way of quickly figuring out where
an error occurred. Get into the habit of return errors with
`return log_msg_ret("here", ret)` instead of just `return ret`. The string
just needs to be long enough to find in a single function, since a log record
stores (and can print with `CONFIG_LOGF_FUNC`) the function where it was
generated.


Data Structures
---------------

Driver model uses a doubly-linked list as the basic data structure. Some
nodes have several lists running through them. Creating a more efficient
data structure might be worthwhile in some rare cases, once we understand
what the bottlenecks are.


Tag Support
-----------

It is sometimes useful for a subsystem to associate its own private
data (or object) to a DM device, i.e. struct udevice, to support
additional features.

Tag support in driver model will give us the ability to do so dynamically
instead of modifying "udevice" data structure. In the initial release, we
will support two type of attributes:

- a pointer with dm_tag_set_ptr(), and
- an unsigned long with dm_tag_set_val()

For example, UEFI subsystem utilizes the feature to maintain efi_disk
objects depending on linked udevice's lifecycle.

While the current implementation is quite simple, it will get evolved
as the feature is more extensively used in U-Boot subsystems.


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
  instead of struct instance, struct plat, etc.)
- Change the name 'core' to 'uclass', meaning U-Boot class. It seems that
  this concept relates to a class of drivers (or a subsystem). We shouldn't
  use 'class' since it is a C++ reserved word, so U-Boot class (uclass) seems
  better than 'core'.
- Remove 'struct driver_instance' and just use a single 'struct udevice'.
  This removes a level of indirection that doesn't seem necessary.
- Built in device tree support, to avoid the need for plat
- Removed the concept of driver relocation, and just make it possible for
  the new driver (created after relocation) to access the old driver data.
  I feel that relocation is a very special case and will only apply to a few
  drivers, many of which can/will just re-init anyway. So the overhead of
  dealing with this might not be worth it.
- Implemented a GPIO system, trying to keep it simple


Pre-Relocation Support
----------------------

For pre-relocation we simply call the driver model init function. Only
drivers marked with DM_FLAG_PRE_RELOC or the device tree 'bootph-all'
property are initialised prior to relocation. This helps to reduce the driver
model overhead. This flag applies to SPL and TPL as well, if device tree is
enabled (CONFIG_OF_CONTROL) there.

Note when device tree is enabled, the device tree 'bootph-all'
property can provide better control granularity on which device is bound
before relocation. While with DM_FLAG_PRE_RELOC flag of the driver all
devices with the same driver are bound, which requires allocation a large
amount of memory. When device tree is not used, DM_FLAG_PRE_RELOC is the
only way for statically declared devices via U_BOOT_DRVINFO() to be bound
prior to relocation.

It is possible to limit this to specific relocation steps, by using
the more specialized 'bootph-pre-ram' and 'bootph-pre-sram' flags
in the device tree node. For U-Boot proper you can use 'bootph-some-ram'
which means that it will be processed (and a driver bound) in U-Boot proper
prior to relocation, but will not be available in SPL or TPL.

To reduce the size of SPL and TPL, only the nodes with pre-relocation
properties ('bootph-all', 'bootph-pre-ram' or 'bootph-pre-sram') are kept in
their device trees (see README.SPL for details); the remaining nodes are
always bound.

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

   - CONFIG_SPL_SYS_MALLOC_SIMPLE
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
