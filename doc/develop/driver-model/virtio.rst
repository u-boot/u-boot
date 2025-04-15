.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

VirtIO Support
==============

This document describes the information about U-Boot support for VirtIO_
devices, including supported boards, build instructions, driver details etc.

What's VirtIO?
--------------
VirtIO is a virtualization standard for network and disk device drivers where
just the guest's device driver "knows" it is running in a virtual environment,
and cooperates with the hypervisor. This enables guests to get high performance
network and disk operations, and gives most of the performance benefits of
paravirtualization. In the U-Boot case, the guest is U-Boot itself, while the
virtual environment are normally QEMU_ targets like ARM, RISC-V and x86.

Status
------
VirtIO can use various different buses, aka transports as described in the
spec. While VirtIO devices are commonly implemented as PCI devices on x86,
embedded devices models like ARM/RISC-V, which does not normally come with
PCI support might use simple memory mapped device (MMIO) instead of the PCI
device. The memory mapped virtio device behaviour is based on the PCI device
specification. Therefore most operations including device initialization,
queues configuration and buffer transfers are nearly identical. Both MMIO
and PCI transport options are supported in U-Boot.

The VirtIO spec defines a lots of VirtIO device types, however at present only
network and block device, the most two commonly used devices, are supported.

The following QEMU targets are supported.

  - qemu_arm_defconfig
  - qemu_arm64_defconfig
  - qemu-arm-sbsa_defconfig
  - qemu-riscv32_defconfig
  - qemu-riscv64_defconfig
  - qemu-x86_defconfig
  - qemu-x86_64_defconfig

Note ARM and RISC-V targets are configured with VirtIO MMIO transport driver,
and on x86 it's the PCI transport driver.

Build Instructions
------------------
Building U-Boot for pre-configured QEMU targets is no different from others.
For example, we can do the following with the CROSS_COMPILE environment
variable being properly set to a working toolchain for ARM:

.. code-block:: bash

  $ make qemu_arm_defconfig
  $ make

You can even create a QEMU ARM target with VirtIO devices showing up on both
MMIO and PCI buses. In this case, you can enable the PCI transport driver
from 'make menuconfig':

.. code-block:: none

  Device Drivers  --->
  	...
  	VirtIO Drivers  --->
  		...
  		[*] PCI driver for virtio devices

Other drivers are at the same location and can be tuned to suit the needs.

Requirements
------------
It is required that QEMU v2.5.0+ should be used to test U-Boot VirtIO support
on QEMU ARM and x86, and v2.12.0+ on QEMU RISC-V.

Testing
-------
The following QEMU command line is used to get U-Boot up and running with
VirtIO net and block devices on ARM.

.. code-block:: bash

  $ qemu-system-arm -nographic -machine virt -bios u-boot.bin \
    -netdev tap,ifname=tap0,id=net0 \
    -device virtio-net-device,netdev=net0 \
    -drive if=none,file=test.img,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0

On x86, command is slightly different to create PCI VirtIO devices.

.. code-block:: bash

  $ qemu-system-i386 -nographic -bios u-boot.rom \
    -netdev tap,ifname=tap0,id=net0 \
    -device virtio-net-pci,netdev=net0 \
    -drive if=none,file=test.img,format=raw,id=hd0 \
    -device virtio-blk-pci,drive=hd0

Additional net and block devices can be created by appending more '-device'
parameters. It is also possible to specify both MMIO and PCI VirtIO devices.
For example, the following commnad creates 3 VirtIO devices, with 1 on MMIO
and 2 on PCI bus.

.. code-block:: bash

  $ qemu-system-arm -nographic -machine virt -bios u-boot.bin \
    -netdev tap,ifname=tap0,id=net0 \
    -device virtio-net-pci,netdev=net0 \
    -drive if=none,file=test0.img,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0 \
    -drive if=none,file=test1.img,format=raw,id=hd1 \
    -device virtio-blk-pci,drive=hd1

By default QEMU creates VirtIO legacy devices by default. To create non-legacy
(aka modern) devices, pass additional device property/value pairs like below:

.. code-block:: bash

  $ qemu-system-i386 -nographic -bios u-boot.rom \
    -netdev tap,ifname=tap0,id=net0 \
    -device virtio-net-pci,netdev=net0,disable-legacy=true,disable-modern=false \
    -drive if=none,file=test.img,format=raw,id=hd0 \
    -device virtio-blk-pci,drive=hd0,disable-legacy=true,disable-modern=false

A 'virtio' command is provided in U-Boot shell.

.. code-block:: none

  => virtio
  virtio - virtio block devices sub-system

  Usage:
  virtio scan - initialize virtio bus
  virtio info - show all available virtio block devices
  virtio device [dev] - show or set current virtio block device
  virtio part [dev] - print partition table of one or all virtio block devices
  virtio read addr blk# cnt - read `cnt' blocks starting at block
       `blk#' to memory address `addr'
  virtio write addr blk# cnt - write `cnt' blocks starting at block
       `blk#' from memory address `addr'

To probe all the VirtIO devices, type:

.. code-block:: none

  => virtio scan

Then we can show the connected block device details by:

.. code-block:: none

  => virtio info
  Device 0: QEMU VirtIO Block Device
              Type: Hard Disk
              Capacity: 4096.0 MB = 4.0 GB (8388608 x 512)

And list the directories and files on the disk by:

.. code-block:: none

  => ls virtio 0 /
  <DIR>       4096 .
  <DIR>       4096 ..
  <DIR>      16384 lost+found
  <DIR>       4096 dev
  <DIR>       4096 proc
  <DIR>       4096 sys
  <DIR>       4096 var
  <DIR>       4096 etc
  <DIR>       4096 usr
  <SYM>          7 bin
  <SYM>          8 sbin
  <SYM>          7 lib
  <SYM>          9 lib64
  <DIR>       4096 run
  <DIR>       4096 boot
  <DIR>       4096 home
  <DIR>       4096 media
  <DIR>       4096 mnt
  <DIR>       4096 opt
  <DIR>       4096 root
  <DIR>       4096 srv
  <DIR>       4096 tmp
                 0 .autorelabel

Driver Internals
----------------
There are 3 level of drivers in the VirtIO driver family.

.. code-block:: none

	+---------------------------------------+
	|	 virtio device drivers		|
	|    +-------------+ +------------+	|
	|    | virtio-net  | | virtio-blk |	|
	|    +-------------+ +------------+	|
	+---------------------------------------+
	+---------------------------------------+
	|	virtio transport drivers	|
	|    +-------------+ +------------+	|
	|    | virtio-mmio | | virtio-pci |	|
	|    +-------------+ +------------+	|
	+---------------------------------------+
		+----------------------+
		| virtio uclass driver |
		+----------------------+

The root one is the virtio uclass driver (virtio-uclass.c), which does lots of
common stuff for the transport drivers (virtio_mmio.c, virtio_pci.c). The real
virtio device is discovered in the transport driver's probe() method, and its
device ID is saved in the virtio uclass's private data of the transport device.
Then in the virtio uclass's post_probe() method, the real virtio device driver
(virtio_net.c, virtio_blk.c) is bound if there is a match on the device ID.

The child_post_bind(), child_pre_probe() and child_post_probe() methods of the
virtio uclass driver help bring the virtio device driver online. They do things
like acknowledging device, feature negotiation, etc, which are really common
for all virtio devices.

The transport drivers provide a set of ops (struct dm_virtio_ops) for the real
virtio device driver to call. These ops APIs's parameter is designed to remind
the caller to pass the correct 'struct udevice' id of the virtio device, eg:

.. code-block:: C

  int virtio_get_status(struct udevice *vdev, u8 *status)

So the parameter 'vdev' indicates the device should be the real virtio device.
But we also have an API like:

.. code-block:: C

  struct virtqueue *vring_create_virtqueue(unsigned int index, unsigned int num,
  					 unsigned int vring_align,
  					 struct udevice *udev)

Here the parameter 'udev' indicates the device should be the transport device.
Similar naming is applied in other functions that are even not APIs, eg:

.. code-block:: C

  static int virtio_uclass_post_probe(struct udevice *udev)
  static int virtio_uclass_child_pre_probe(struct udevice *vdev)

So it's easy to tell which device these functions are operating on.

Development Flow
----------------
At present only VirtIO network card (device ID 1) and block device (device
ID 2) are supported. If you want to develop new driver for new devices,
please follow the guideline below.

1. add new device ID in virtio.h

.. code-block:: C

  #define VIRTIO_ID_XXX		X

2. update VIRTIO_ID_MAX_NUM to be the largest device ID plus 1

3. add new driver name string in virtio.h

.. code-block:: C

  #define VIRTIO_XXX_DRV_NAME	"virtio-xxx"

4. create a new driver with name set to the name string above

.. code-block:: C

  U_BOOT_DRIVER(virtio_xxx) = {
  	.name = VIRTIO_XXX_DRV_NAME,
  	...
  	.remove = virtio_reset,
  	.flags = DM_FLAG_ACTIVE_DMA,
  }

Note the driver needs to provide the remove method and normally this can be
hooked to virtio_reset(). The driver flags should contain DM_FLAG_ACTIVE_DMA
for the remove method to be called before jumping to OS.

5. provide bind() method in the driver, where virtio_driver_features_init()
   should be called for driver to negotiate feature support with the device.

6. do funny stuff with the driver

.. _VirtIO: http://docs.oasis-open.org/virtio/virtio/v1.0/virtio-v1.0.pdf
.. _QEMU: https://www.qemu.org
