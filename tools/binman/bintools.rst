.. SPDX-License-Identifier: GPL-2.0+

Binman bintool Documentation
============================

This file describes the bintools (binary tools) supported by binman. Bintools
are binman's name for external executables that it runs to generate or process
binaries. It is fairly easy to create new bintools. Just add a new file to the
'btool' directory. You can use existing bintools as examples.



Bintool: cbfstool: Coreboot filesystem (CBFS) tool
--------------------------------------------------

This bintool supports creating new CBFS images and adding files to an
existing image, i.e. the features needed by binman.

It also supports fetching a binary cbfstool, since building it from source
is fairly slow.

Documentation about CBFS is at https://www.coreboot.org/CBFS



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



Bintool: mkimage: Image generation for U-Boot
---------------------------------------------

This bintool supports running `mkimage` with some basic parameters as
neeed by binman.

Normally binman uses the mkimage built by U-Boot. But when run outside the
U-Boot build system, binman can use the version installed in your system.
Support is provided for fetching this on Debian-like systems, using apt.



