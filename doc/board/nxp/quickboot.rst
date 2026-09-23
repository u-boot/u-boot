.. SPDX-License-Identifier: GPL-2.0+
   Copyright 2026 NXP

DDR QuickBoot flow
------------------

Some NXP SoCs (which use OEI - iMX943, iMX95, iMX952 etc.) support saving
DDR training data (collected by OEI during Training flow) from volatile
to non-volatile memory, which is then available to OEI at next cold reboot.
OEI uses the saved data to run Quickboot flow and avoid training the DDR again.
This significantly reduces the boot time.

The location of the quickboot data in NVM is a space left in the bootloader by
mkimage, with the size of 64K. The qb command searches for this space to
save the data. Thus, the NVM should also be a boot device and contain
the bootloader at the time of the saving.

U-Boot provides no authentication for quickboot data, only its integrity
is verified via the CRC32. The authentication is done in OEI. With
the exception of iMX95 A0/A1, which use CRC32 as well for verifying
the data, the rest of the SoCs use ELE to verify the MAC stored
in the ddrphy_qb_state structure.

If the quickboot data in memory is not valid (CRC32 check fails),
U-Boot does not save it to NVM. So, if OEI runs Quickboot flow -> no
data is written to volatile memory -> invalid data -> no saving happens
(qb save fails during qb check).

After successful saving, U-Boot clears the data in volatile memory so
that qb check fails at next reboot and the NVM isn't accessed again.

There are 2 ways to save this data, both can be enabled:

1. automatically, in SPL (by enabling CONFIG_SPL_IMX_QB)

- this will save the data on the current boot device (e.g. SD)
- other configs specific to the boot device need to be enabled (CONFIG_SPL_MMC_WRITE for saving to eMMC/SD)
- use for: automating qb save / saving quickboot data if using Falcon mode (skipping U-Boot proper)

2. using qb command in U-Boot console (by enabling CONFIG_CMD_IMX_QB)

- supports saving on the current boot device, or on another, specified device.
- supports specifying the hwpartition for eMMC (for booting from boot0/boot1)
- if flashing via uuu, the command can be added in an uuu script (boot device needs to be specified)
- use 'qb erase' to force DDR re-training
- use for: saving quickboot data during flashing / controlling the NVM to save to / forcing re-training

::

        # To save/erase on current boot device
        # For eMMC boot1, mmc 0:2 has to be specified explicitly
        => qb save/erase

        # To save/erase on other boot device
        => qb save/erase mmc 0   # eMMC boot0
        => qb save/erase mmc 0:1 # eMMC boot0
        => qb save/erase mmc 0:2 # eMMC boot1
        => qb save/erase mmc 1   # SD
        => qb save/erase spi     # NOR SPI
