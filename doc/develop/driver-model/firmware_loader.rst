.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2018-2019 Intel Corporation <www.intel.com>

Firmware Loader
===========================

This is firmware loader for U-Boot framework, which has very close
to some Linux Firmware API. For the details of Linux Firmware API, you can refer
to https://docs.kernel.org/driver-api/firmware/index.html.

File system firmware loader can be used to load whatever(firmware, image,
and binary) from the storage device in file system format into target location
such as memory, then consumer driver such as FPGA driver can program FPGA image
from the target location into FPGA.

FIP firmware loader can be used to load a firmware from a specific FIP entry
referenced by the specific UUID into target location such as memory, then
consumer driver such as PHY driver can load such firmware to make the PHY
operational.

To enable file system firmware loader, CONFIG_FS_LOADER need to be set at
<board_name>_defconfig such as "CONFIG_FS_LOADER=y".

To enable FIP firmware loader, CONFIG_FIP_LOADER need to be set at
<board_name>_defconfig such as "CONFIG_FIP_LOADER=y".

Firmware Loader API core features
---------------------------------

Firmware storage device described in device tree source
-------------------------------------------------------
For passing data like storage device phandle and partition where the
firmware loading from to the firmware loader driver, those data could be
defined in fw-loader node as shown in below:

Example for block device::

	fw_loader0: fs-loader {
		bootph-all;
		compatible = "u-boot,fs-loader";
		phandlepart = <&mmc 1>;
	};

<&mmc 1> means block storage device pointer and its partition.

Above example is a description for block storage, but for UBI storage
device, it can be described in FDT as shown in below:

Example for ubi::

	fw_loader1: fs-loader {
		bootph-all;
		compatible = "u-boot,fs-loader";
		mtdpart = "UBI",
		ubivol = "ubi0";
	};

(FIP loader follow the same exact implementation by just using the
dedicated compatible "u-boo,fip-loader")

Then, firmware-loader property can be added with any device node, which
driver would use the firmware loader for loading.

The value of the firmware-loader property should be set with phandle
of the fw-loader node. For example::

	firmware-loader = <&fw_loader0>;

And firmware loader device can be reached by the use get_fw_loader_from_node API:

For example:

.. code-block:: c

	get_fw_loader_from_node(dev_ofnode(dev), &fw_loader);

If there are majority of devices using the same fs-loader node, then
firmware-loader property can be added under /chosen node instead of
adding to each of device node.

For example::

	/{
		chosen {
			firmware-loader = <&fw_loader0>;
		};
	};

In each respective driver of devices using firmware loader, the firmware
loaded instance	should be created by DT phandle.

For example of getting DT phandle from /chosen and creating instance:

.. code-block:: c

	chosen_node = ofnode_path("/chosen");
	if (!ofnode_valid(chosen_node)) {
		debug("/chosen node was not found.\n");
		return -ENOENT;
	}

	phandle_p = ofnode_get_property(chosen_node, "firmware-loader", &size);
	if (!phandle_p) {
		debug("firmware-loader property was not found.\n");
		return -ENOENT;
	}

	phandle = fdt32_to_cpu(*phandle_p);
	ret = uclass_get_device_by_phandle_id(UCLASS_FIRMWARE_LOADER,
					     phandle, &dev);
	if (ret)
		return ret;

Firmware loader driver is also designed to support U-Boot environment
variables, so all these data from FDT can be overwritten
through the U-Boot environment variable during run time.

For examples:

storage_interface:
  Storage interface, it can be "mmc", "usb", "sata" or "ubi".
fw_dev_part:
  Block device number and its partition, it can be "0:1".
fw_ubi_mtdpart:
  UBI device mtd partition, it can be "UBI".
fw_ubi_volume:
  UBI volume, it can be "ubi0".

When above environment variables are set, environment values would be
used instead of data from FDT.
The benefit of this design allows user to change storage attribute data
at run time through U-Boot console and saving the setting as default
environment values in the storage for the next power cycle, so no
compilation is required for both driver and FDT.

File system firmware Loader API
-------------------------------

.. code-block:: c

	int get_fw_loader_from_node(ofnode node, struct udevice **dev);

Get firmware loader from referenced node

Parameters:

* ofnode \*node: node where the "firmware-loader" phandle needs to be parsed
* struct udevice \**dev:  pointer where to store the firmware loader device

Returns:
	0 on success
	-ve when error

Description:
	Parse the "firmware-loader" phandle from the passed node and
	probe and put the matching firmware loader device in the dev pointer.

.. code-block:: c

	int request_firmware_size(struct udevice *dev, const char *name)

Get firmware size from Firmware Loader driver

Parameters:

* struct udevice \*dev: An instance of a driver
* const char \*name: name of firmware file

Returns:
	size of firmware
	-ve when error

Description:
	Get the firmware size before reading it to validate how much buffer
	is needed for it

.. code-block:: c

	int request_firmware_into_buf(struct udevice *dev,
				      const char *name,
				      void *buf, size_t size, u32 offset)

Load firmware into a previously allocated buffer

Parameters:

* struct udevice \*dev: An instance of a driver
* const char \*name: name of firmware file
* void \*buf: address of buffer to load firmware into
* size_t size: size of buffer
* u32 offset: offset of a file for start reading into buffer

Returns:
	size of total read
	-ve when error

Description:
	The firmware is loaded directly into the buffer pointed to by buf

Example of calling request_firmware_into_buf API after creating firmware loader
instance:

.. code-block:: c

	ret = uclass_get_device_by_phandle_id(UCLASS_FIRMWARE_LOADER,
					     phandle, &dev);
	if (ret)
		return ret;

	request_firmware_into_buf(dev, filename, buffer_location, buffer_size,
				  offset_ofreading);

Example of request_firmware_into_buf API from a "firmware-loader" defined in
the device node and dynamic buffer allocation:

.. code-block:: c

	ret = get_fw_loader_from_node(dev_ofnode(dev), &fw_loader);
	if (ret)
		return ret;

	buffer_size = request_firmware_size(fw_loader, filename);
	if (buffer_size <= 0)
		return buffer_size < 0 ? buffer_size : -EINVAL;

	buffer_location = calloc(buffer_size, sizeof(*buffer_location));
	if (!buffer_location)
		return -ENOMEM;

	request_firmware_into_buf(fw_loader, filename, buffer_location, buffer_size,
				  offset_ofreading);
