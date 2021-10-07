.. SPDX-License-Identifier: GPL-2.0+

U-Boot for JetHub J100
=======================

JetHome Jethub D1 (http://jethome.ru/jethub-d1) is a home automation
controller manufactured by JetHome with the following specifications:

 - Amlogic A113X (ARM Cortex-A53) quad-core up to 1.5GHz
 - no video out
 - 512Mb/1GB DDR3
 - 8/16GB eMMC flash
 - 1 x USB 2.0
 - 1 x 10/100Mbps ethernet
 - WiFi / Bluetooth AMPAK AP6255 (Broadcom BCM43455) IEEE
   802.11a/b/g/n/ac, Bluetooth 4.2.
 - TI CC2538 + CC2592 Zigbee Wireless Module with up to 20dBm output
   power and Zigbee 3.0 support.
 - 2 x gpio LEDS
 - GPIO user Button
 - 1 x 1-Wire
 - 2 x RS-485
 - 4 x dry contact digital GPIO inputs
 - 3 x relay GPIO outputs
 - DC source with a voltage of 9 to 56 V / Passive POE
 - DIN Rail Mounting case

U-Boot compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make jethub_j100_defconfig
    $ make

Image creation
--------------

Amlogic doesn't provide sources for the firmware and for tools needed
to create the bootloader image, so it is necessary to obtain binaries
from the git tree published by the board vendor:

.. code-block:: bash

    $ git clone https://github.com/jethome-ru/jethub-aml-tools jethub-u-boot
    $ cd jethub-u-boot
    $ export FIPDIR=$PWD

Go back to mainline U-boot source tree then :

.. code-block:: bash

    $ mkdir fip

    $ cp $FIPDIR/j100/bl2.bin fip/
    $ cp $FIPDIR/j100/acs.bin fip/
    $ cp $FIPDIR/j100/bl21.bin fip/
    $ cp $FIPDIR/j100/bl30.bin fip/
    $ cp $FIPDIR/j100/bl301.bin fip/
    $ cp $FIPDIR/j100/bl31.img fip/
    $ cp u-boot.bin fip/bl33.bin

    $ $FIPDIR/blx_fix.sh \
        fip/bl30.bin \
        fip/zero_tmp \
        fip/bl30_zero.bin \
        fip/bl301.bin \
        fip/bl301_zero.bin \
        fip/bl30_new.bin \
        bl30

    $ $FIPDIR/acs_tool.pyc fip/bl2.bin fip/bl2_acs.bin fip/acs.bin 0

    $ $FIPDIR/blx_fix.sh \
        fip/bl2_acs.bin \
        fip/zero_tmp \
        fip/bl2_zero.bin \
        fip/bl21.bin \
        fip/bl21_zero.bin \
        fip/bl2_new.bin \
        bl2

    $ $FIPDIR/j100/aml_encrypt_axg --bl3sig --input fip/bl30_new.bin \
                                        --output fip/bl30_new.bin.enc \
                                        --level v3 --type bl30
    $ $FIPDIR/j100/aml_encrypt_axg --bl3sig --input fip/bl31.img \
                                        --output fip/bl31.img.enc \
                                        --level v3 --type bl31
    $ $FIPDIR/j100/aml_encrypt_axg --bl3sig --input fip/bl33.bin --compress lz4 \
                                        --output fip/bl33.bin.enc \
                                        --level v3 --type bl33
    $ $FIPDIR/j100/aml_encrypt_axg --bl2sig --input fip/bl2_new.bin \
                                        --output fip/bl2.n.bin.sig
    $ $FIPDIR/j100/aml_encrypt_axg --bootmk \
                --output fip/u-boot.bin \
                --bl2 fip/bl2.n.bin.sig \
                --bl30 fip/bl30_new.bin.enc \
                --bl31 fip/bl31.img.enc \
                --bl33 fip/bl33.bin.enc --level v3

and then write the image to eMMC with:

.. code-block:: bash

    $ DEV=/dev/your_emmc_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=444
