.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2018 Heinrich Schuchardt

UEFI on U-Boot
==============

The Unified Extensible Firmware Interface Specification (UEFI) [1] has become
the default for booting on AArch64 and x86 systems. It provides a stable API for
the interaction of drivers and applications with the firmware. The API comprises
access to block storage, network, and console to name a few. The Linux kernel
and boot loaders like GRUB or the FreeBSD loader can be executed.

Development target
------------------

The implementation of UEFI in U-Boot strives to reach the requirements described
in the "Embedded Base Boot Requirements (EBBR) Specification - Release v1.0"
[2]. The "Server Base Boot Requirements System Software on ARM Platforms" [3]
describes a superset of the EBBR specification and may be used as further
reference.

A full blown UEFI implementation would contradict the U-Boot design principle
"keep it small".

Building U-Boot for UEFI
------------------------

The UEFI standard supports only little-endian systems. The UEFI support can be
activated for ARM and x86 by specifying::

    CONFIG_CMD_BOOTEFI=y
    CONFIG_EFI_LOADER=y

in the .config file.

Support for attaching virtual block devices, e.g. iSCSI drives connected by the
loaded UEFI application [4], requires::

    CONFIG_BLK=y
    CONFIG_PARTITIONS=y

Executing a UEFI binary
~~~~~~~~~~~~~~~~~~~~~~~

The bootefi command is used to start UEFI applications or to install UEFI
drivers. It takes two parameters::

    bootefi <image address> [fdt address]

* image address - the memory address of the UEFI binary
* fdt address - the memory address of the flattened device tree

Below you find the output of an example session starting GRUB::

    => load mmc 0:2 ${fdt_addr_r} boot/dtb
    29830 bytes read in 14 ms (2 MiB/s)
    => load mmc 0:1 ${kernel_addr_r} efi/debian/grubaa64.efi
    reading efi/debian/grubaa64.efi
    120832 bytes read in 7 ms (16.5 MiB/s)
    => bootefi ${kernel_addr_r} ${fdt_addr_r}

The environment variable 'bootargs' is passed as load options in the UEFI system
table. The Linux kernel EFI stub uses the load options as command line
arguments.

Launching a UEFI binary from a FIT image
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A signed FIT image can be used to securely boot a UEFI image via the
bootm command. This feature is available if U-Boot is configured with::

    CONFIG_BOOTM_EFI=y

A sample configuration is provided as file doc/uImage.FIT/uefi.its.

Below you find the output of an example session starting GRUB::

    => load mmc 0:1 ${kernel_addr_r} image.fit
    4620426 bytes read in 83 ms (53.1 MiB/s)
    => bootm ${kernel_addr_r}#config-grub-nofdt
    ## Loading kernel from FIT Image at 40400000 ...
       Using 'config-grub-nofdt' configuration
       Verifying Hash Integrity ... sha256,rsa2048:dev+ OK
       Trying 'efi-grub' kernel subimage
         Description:  GRUB EFI Firmware
         Created:      2019-11-20   8:18:16 UTC
         Type:         Kernel Image (no loading done)
         Compression:  uncompressed
         Data Start:   0x404000d0
         Data Size:    450560 Bytes = 440 KiB
         Hash algo:    sha256
         Hash value:   4dbee00021112df618f58b3f7cf5e1595533d543094064b9ce991e8b054a9eec
       Verifying Hash Integrity ... sha256+ OK
       XIP Kernel Image (no loading done)
    ## Transferring control to EFI (at address 404000d0) ...
    Welcome to GRUB!

See doc/uImage.FIT/howto.txt for an introduction to FIT images.

Configuring UEFI secure boot
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The UEFI specification[1] defines a secure way of executing UEFI images
by verifying a signature (or message digest) of image with certificates.
This feature on U-Boot is enabled with::

    CONFIG_UEFI_SECURE_BOOT=y

To make the boot sequence safe, you need to establish a chain of trust;
In UEFI secure boot the chain trust is defined by the following UEFI variables

* PK - Platform Key
* KEK - Key Exchange Keys
* db - white list database
* dbx - black list database

An in depth description of UEFI secure boot is beyond the scope of this
document. Please, refer to the UEFI specification and available online
documentation. Here is a simple example that you can follow for your initial
attempt (Please note that the actual steps will depend on your system and
environment.):

Install the required tools on your host

* openssl
* efitools
* sbsigntool

Create signing keys and the key database on your host:

The platform key

.. code-block:: bash

    openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_PK/ \
            -keyout PK.key -out PK.crt -nodes -days 365
    cert-to-efi-sig-list -g 11111111-2222-3333-4444-123456789abc \
            PK.crt PK.esl;
    sign-efi-sig-list -c PK.crt -k PK.key PK PK.esl PK.auth

The key exchange keys

.. code-block:: bash

    openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_KEK/ \
            -keyout KEK.key -out KEK.crt -nodes -days 365
    cert-to-efi-sig-list -g 11111111-2222-3333-4444-123456789abc \
            KEK.crt KEK.esl
    sign-efi-sig-list -c PK.crt -k PK.key KEK KEK.esl KEK.auth

The whitelist database

.. code-block:: bash

    $ openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_db/ \
            -keyout db.key -out db.crt -nodes -days 365
    $ cert-to-efi-sig-list -g 11111111-2222-3333-4444-123456789abc \
            db.crt db.esl
    $ sign-efi-sig-list -c KEK.crt -k KEK.key db db.esl db.auth

Copy the \*.auth files to media, say mmc, that is accessible from U-Boot.

Sign an image with one of the keys in "db" on your host

.. code-block:: bash

    sbsign --key db.key --cert db.crt helloworld.efi

Now in U-Boot install the keys on your board::

    fatload mmc 0:1 <tmpaddr> PK.auth
    setenv -e -nv -bs -rt -at -i <tmpaddr>,$filesize PK
    fatload mmc 0:1 <tmpaddr> KEK.auth
    setenv -e -nv -bs -rt -at -i <tmpaddr>,$filesize KEK
    fatload mmc 0:1 <tmpaddr> db.auth
    setenv -e -nv -bs -rt -at -i <tmpaddr>,$filesize db

Set up boot parameters on your board::

    efidebug boot add 1 HELLO mmc 0:1 /helloworld.efi.signed ""

Now your board can run the signed image via the boot manager (see below).
You can also try this sequence by running Pytest, test_efi_secboot,
on the sandbox

.. code-block:: bash

    cd <U-Boot source directory>
    pytest.py test/py/tests/test_efi_secboot/test_signed.py --bd sandbox

Using OP-TEE for EFI variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Instead of implementing UEFI variable services inside U-Boot they can
also be provided in the secure world by a module for OP-TEE[1]. The
interface between U-Boot and OP-TEE for variable services is enabled by
CONFIG_EFI_MM_COMM_TEE=y.

Tianocore EDK II's standalone management mode driver for variables can
be linked to OP-TEE for this purpose. This module uses the Replay
Protected Memory Block (RPMB) of an eMMC device for persisting
non-volatile variables. When calling the variable services via the
OP-TEE API U-Boot's OP-TEE supplicant relays calls to the RPMB driver
which has to be enabled via CONFIG_SUPPORT_EMMC_RPMB=y.

[1] https://optee.readthedocs.io/ - OP-TEE documentation

Executing the boot manager
~~~~~~~~~~~~~~~~~~~~~~~~~~

The UEFI specification foresees to define boot entries and boot sequence via UEFI
variables. Booting according to these variables is possible via::

    bootefi bootmgr [fdt address]

As of U-Boot v2018.03 UEFI variables are not persisted and cannot be set at
runtime.

Executing the built in hello world application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A hello world UEFI application can be built with::

    CONFIG_CMD_BOOTEFI_HELLO_COMPILE=y

It can be embedded into the U-Boot binary with::

    CONFIG_CMD_BOOTEFI_HELLO=y

The bootefi command is used to start the embedded hello world application::

    bootefi hello [fdt address]

Below you find the output of an example session::

    => bootefi hello ${fdtcontroladdr}
    ## Starting EFI application at 01000000 ...
    WARNING: using memory device/image path, this may confuse some payloads!
    Hello, world!
    Running on UEFI 2.7
    Have SMBIOS table
    Have device tree
    Load options: root=/dev/sdb3 init=/sbin/init rootwait ro
    ## Application terminated, r = 0

The environment variable fdtcontroladdr points to U-Boot's internal device tree
(if available).

Executing the built-in self-test
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An UEFI self-test suite can be embedded in U-Boot by building with::

    CONFIG_CMD_BOOTEFI_SELFTEST=y

For testing the UEFI implementation the bootefi command can be used to start the
self-test::

    bootefi selftest [fdt address]

The environment variable 'efi_selftest' can be used to select a single test. If
it is not provided all tests are executed except those marked as 'on request'.
If the environment variable is set to 'list' a list of all tests is shown.

Below you can find the output of an example session::

    => setenv efi_selftest simple network protocol
    => bootefi selftest
    Testing EFI API implementation
    Selected test: 'simple network protocol'
    Setting up 'simple network protocol'
    Setting up 'simple network protocol' succeeded
    Executing 'simple network protocol'
    DHCP Discover
    DHCP reply received from 192.168.76.2 (52:55:c0:a8:4c:02)
      as broadcast message.
    Executing 'simple network protocol' succeeded
    Tearing down 'simple network protocol'
    Tearing down 'simple network protocol' succeeded
    Boot services terminated
    Summary: 0 failures
    Preparing for reset. Press any key.

The UEFI life cycle
-------------------

After the U-Boot platform has been initialized the UEFI API provides two kinds
of services:

* boot services
* runtime services

The API can be extended by loading UEFI drivers which come in two variants:

* boot drivers
* runtime drivers

UEFI drivers are installed with U-Boot's bootefi command. With the same command
UEFI applications can be executed.

Loaded images of UEFI drivers stay in memory after returning to U-Boot while
loaded images of applications are removed from memory.

An UEFI application (e.g. an operating system) that wants to take full control
of the system calls ExitBootServices. After a UEFI application calls
ExitBootServices

* boot services are not available anymore
* timer events are stopped
* the memory used by U-Boot except for runtime services is released
* the memory used by boot time drivers is released

So this is a point of no return. Afterwards the UEFI application can only return
to U-Boot by rebooting.

The UEFI object model
---------------------

UEFI offers a flexible and expandable object model. The objects in the UEFI API
are devices, drivers, and loaded images. These objects are referenced by
handles.

The interfaces implemented by the objects are referred to as protocols. These
are identified by GUIDs. They can be installed and uninstalled by calling the
appropriate boot services.

Handles are created by the InstallProtocolInterface or the
InstallMultipleProtocolinterfaces service if NULL is passed as handle.

Handles are deleted when the last protocol has been removed with the
UninstallProtocolInterface or the UninstallMultipleProtocolInterfaces service.

Devices offer the EFI_DEVICE_PATH_PROTOCOL. A device path is the concatenation
of device nodes. By their device paths all devices of a system are arranged in a
tree.

Drivers offer the EFI_DRIVER_BINDING_PROTOCOL. This protocol is used to connect
a driver to devices (which are referenced as controllers in this context).

Loaded images offer the EFI_LOADED_IMAGE_PROTOCOL. This protocol provides meta
information about the image and a pointer to the unload callback function.

The UEFI events
---------------

In the UEFI terminology an event is a data object referencing a notification
function which is queued for calling when the event is signaled. The following
types of events exist:

* periodic and single shot timer events
* exit boot services events, triggered by calling the ExitBootServices() service
* virtual address change events
* memory map change events
* read to boot events
* reset system events
* system table events
* events that are only triggered programmatically

Events can be created with the CreateEvent service and deleted with CloseEvent
service.

Events can be assigned to an event group. If any of the events in a group is
signaled, all other events in the group are also set to the signaled state.

The UEFI driver model
---------------------

A driver is specific for a single protocol installed on a device. To install a
driver on a device the ConnectController service is called. In this context
controller refers to the device for which the driver is installed.

The relevant drivers are identified using the EFI_DRIVER_BINDING_PROTOCOL. This
protocol has has three functions:

* supported - determines if the driver is compatible with the device
* start - installs the driver by opening the relevant protocol with
  attribute EFI_OPEN_PROTOCOL_BY_DRIVER
* stop - uninstalls the driver

The driver may create child controllers (child devices). E.g. a driver for block
IO devices will create the device handles for the partitions. The child
controllers  will open the supported protocol with the attribute
EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER.

A driver can be detached from a device using the DisconnectController service.

U-Boot devices mapped as UEFI devices
-------------------------------------

Some of the U-Boot devices are mapped as UEFI devices

* block IO devices
* console
* graphical output
* network adapter

As of U-Boot 2018.03 the logic for doing this is hard coded.

The development target is to integrate the setup of these UEFI devices with the
U-Boot driver model [5]. So when a U-Boot device is discovered a handle should
be created and the device path protocol and the relevant IO protocol should be
installed. The UEFI driver then would be attached by calling ConnectController.
When a U-Boot device is removed DisconnectController should be called.

UEFI devices mapped as U-Boot devices
-------------------------------------

UEFI drivers binaries and applications may create new (virtual) devices, install
a protocol and call the ConnectController service. Now the matching UEFI driver
is determined by iterating over the implementations of the
EFI_DRIVER_BINDING_PROTOCOL.

It is the task of the UEFI driver to create a corresponding U-Boot device and to
proxy calls for this U-Boot device to the controller.

In U-Boot 2018.03 this has only been implemented for block IO devices.

UEFI uclass
~~~~~~~~~~~

An UEFI uclass driver (lib/efi_driver/efi_uclass.c) has been created that
takes care of initializing the UEFI drivers and providing the
EFI_DRIVER_BINDING_PROTOCOL implementation for the UEFI drivers.

A linker created list is used to keep track of the UEFI drivers. To create an
entry in the list the UEFI driver uses the U_BOOT_DRIVER macro specifying
UCLASS_EFI as the ID of its uclass, e.g::

    /* Identify as UEFI driver */
    U_BOOT_DRIVER(efi_block) = {
        .name  = "EFI block driver",
        .id    = UCLASS_EFI,
        .ops   = &driver_ops,
    };

The available operations are defined via the structure struct efi_driver_ops::

    struct efi_driver_ops {
        const efi_guid_t *protocol;
        const efi_guid_t *child_protocol;
        int (*bind)(efi_handle_t handle, void *interface);
    };

When the supported() function of the EFI_DRIVER_BINDING_PROTOCOL is called the
uclass checks if the protocol GUID matches the protocol GUID of the UEFI driver.
In the start() function the bind() function of the UEFI driver is called after
checking the GUID.
The stop() function of the EFI_DRIVER_BINDING_PROTOCOL disconnects the child
controllers created by the UEFI driver and the UEFI driver. (In U-Boot v2013.03
this is not yet completely implemented.)

UEFI block IO driver
~~~~~~~~~~~~~~~~~~~~

The UEFI block IO driver supports devices exposing the EFI_BLOCK_IO_PROTOCOL.

When connected it creates a new U-Boot block IO device with interface type
IF_TYPE_EFI, adds child controllers mapping the partitions, and installs the
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL on these. This can be used together with the
software iPXE to boot from iSCSI network drives [4].

This driver is only available if U-Boot is configured with::

    CONFIG_BLK=y
    CONFIG_PARTITIONS=y

Miscellaneous
-------------

Load file 2 protocol
~~~~~~~~~~~~~~~~~~~~

The load file 2 protocol can be used by the Linux kernel to load the initial
RAM disk. U-Boot can be configured to provide an implementation with::

    EFI_LOAD_FILE2_INITRD=y
    EFI_INITRD_FILESPEC=interface dev:part path_to_initrd

Links
-----

* [1] http://uefi.org/specifications - UEFI specifications
* [2] https://github.com/ARM-software/ebbr/releases/download/v1.0/ebbr-v1.0.pdf -
  Embedded Base Boot Requirements (EBBR) Specification - Release v1.0
* [3] https://developer.arm.com/docs/den0044/latest/server-base-boot-requirements-system-software-on-arm-platforms-version-11 -
  Server Base Boot Requirements System Software on ARM Platforms - Version 1.1
* [4] :doc:`iscsi`
* [5] :doc:`../driver-model/index`
