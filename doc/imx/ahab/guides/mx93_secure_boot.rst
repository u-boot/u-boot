.. SPDX-License-Identifier: GPL-2.0+

i.MX93 Secure boot guide using AHAB
===================================

This document provides a step-by-step guide on how to securely boot an
i.MX93 boot image. It is assumed that the reader is familiar with basic
AHAB concepts and with the PKI tree generation. Details about AHAB can be
found in the :doc:`AHAB introduction <../introduction_ahab>` document and
in the processor Security Reference Manual (SRM). The i.MX93 signing
procedure is integrated in the U-Boot build thanks to binman.

1. Preparing the environment to build a secure boot image
---------------------------------------------------------

Secure boot image preparation requires the U-Boot build system to build
the image and the Code Signing Tool (CST) to sign it.

The following files are needed to prepare the boot image:

  - EdgeLock secure enclave Firmware (ELE) (Optional).
  - DDR PHY initialization firmware.
  - U-Boot proper and SPL.
  - ARM Trusted Firmware (ATF).
  - OPTEE (Optional)

The ELE FW is distributed by NXP and is already signed using NXP keys.
In the following sections, <work> designates the repository where all
parts have been downloaded and built.

2. Preparing U-Boot to support AHAB secure boot features
--------------------------------------------------------

The U-Boot/SPL provides extra AHAB supported functionalities that include
extension of Root of Trust, checking any events(issues) after image
authentication, chip lifecycle status, securing the target etc.

The support is enabled by adding CONFIG_AHAB_BOOT to the defconfig file
used by your target:

  - Defconfig:
    CONFIG_AHAB_BOOT=y
  - Kconfig:
    ARM architecture -> Support i.MX9 AHAB features

Enabling this feature allows the SPL and U-Boot images to extend the Root
of Trust by using the AHAB API call via ELE FW. It also enables binman to
automatically sign the SPL and U-Boot containers while building
flash.bin, as described in the next sections.

3. i.MX93 AHAB image layout
---------------------------

The i.MX93 flash.bin image is built by binman and is composed of up to
three containers. The ELE FW container is optional and is provided by NXP,
it is appended at the beginning of the image when the file
mx93a1-ahab-container.img is present in the build directory. The SPL and
U-Boot containers are built by the nxp-imx9image etype and signed by the
nxp-imx93cst etype.

.. code-block:: text

              *start ----> +---------------------------+ ---------
                           |   1st Container header    |   ^
                           |       and signature       |   |  Authenticated at
                           +---------------------------+   |  ELE ROM/FW Level
                           |          Padding          |   |
                           +---------------------------+   |
                           |           ELE FW          |   v
       *start + 0x400 ---> +---------------------------+ ---------
                           |   2nd Container header    |   ^
                           |       and signature       |   |  Authenticated at
                           +---------------------------+   |  ELE ROM/FW Level
                           |          Padding          |   |
                           +---------------------------+   |
                           |            SPL            |   v
                           +---------------------------+ ---------
                           |   3rd Container header    |   ^
                           |       and signature       |   |  Authenticated at
                           +---------------------------+   |  SPL Level
                           |          Padding          |   |
                           +---------------------------+   |
                           |    ARM Trusted FW (ATF)   |   |
                           +---------------------------+   |
                           |       U-Boot proper       |   |
                           +---------------------------+   |
                           |      OPTEE (optional)     |   v
                           +---------------------------+ ---------

The first container includes the ELE FW which is signed using NXP keys,
this container is authenticated by ELE ROM.

The second container includes the SPL which is signed using OEM keys,
this container is authenticated at ELE FW level.

The third container includes the U-Boot proper and the ATF. The SPL is in
charge to load this container and also to interface with ELE FW to
authenticate the additional container.

The signing procedure is slightly different when compared with HABv4
series. On AHAB the signature is directly included in the container, the
CST is responsible to sign and handle the "Signature Block":

.. code-block:: text

               +----------------------------+ ---------
               |                            |  ^
               |                            |  |
               |      Container header      |  |
               |                            |  |
               |                            |  |
               +---+------------------------+  |
               | S | Signature block header |  | Signed
               | i +------------------------+  |
               | g |                        |  |
               | n |                        |  |
               | a |        SRK table       |  |
               | t |                        |  |
               | u |                        |  v
               | r +------------------------+ ---------
               | e |       Signature        |
               |   +------------------------+
               | B |                        |
               | l |        SGK Key         |
               | o | Certificate (optional) |
               | c |                        |
               | k |                        |
               +---+------------------------+

The certificate block is divided into:

.. code-block:: text

                      +---------------+ ^
                      |  Public key   | | Signed
                      |  Permission   | |
                      +---------------+ v
                      |   Signature   |
                      +---------------+

The first block (public key permission) verifies the Signature block
preceding (between SRK table and Certificate blocks), while the second
block (signature) is verified by the SRK table block.

In case not using the subordinate key, the container signature is
verified against the SRK keys directly.

4. Signing the images
---------------------

Internally, Command Sequence Files (CSF) are used. The CSF files contain
all the commands that the AHAB executes during the secure boot. These
commands instruct the AHAB code on which memory areas of the image to
authenticate, which keys to install and use, and so on. The CSF is generated
using the CST Code Signing Tool based on input configuration file. This
tool input configuration file is generated using binman, and the tool
is invoked from binman as well.

The SPL and U-Boot containers built by the nxp-imx9image etype are each
wrapped in their own nxp-imx93cst node and signed individually by the CST
tool. For each container, the container header address and the signature
block offset are read from the i.MX container header itself, so the offsets
in the CST configuration file are always correct, whether the ELE FW is
bundled in the SPL container or not.

Per default the AHAB keys and certificates need to be located in the
build directory, this means creating a symbolic link or copying the
following files from the AHAB PKI tree directory into the u-boot build
directory for the CST Code Signing Tool to locate them:

- ``crts/SRK_1_2_3_4_table.bin``
- ``crts/SRK1_sha384_secp384r1_v3_usr_crt.pem``
- ``keys/SRK1_sha384_secp384r1_v3_usr_key.pem``
- ``keys/key_pass.txt``

See the :doc:`AHAB introduction <../introduction_ahab>` document for the
PKI tree generation procedure (ahab_pki_tree tool) and for the SRK Table
generation (srktool).

The nxp-imx93cst etype is configurable using either DT properties or
environment variables. The following DT properties and environment
variables are supported. Note that environment variables override DT
properties.

.. list-table::
   :header-rows: 1

   * - DT property
     - Variable
     - Description
   * - ``nxp,srk-table``
     - ``SRK_TABLE``
     - full path to ``SRK_1_2_3_4_table.bin``
   * - ``nxp,srk-crt``
     - ``SRK_KEY``
     - full path to the SRK certificate ``SRK1_sha384_secp384r1_v3_usr_crt.pem``


The SRK private key (``SRK1_sha384_secp384r1_v3_usr_key.pem``) and the
``key_pass.txt`` file, which contains the passphrase protecting the private
keys, are required by the CST tool as well, and will be searched for in
relation to the SRK table and certificate paths.

When the key material is not located in the build directory, environment
variables can be set to point the build process to it. In the example
below, CST_DIR designates the CST installation directory, that is the
directory holding the crts/ and keys/ directories generated when creating
the PKI tree:

.. code-block:: console

   $ export CST_DIR=<path_to_cst_dir>
   $ export SRK_TABLE=$CST_DIR/crts/SRK_1_2_3_4_table.bin
   $ export SRK_KEY=$CST_DIR/crts/SRK1_sha384_secp384r1_v3_usr_crt.pem
   $ make flash.bin

5. Programming SRK Hash
-----------------------

As explained in the :doc:`AHAB introduction <../introduction_ahab>`
document, the SRK Hash fuse values are generated by the srktool and
should be programmed in the SoC SRK_HASH[255:0] fuses.

Be careful when programming these values, as this data is the basis for
the root of trust. An error in SRK Hash results in a part that does not
boot.

The U-Boot fuse tool can be used for programming eFuses on i.MX SoCs.

- Dump SRK Hash fuses values in host machine:

  On i.MX93 family, the SRK Hash uses sha256 and dump 8 words fuses:

  .. code-block:: console

     $ od -t x4 SRK_1_2_3_4_fuse.bin
     0000000 db2959f2 90dfc39c 53394566 e0b75829
     0000020 85e6f3b1 af00983d e5e804fe 7a451024

- Program SRK_HASH[255:0] fuses:

On i.MX93:

.. code-block:: console

   => fuse prog 16 0 0xdb2959f2
   => fuse prog 16 1 0x90dfc39c
   => fuse prog 16 2 0x53394566
   => fuse prog 16 3 0xe0b75829
   => fuse prog 16 4 0x85e6f3b1
   => fuse prog 16 5 0xaf00983d
   => fuse prog 16 6 0xe5e804fe
   => fuse prog 16 7 0x7a451024

6. Verify AHAB events
---------------------

If the fuses have been burned properly, there should be no AHAB events
after boot. To validate this, power on the board, and run ahab_status
command on U-Boot terminal.

No events should be returned after this command:

.. code-block:: console

   => ahab_status
   Lifecycle: 0x00000008, OEM Open

           No Events Found!

7. Close the device
-------------------
.. warning::

   Before closing the device, please ensure your sample is in OEM Open state,
   OEM SRK hash has been fused, and you are able to boot a signed image
   successfully without any AHAB events reported . If not, your sample will be
   unrecoverable.

After the device successfully boots a signed image without generating any
AHAB security events, it is safe to close the device. The chip lifecycle
should be changed from OEM open to OEM closed. Be aware this step can
damage your board if a previous step failed. It is also irreversible. Run
on the U-Boot terminal:

.. code-block:: console

   => ahab_close

Now reboot the target, and run:

.. code-block:: console

   => ahab_status
   Lifecycle: 0x00000020, OEM Closed

           No Events Found!
