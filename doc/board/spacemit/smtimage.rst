.. SPDX-License-Identifier: GPL-2.0-or-later

SpacemiT boot image format
==========================

The BootROM on a SpacemiT K1 or K3 SoC expects the first-stage
bootloader to be wrapped in a specially constructed image layout.

Image layout
------------

A fixed 4 KiB prefix contains two 32-byte metadata headers, key slots, and
signature slots. The aligned SPL payload and a trailing authentication area
follow::

    offset                                    size
    0x000  +-------------------------------+
           | root RSA-2048 modulus         |  0x100
    0x100  +-------------------------------+
           | header0                       |  0x020
    0x120  +-------------------------------+
           | key-selection metadata        |  0x1e0
    0x300  +-------------------------------+
           | OEM public-key slots          |  0x800
    0xb00  +-------------------------------+
           | signature0                    |  0x100
    0xc00  +-------------------------------+
           | reserved                      |  0x3e0
    0xfe0  +-------------------------------+
           | header1                       |  0x020
    0x1000 +-------------------------------+
           | SPL payload (32-byte aligned) |
           +-------------------------------+
           | signature1                    |  0x100
           +-------------------------------+

For payload size ``P`` and ``A = ALIGN(P, 32)``, the image size is
``0x1000 + A + 0x100``.

The payload is aligned to 32 bytes because the BootROM RSA authentication path
rounds its length up to a multiple of 32 bytes when locating and verifying
signature1.

Metadata header
---------------

Both header0 and header1 use the following format:

.. list-table::
   :header-rows: 1

   * - Offset
     - Size
     - Field
     - Description
   * - 0x00
     - 4
     - ``magic``
     - ``AIHD``
   * - 0x04
     - 1
     - ``version``
     - Anti-rollback image version
   * - 0x05
     - 1
     - ``secure``
     - Zero selects anti-rollback bank 0

       Non-zero selects anti-rollback bank 1
   * - 0x06
     - 2
     - ``reserved``
     - Reserved
   * - 0x08
     - 8
     - ``image_size``
     - header0: prefix information

       header1: aligned payload size
   * - 0x10
     - 8
     - ``load_addr``
     - Unused by the common authentication code
   * - 0x18
     - 4
     - ``header_crc``
     - K3: CRC32 over header bytes ``[0x00, 0x18)``
   * - 0x1c
     - 4
     - ``image_crc``
     - K3: CRC32 over the payload in header1 only

``header1.image_size`` records the aligned payload size and therefore
determines the location of signature1.

Authentication
--------------

K1 and K3 apply different authentication policies:

.. list-table:: Authentication policy
   :header-rows: 1

   * - Image variant
     - Root and OEM keys
     - Header CRC
     - Image CRC
     - signature0
     - signature1
   * - K1
     - RSA-2048 moduli with an eFuse root-hash check when
       secure boot is enabled
     - Not checked separately
     - Not checked separately
     - Verified with the root key over the byte range ``[0x100, 0xb00)``
     - Verified with the SPL key over header1 and the aligned payload
   * - K3 non-secure
     - Unused
     - header1 verified

       header0 ignored
     - Payload CRC32 in header1
     - Unused
     - Unused
   * - K3 secure
     - RSA-2048 moduli with an eFuse root-hash check
     - Both verified and covered by RSA
     - Covered by RSA but not checked separately
     - Verified with the root key over the byte range ``[0x100, 0xb00)``
     - Verified with the SPL key over header1 and the aligned payload

K1 verifies both signatures even without a programmed root-key hash. In this
case, the root key comes from the image and is not authenticated by hardware,
so the image, keys, and signatures can be replaced together.

K3 uses CRC32 in non-secure boot mode and hardware-rooted RSA in secure boot
mode.

Creating an image
-----------------

Package a K3 SPL payload for non-secure boot mode with::

    $ tools/mkimage -T smtimage -n k3 -d u-boot-spl.bin u-boot-spl-mkimage.bin
