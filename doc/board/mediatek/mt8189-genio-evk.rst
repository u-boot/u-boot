.. SPDX-License-Identifier: GPL-2.0-only
.. Copyright (C) 2026 BayLibre SAS

MediaTek Genio 520 and Genio 720 EVKs
=====================================

The Genio 520 and Genio 720 EVKs use the same MT8189-family boot flow:

================  ======  ====================================
Board             SoC     Defconfig
================  ======  ====================================
Genio 520 EVK     MT8371  ``mt8371_genio_520_evk_defconfig``
Genio 720 EVK     MT8391  ``mt8391_genio_720_evk_defconfig``
================  ======  ====================================

The configurations share their memory layout, drivers and firmware-image
layout. Each board has its own control devicetree.

U-Boot can be used on these boards in two ways:

* as U-Boot proper only, started as BL33 by the platform's own BL2 and Arm
  Trusted Firmware-A, or
* optionally, also as U-Boot SPL, which replaces the platform BL2 and loads
  BL31, OP-TEE and U-Boot proper itself.

U-Boot proper
-------------

In the usual boot chain, U-Boot provides only U-Boot proper::

   BootROM
     -> platform BL2
     -> Arm Trusted Firmware-A (BL31)
     -> OP-TEE (BL32)
     -> U-Boot proper (BL33)

The platform BL2 initializes DRAM and loads BL31, BL32 and BL33. U-Boot proper
is linked at ``CONFIG_TEXT_BASE``, ``0x4c000000``. It takes the DRAM size from
the ``memory`` node of its devicetree, which describes 8 GiB of DRAM.

Building
~~~~~~~~

Select the defconfig for the target board. For example, for Genio 720::

   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ export KBUILD_OUTPUT=build
   $ make mt8391_genio_720_evk_defconfig
   $ make

Use ``mt8371_genio_520_evk_defconfig`` instead for Genio 520.

The EVKs have DIP switches and jumpers which connect either the eMMC or the
UFS storage. The default devicetree is for eMMC. When the board is set up for
UFS, select the UFS variant of the devicetree instead::

   $ make DEVICE_TREE=mt8391-genio-720-evk-ufs

Use ``DEVICE_TREE=mt8371-genio-520-evk-ufs`` for Genio 520.

This produces ``build/u-boot.bin``, which is the BL33 image. The build also
produces ``build/spl/u-boot-spl.bin``, which is not used in this boot chain.
No external firmware is needed. Install ``u-boot.bin`` as BL33 as described
by the platform firmware.

U-Boot SPL
----------

U-Boot SPL can optionally replace the platform BL2::

   BootROM
     -> platform DDR loader
     -> U-Boot SPL
     -> Arm Trusted Firmware-A (BL31)
     -> OP-TEE (BL32)
     -> U-Boot proper (BL33)

The BootROM loads a MediaTek image containing a platform DDR loader followed by
U-Boot SPL. The DDR loader initializes DRAM, copies SPL to DRAM and enters it.
SPL then reads a FIT image containing BL31, OP-TEE, U-Boot proper and the
U-Boot control devicetree, and uses the standard FIT and Arm Trusted Firmware
support to hand off to BL31.

The DDR loader is a platform firmware component built separately from U-Boot
and supplied to binman as an external blob.

Boot storage
~~~~~~~~~~~~

The supplied configurations boot from eMMC. The BootROM loads ``mtk-boot.bin``
from the eMMC boot0 hardware partition, and SPL loads ``bootloaders.img`` from
GPT partition 1 of the eMMC user area. SPL selects the partition by number
through ``CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION``; it does not locate it
by name.

Make sure the DIP switches and jumpers are set for eMMC, and use the default
devicetree.

DDR-loader handoff contract
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The contract between the DDR loader and U-Boot SPL consists of a fixed image
layout, initialized DRAM and the execution state at the SPL entry point. The
BootROM loads the image payload to ``CONFIG_MTK_GENIO_BROM_LOAD_ADDR``::

   offset 0                                     +----------------------+
                                                | DDR loader           |
                                                |                      |
   offset CONFIG_MTK_GENIO_DDR_LOADER_SIZE      +----------------------+
                                                | U-Boot SPL           |
                                                | CONFIG_SPL_MAX_SIZE  |
   image end                                    +----------------------+

The supplied configurations use the following values:

=====================================  ==============  ========
Setting                                Value           Size
=====================================  ==============  ========
``CONFIG_MTK_GENIO_BROM_LOAD_ADDR``    ``0x2001000``
``CONFIG_MTK_GENIO_DDR_LOADER_SIZE``   ``0xa0000``     640 KiB
``CONFIG_SPL_MAX_SIZE``                ``0x40000``     256 KiB
``CONFIG_SPL_TEXT_BASE``               ``0x40000000``
``CONFIG_MTK_GENIO_BL31_LOAD_ADDR``    ``0x54601000``
=====================================  ==============  ========

The DDR loader must:

* initialize and train DRAM,
* copy the complete ``CONFIG_SPL_MAX_SIZE`` byte SPL region from
  ``CONFIG_MTK_GENIO_BROM_LOAD_ADDR + CONFIG_MTK_GENIO_DDR_LOADER_SIZE``
  (``0x20a1000``) to ``CONFIG_SPL_TEXT_BASE``,
* leave the MMU and caches disabled, with all of its writes visible to the
  CPU, and
* enter SPL at ``CONFIG_SPL_TEXT_BASE`` in AArch64 EL3h with exceptions
  masked.

SPL obtains platform information from its embedded control devicetree and
initializes the console, clocks, pinctrl, watchdog and eMMC through U-Boot
drivers.

The MT8189 BootROM loads at most about 930 KiB from eMMC boot0 and does not
start a larger image. The DDR loader and SPL regions together must fit in this
limit.

The DDR loader runs from SRAM at the load address, and the SPL region stays in
the same SRAM until the loader has copied it to DRAM. The loader's code, data,
BSS and stack must therefore all stay below the SPL region, not only its
binary image.

Firmware image prerequisites
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following external binaries are required to assemble the complete
firmware images:

``ddr-loader.bin``
   A MediaTek DDR loader which initializes the DRAM fitted to the target board
   and satisfies the handoff contract above. The input may be shorter than the
   configured loader region; binman pads it with zeros.

``BL31``
   The U-Boot build variable naming the Arm Trusted Firmware-A BL31 binary
   built for the MT8189 platform. It is loaded and entered at
   ``CONFIG_MTK_GENIO_BL31_LOAD_ADDR``.

``TEE``
   The U-Boot build variable naming the OP-TEE binary. A standard OP-TEE v1
   ``tee.bin`` image is supported. Binman removes its header and derives the
   load and entry addresses from it when creating the FIT.

BL31 and OP-TEE must agree on the secure-memory layout. Place
``ddr-loader.bin`` in a directory which can be passed to binman with
``BINMAN_INDIRS``.

Building
~~~~~~~~

Enable ``CONFIG_MTK_GENIO_BOOT_IMAGES`` to assemble the complete firmware
images. For example, for Genio 720 with the DDR loader in
``/path/to/firmware/ddr-loader.bin``::

   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ export KBUILD_OUTPUT=build
   $ export BL31=/path/to/bl31.bin
   $ export TEE=/path/to/tee.bin
   $ make mt8391_genio_720_evk_defconfig
   $ scripts/config --file "${KBUILD_OUTPUT}/.config" \
         --enable MTK_GENIO_BOOT_IMAGES
   $ make olddefconfig
   $ make BINMAN_INDIRS=/path/to/firmware

Use ``mt8371_genio_520_evk_defconfig`` instead for Genio 520. ``TEE`` must be
set for both configuration and compilation. Do not deploy images if binman
reports that it used fake or missing external blobs.

Binman produces two deployable images and one intermediate payload in the
build directory:

``mtk-boot.bin``
   A MediaTek eMMC image with load and entry address
   ``CONFIG_MTK_GENIO_BROM_LOAD_ADDR``. It contains the DDR loader, padded to
   ``CONFIG_MTK_GENIO_DDR_LOADER_SIZE``, followed by U-Boot SPL padded to
   ``CONFIG_SPL_MAX_SIZE``.

``mtk-boot-payload.bin``
   The raw DDR-loader and SPL payload contained in ``mtk-boot.bin``, without
   the MediaTek BootROM header. This is an intermediate input for external
   MediaTek secure-boot tooling and must not be flashed directly.

``bootloaders.img``
   A FIT image containing BL31, U-Boot proper, OP-TEE and the U-Boot control
   devicetree. Each component has a SHA-256 hash.

The FIT can also be signed, as described for the Genio 510 and Genio 700 EVKs
in :doc:`mt8188-genio-evk`.

Installing
~~~~~~~~~~

Use the board provisioning tools to place the images as follows:

===================  ================================================
Image                Destination
===================  ================================================
``mtk-boot.bin``     Start of the eMMC boot0 hardware partition
``bootloaders.img``  GPT partition 1 in the eMMC user area
===================  ================================================

For example, with `Genio Tools
<https://genio.mediatek.com/doc/iot-yocto/latest/tools/genio-tools.html>`_ and
a prebuilt Genio image for the target board::

   $ genio-flash -P /path/to/genio-image \
         mmc0boot0:/absolute/path/to/build/mtk-boot.bin \
         bootloaders:/absolute/path/to/build/bootloaders.img

Writing an invalid image to eMMC boot0 can make the board unbootable. Preserve
the platform recovery path while testing.
