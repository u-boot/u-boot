.. SPDX-License-Identifier: GPL-2.0

Arm Versatile Express
=====================

The vexpress_* board configuration supports the following platforms:

 * FVP_Base_RevC-2xAEMvA
 * FVP_BaseR_AEMv8R
 * Juno development board

Fixed Virtual Platforms
-----------------------

The Fixed Virtual Platforms (FVP) are complete simulations of an Arm system,
including processor, memory and peripherals. They are set out in a "programmer's
view", which gives a comprehensive model on which to build and test software.

The supported FVPs are available free of charge and can be downloaded from the
Arm developer site [1]_ (user registration might be required).

The Architecture Envelope Models (AEM) FVPs offer virtual platforms for Armv8-A,
Armv9-A, and Armv8-R architectures, including a comprehensive set of System IP.
For general use though, the Armv8-A Base Rev C FVP, which emulates a generic 64-bit
Armv8-A hardware platform, is a suitable option.

Supported features:

 * GICv3
 * Generic timer
 * PL011 UART

The default configuration assumes that U-Boot is bootstrapped using a suitable
bootloader, such as Trusted Firmware-A [4]_. The u-boot binary can be passed
into the TF-A build: ``make PLAT=<platform> all fip BL33=u-boot.bin``

The FVPs can be debugged using Arm Development Studio [2]_.

Building U-Boot
^^^^^^^^^^^^^^^

Set the ``CROSS_COMPILE`` environment variable as usual, and run:

.. code-block:: bash

    make vexpress_fvp_defconfig
    make

Running U-Boot
^^^^^^^^^^^^^^

Set ``CROSS_COMPILE`` as usual and build TF-A:

.. code-block:: bash

    git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    cd trusted-firmware-a
    make PLAT=fvp BL33=/path/to/u-boot.bin fiptool all fip

This command generates the ROM image `bl1.bin`, and a boot image `fip.bin` in
TF-A's FIP format [5]_. It contains all images executed by TF-A, including U-Boot.
Note that TF-A outputs the built binaries into `build/fvp/release/`.

If you already have a FIP image, and are primarily interested in updating the BL33
image (i.e., U-Boot), use `fiptool` from TF-A:

.. code-block:: bash

    make fiptool
    tools/fiptool/fiptool update --nt-fw=/path/to/u-boot.bin /path/to/fip.bin

To run the FVP:

.. code-block:: bash

    FVP_Base_RevC-2xAEMvA -C bp.flashloader0.fname=fip.bin \
        -C bp.secureflashloader.fname=bl1.bin \
        -C bp.vis.disable_visualisation=1

This setup relies on semi-hosting, as well as, having a kernel image (``Image``)
and ramdisk (``ramdisk.img``) in the current working directory.

Juno
----

Juno is an Arm development board with the following features:

 * Arm Cortex-A72/A57 and Arm Cortex-A53 in a "big.LITTLE" configuration
 * A PCIe Gen2.0 bus with 4 lanes
 * 8GB of DRAM
 * GICv2

More details can be found in the board documentation [3]_.

Bloblist Support
----------------

The ``vexpress_fvp_bloblist_defconfig`` configures U-Boot to be compiled for
Vexpress64 with Bloblist as the primary method for information handoff between
boot stages. U-Boot offers three methods to set up a bloblist: using a
predefined bloblist at a specified address, dynamically allocating memory for a
bloblist, or utilizing a standard passage-provided bloblist with automatic size
detection.

By default, ``vexpress_fvp_bloblist_defconfig`` uses the standard passage method mandatorily
(CONFIG_BLOBLIST_PASSAGE_MANDATORY) because TF-A provides a Transfer List in non-secure
memory that U-Boot can utilise. This Bloblist, which is referred to as a Transfer List in
TF-A, contains all necessary data for the handoff process, including DT and ACPI
tables.

References
----------

.. [1] https://developer.arm.com/Tools%20and%20Software/Fixed%20Virtual%20Platforms/Arm%20Architecture%20FVPs
.. [2] https://developer.arm.com/tools-and-software/embedded/arm-development-studio
.. [3] https://developer.arm.com/tools-and-software/development-boards/juno-development-board
.. [4] https://trustedfirmware-a.readthedocs.io/
.. [5] https://trustedfirmware-a.readthedocs.io/en/latest/getting_started/image-terminology.html#firmware-image-package-fip
