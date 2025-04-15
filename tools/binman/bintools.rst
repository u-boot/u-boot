.. SPDX-License-Identifier: GPL-2.0+

Binman bintool Documentation
============================

This file describes the bintools (binary tools) supported by binman. Bintools
are binman's name for external executables that it runs to generate or process
binaries. It is fairly easy to create new bintools. Just add a new file to the
'btool' directory. You can use existing bintools as examples.



Bintool: bootgen: Generate bootable fsbl image for zynq/zynqmp
--------------------------------------------------------------

This bintools supports running Xilinx "bootgen" in order
to generate a bootable, authenticated image form an SPL.




Bintool: bzip2: Compression/decompression using the bzip2 algorithm
-------------------------------------------------------------------

This bintool supports running `bzip2` to compress and decompress data, as
used by binman.

It is also possible to fetch the tool, which uses `apt` to install it.

Documentation is available via::

    man bzip2



Bintool: cbfstool: Coreboot filesystem (CBFS) tool
--------------------------------------------------

This bintool supports creating new CBFS images and adding files to an
existing image, i.e. the features needed by binman.

It also supports fetching a binary cbfstool, since building it from source
is fairly slow.

Documentation about CBFS is at https://www.coreboot.org/CBFS



Bintool: cst: Image generation for U-Boot
-----------------------------------------

This bintool supports running `cst` with some basic parameters as
needed by binman.

cst (imx code signing tool) is used for sigining bootloader binaries for
various i.MX SoCs.

See `Code Signing Tool Users Guide`_ for more information.

.. _`Code Signing Tool Users Guide`:
    https://community.nxp.com/pwmxy87654/attachments/pwmxy87654/imx-processors/202591/1/CST_UG.pdf



Bintool: fdt_add_pubkey: Add public key to control dtb (spl or u-boot proper)
-----------------------------------------------------------------------------

This bintool supports running `fdt_add_pubkey`.

Normally mkimage adds signature information to the control dtb. However
binman images are built independent from each other. Thus it is required
to add the public key separately from mkimage.



Bintool: fdtgrep: Handles the 'fdtgrep' tool
--------------------------------------------

This bintool supports running `fdtgrep` with parameters suitable for
producing SPL devicetrees from the main one.



Bintool: fiptool: Image generation for ARM Trusted Firmware
-----------------------------------------------------------

This bintool supports running `fiptool` with some basic parameters as
neeed by binman.

It also supports build fiptool from source.

fiptool provides a way to package firmware in an ARM Trusted Firmware
Firmware Image Package (ATF FIP) format. It is used with Trusted Firmware A,
for example.

See `TF-A FIP tool documentation`_ for more information.

.. _`TF-A FIP tool documentation`:
    https://trustedfirmware-a.readthedocs.io/en/latest/getting_started/tools-build.html?highlight=fiptool#building-and-using-the-fip-tool



Bintool: futility: Handles the 'futility' tool
----------------------------------------------

futility (flash utility) is a tool for working with Chromium OS flash
images. This Bintool implements just the features used by Binman, related to
GBB creation and firmware signing.

A binary version of the tool can be fetched.

See `Chromium OS vboot documentation`_ for more information.

.. _`Chromium OS vboot documentation`:
    https://chromium.googlesource.com/chromiumos/platform/vboot/+/refs/heads/main/_vboot_reference/README



Bintool: gzip: Compression/decompression using the gzip algorithm
-----------------------------------------------------------------

This bintool supports running `gzip` to compress and decompress data, as
used by binman.

It is also possible to fetch the tool, which uses `apt` to install it.

Documentation is available via::

    man gzip



Bintool: ifwitool: Handles the 'ifwitool' tool
----------------------------------------------

This bintool supports running `ifwitool` with some basic parameters as
neeed by binman. It includes creating a file from a FIT as well as adding,
replacing, deleting and extracting subparts.

The tool is built as part of U-Boot, but a binary version can be fetched if
required.

ifwitool provides a way to package firmware in an Intel Firmware Image
(IFWI) file on some Intel SoCs, e.g. Apolo Lake.



Bintool: lz4: Compression/decompression using the LZ4 algorithm
---------------------------------------------------------------

This bintool supports running `lz4` to compress and decompress data, as
used by binman.

It is also possible to fetch the tool, which uses `apt` to install it.

Documentation is available via::

    man lz4



Bintool: lzma_alone: Compression/decompression using the LZMA algorithm
-----------------------------------------------------------------------

This bintool supports running `lzma_alone` to compress and decompress data,
as used by binman.

It is also possible to fetch the tool, which uses `apt` to install it.

Documentation is available via::

    man lzma_alone



Bintool: lzop: Compression/decompression using the lzop algorithm
-----------------------------------------------------------------

This bintool supports running `lzop` to compress and decompress data, as
used by binman.

It is also possible to fetch the tool, which uses `apt` to install it.

Documentation is available via::

    man lzop



Bintool: mkeficapsule: Handles the 'mkeficapsule' tool
------------------------------------------------------

This bintool is used for generating the EFI capsules. The
capsule generation parameters can either be specified through
commandline, or through a config file.



Bintool: mkimage: Image generation for U-Boot
---------------------------------------------

This bintool supports running `mkimage` with some basic parameters as
needed by binman.

Normally binman uses the mkimage built by U-Boot. But when run outside the
U-Boot build system, binman can use the version installed in your system.
Support is provided for fetching this on Debian-like systems, using apt.



Bintool: openssl: openssl tool
------------------------------

This bintool supports creating new openssl certificates.

It also supports fetching a binary openssl

Documentation about openssl is at https://www.openssl.org/



Bintool: xz: Compression/decompression using the xz algorithm
-------------------------------------------------------------

This bintool supports running `xz` to compress and decompress data, as
used by binman.

It is also possible to fetch the tool, which uses `apt` to install it.

Documentation is available via::

    man xz



Bintool: zstd: Compression/decompression using the zstd algorithm
-----------------------------------------------------------------

This bintool supports running `zstd` to compress and decompress data, as
used by binman.

It is also possible to fetch the tool, which uses `apt` to install it.

Documentation is available via::

    man zstd



