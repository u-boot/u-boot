.. SPDX-License-Identifier: GPL-2.0+

FPGA design with HSS programming file
'''''''''''''''''''''''''''''''''''''

https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/reference-designs-fpga-and-development-kits/updating-linux-in-mpfs-kit.md

The HSS firmware runs from the PolarFire SoC eNVM on reset.

Creating the HSS payload - Microchip boot-flow
''''''''''''''''''''''''''''''''''''''''''''''

1. You will be creating a payload from `u-boot-dtb.bin`.
   Copy this file to the HSS/tools/hss-payload-generator/test directory.
2. Go to hss-payload-generator source directory.

.. code-block:: none

   cd hart-software-services/tools/hss-payload-generator

3. Edit test/uboot.yaml file for hart entry points and correct name of the binary file.

	hart-entry-points: {u54_1: '0x80200000', u54_2: '0x80200000', u54_3: '0x80200000', u54_4: '0x80200000'}

	payloads:
	test/u-boot-dtb.bin: {exec-addr: '0x80200000', owner-hart: u54_1, secondary-hart: u54_2, secondary-hart: u54_3, secondary-hart: u54_4, priv-mode: prv_s}

4. Generate payload

.. code-block:: none

   ./hss-payload-generator -c test/uboot.yaml payload.bin

Once the payload binary is generated, it should be copied to the eMMC.

Please refer to HSS documenation to build the HSS firmware for payload.
(Note: HSS git repo is at https://github.com/polarfire-soc/hart-software-services/blob/master/tools/hss-payload-generator/README.md)