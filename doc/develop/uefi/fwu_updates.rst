.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2022 Linaro Limited

FWU Multi Bank Updates in U-Boot
================================

The FWU Multi Bank Update feature implements the firmware update
mechanism described in the PSA Firmware Update for A-profile Arm
Architecture specification[1]. Certain aspects of the Dependable
Boot specification[2] are also implemented. The feature provides a
mechanism to have multiple banks of updatable firmware images and for
updating the firmware images on the non-booted bank. On a successful
update, the platform boots from the updated bank on subsequent
boot. The UEFI capsule-on-disk update feature is used for performing
the actual updates of the updatable firmware images.

The bookkeeping of the updatable images is done through a structure
called metadata. Currently, the FWU metadata supports identification
of images based on image GUIDs stored on a GPT partitioned storage
media. There are plans to extend the metadata structure for non GPT
partitioned devices as well.

Accessing the FWU metadata is done through generic API's which are
defined in a driver which complies with the u-boot's driver model. A
new uclass UCLASS_FWU_MDATA has been added for accessing the FWU
metadata. Individual drivers can be added based on the type of storage
media, and it's partitioning method. Details of the storage device
containing the FWU metadata partitions are specified through a U-Boot
specific device tree property `fwu-mdata-store`. Please refer to
U-Boot `doc <doc/device-tree-bindings/firmware/fwu-mdata.txt>`__ for
the device tree bindings.

Enabling the FWU Multi Bank Update feature
------------------------------------------

The feature can be enabled by specifying the following configs::

    CONFIG_EFI_CAPSULE_ON_DISK=y
    CONFIG_EFI_CAPSULE_FIRMWARE_MANAGEMENT=y
    CONFIG_EFI_CAPSULE_FIRMWARE=y
    CONFIG_EFI_CAPSULE_FIRMWARE_RAW=y

    CONFIG_FWU_MULTI_BANK_UPDATE=y
    CONFIG_CMD_FWU_METADATA=y
    CONFIG_DM_FWU_MDATA=y
    CONFIG_FWU_MDATA_GPT_BLK=y
    CONFIG_FWU_NUM_BANKS=<val>
    CONFIG_FWU_NUM_IMAGES_PER_BANK=<val>

in the .config file

The first group of configs enable the UEFI capsule-on-disk update
functionality. The second group of configs enable the FWU Multi Bank
Update functionality. Please refer to the section
:ref:`uefi_capsule_update_ref` for more details on generation of the
UEFI capsule.

Setting up the device for GPT partitioned storage
-------------------------------------------------

Before enabling the functionality in U-Boot, certain changes are
required to be done on the storage device. Assuming a GPT partitioned
storage device, the storage media needs to be partitioned with the
correct number of partitions, given the number of banks and number of
images per bank that the platform is going to support. Each updatable
firmware image will be stored on an separate partition. In addition,
the two copies of the FWU metadata will be stored on two separate
partitions.

As an example, a platform supporting two banks with each bank
containing three images would need to have 2 * 3 = 6 parititions plus
the two metadata partitions, or 8 partitions. In addition the storage
media can have additional partitions of non-updatable images, like the
EFI System Partition(ESP), a partition for the root file system etc.

When generating the partitions, a few aspects need to be taken care
of. Each GPT partition entry in the GPT header has two GUIDs::

    *PartitionTypeGUID*
    *UniquePartitionGUID*

The PartitionTypeGUID value should correspond to the *image_type_uuid*
field of the FWU metadata. This field is used to identify a given type
of updatable firmware image, e.g. u-boot, op-tee, FIP etc. This GUID
should also be used for specifying the `--guid` parameter when
generating the capsule.

The UniquePartitionGUID value should correspond to the *image_uuid*
field in the FWU metadata. This GUID is used to identify images of a
given image type in different banks.

Similarly, the FWU specifications defines the GUID value to be used
for the metadata partitions. This would be the PartitionTypeGUID for
the metadata partitions.

When generating the metadata, the *image_type_uuid* and the
*image_uuid* values should match the *PartitionTypeGUID* and the
*UniquePartitionGUID* values respectively.

Performing the Update
---------------------

Once the storage media has been partitioned and populated with the
metadata partitions, the UEFI capsule-on-disk update functionality can
be used for performing the update. Refer to the section
:ref:`uefi_capsule_update_ref` for details on how the update can be
invoked.

On a successful update, the FWU metadata gets updated to reflect the
bank from which the platform would be booting on subsequent boot.

Based on the value of bit15 of the Flags member of the capsule header,
the updated images would either be accepted by the u-boot's UEFI
implementation, or by the Operating System. If the Operating System is
accepting the firmware images, it does so by generating an empty
*accept* capsule. The Operating System can also reject the updated
firmware by generating a *revert* capsule. The empty capsule can be
applied by using the exact same procedure used for performing the
capsule-on-disk update.

Generating an empty capsule
---------------------------

The empty capsule can be generated using the mkeficapsule utility. To
build the tool, enable::

    CONFIG_TOOLS_MKEFICAPSULE=y

Run the following commands to generate the accept/revert capsules::

.. code-block:: bash

    $ ./tools/mkeficapsule \
      [--fw-accept --guid <image guid>] | \
      [--fw-revert] \
      <capsule_file_name>

Links
-----

* [1] https://developer.arm.com/documentation/den0118/a/ - FWU Specification
* [2] https://git.codelinaro.org/linaro/dependable-boot/mbfw/uploads/6f7ddfe3be24e18d4319e108a758d02e/mbfw.pdf - Dependable Boot Specification
