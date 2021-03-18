.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2016 Google, Inc

Introduction
============

Firmware often consists of several components which must be packaged together.
For example, we may have SPL, U-Boot, a device tree and an environment area
grouped together and placed in MMC flash. When the system starts, it must be
able to find these pieces.

Building firmware should be separate from packaging it. Many of the complexities
of modern firmware build systems come from trying to do both at once. With
binman, you build all the pieces that are needed, using whatever assortment of
projects and build systems are needed, then use binman to stitch everything
together.


What it does
------------

Binman reads your board's device tree and finds a node which describes the
required image layout. It uses this to work out what to place where.

Binman provides a mechanism for building images, from simple SPL + U-Boot
combinations, to more complex arrangements with many parts. It also allows
users to inspect images, extract and replace binaries within them, repacking if
needed.


Features
--------

Apart from basic padding, alignment and positioning features, Binman supports
hierarchical images, compression, hashing and dealing with the binary blobs
which are a sad trend in open-source firmware at present.

Executable binaries can access the location of other binaries in an image by
using special linker symbols (zero-overhead but somewhat limited) or by reading
the devicetree description of the image.

Binman is designed primarily for use with U-Boot and associated binaries such
as ARM Trusted Firmware, but it is suitable for use with other projects, such
as Zephyr. Binman also provides facilities useful in Chromium OS, such as CBFS,
vblocks and and the like.

Binman provides a way to process binaries before they are included, by adding a
Python plug-in.

Binman is intended for use with U-Boot but is designed to be general enough
to be useful in other image-packaging situations.


Motivation
----------

As mentioned above, packaging of firmware is quite a different task from
building the various parts. In many cases the various binaries which go into
the image come from separate build systems. For example, ARM Trusted Firmware
is used on ARMv8 devices but is not built in the U-Boot tree. If a Linux kernel
is included in the firmware image, it is built elsewhere.

It is of course possible to add more and more build rules to the U-Boot
build system to cover these cases. It can shell out to other Makefiles and
build scripts. But it seems better to create a clear divide between building
software and packaging it.

At present this is handled by manual instructions, different for each board,
on how to create images that will boot. By turning these instructions into a
standard format, we can support making valid images for any board without
manual effort, lots of READMEs, etc.

Benefits:

  - Each binary can have its own build system and tool chain without creating
    any dependencies between them
  - Avoids the need for a single-shot build: individual parts can be updated
    and brought in as needed
  - Provides for a standard image description available in the build and at
    run-time
  - SoC-specific image-signing tools can be accommodated
  - Avoids cluttering the U-Boot build system with image-building code
  - The image description is automatically available at run-time in U-Boot,
    SPL. It can be made available to other software also
  - The image description is easily readable (it's a text file in device-tree
    format) and permits flexible packing of binaries


Terminology
-----------

Binman uses the following terms:

- image - an output file containing a firmware image
- binary - an input binary that goes into the image


Relationship to FIT
-------------------

FIT is U-Boot's official image format. It supports multiple binaries with
load / execution addresses, compression. It also supports verification
through hashing and RSA signatures.

FIT was originally designed to support booting a Linux kernel (with an
optional ramdisk) and device tree chosen from various options in the FIT.
Now that U-Boot supports configuration via device tree, it is possible to
load U-Boot from a FIT, with the device tree chosen by SPL.

Binman considers FIT to be one of the binaries it can place in the image.

Where possible it is best to put as much as possible in the FIT, with binman
used to deal with cases not covered by FIT. Examples include initial
execution (since FIT itself does not have an executable header) and dealing
with device boundaries, such as the read-only/read-write separation in SPI
flash.

For U-Boot, binman should not be used to create ad-hoc images in place of
FIT.


Relationship to mkimage
-----------------------

The mkimage tool provides a means to create a FIT. Traditionally it has
needed an image description file: a device tree, like binman, but in a
different format. More recently it has started to support a '-f auto' mode
which can generate that automatically.

More relevant to binman, mkimage also permits creation of many SoC-specific
image types. These can be listed by running 'mkimage -T list'. Examples
include 'rksd', the Rockchip SD/MMC boot format. The mkimage tool is often
called from the U-Boot build system for this reason.

Binman considers the output files created by mkimage to be binary blobs
which it can place in an image. Binman does not replace the mkimage tool or
this purpose. It would be possible in some situations to create a new entry
type for the images in mkimage, but this would not add functionality. It
seems better to use the mkimage tool to generate binaries and avoid blurring
the boundaries between building input files (mkimage) and packaging then
into a final image (binman).


Using binman
============

Example use of binman in U-Boot
-------------------------------

Binman aims to replace some of the ad-hoc image creation in the U-Boot
build system.

Consider sunxi. It has the following steps:

  #. It uses a custom mksunxiboot tool to build an SPL image called
     sunxi-spl.bin. This should probably move into mkimage.

  #. It uses mkimage to package U-Boot into a legacy image file (so that it can
     hold the load and execution address) called u-boot.img.

  #. It builds a final output image called u-boot-sunxi-with-spl.bin which
     consists of sunxi-spl.bin, some padding and u-boot.img.

Binman is intended to replace the last step. The U-Boot build system builds
u-boot.bin and sunxi-spl.bin. Binman can then take over creation of
sunxi-spl.bin (by calling mksunxiboot, or hopefully one day mkimage). In any
case, it would then create the image from the component parts.

This simplifies the U-Boot Makefile somewhat, since various pieces of logic
can be replaced by a call to binman.


Example use of binman for x86
-----------------------------

In most cases x86 images have a lot of binary blobs, 'black-box' code
provided by Intel which must be run for the platform to work. Typically
these blobs are not relocatable and must be placed at fixed areas in the
firmware image.

Currently this is handled by ifdtool, which places microcode, FSP, MRC, VGA
BIOS, reference code and Intel ME binaries into a u-boot.rom file.

Binman is intended to replace all of this, with ifdtool left to handle only
the configuration of the Intel-format descriptor.


Running binman
--------------

First install prerequisites, e.g::

    sudo apt-get install python-pyelftools python3-pyelftools lzma-alone \
        liblz4-tool

Type::

    binman build -b <board_name>

to build an image for a board. The board name is the same name used when
configuring U-Boot (e.g. for sandbox_defconfig the board name is 'sandbox').
Binman assumes that the input files for the build are in ../b/<board_name>.

Or you can specify this explicitly::

    binman build -I <build_path>

where <build_path> is the build directory containing the output of the U-Boot
build.

(Future work will make this more configurable)

In either case, binman picks up the device tree file (u-boot.dtb) and looks
for its instructions in the 'binman' node.

Binman has a few other options which you can see by running 'binman -h'.


Enabling binman for a board
---------------------------

At present binman is invoked from a rule in the main Makefile. You should be
able to enable CONFIG_BINMAN to enable this rule.

The output file is typically named image.bin and is located in the output
directory. If input files are needed to you add these to INPUTS-y either in the
main Makefile or in a config.mk file in your arch subdirectory.

Once binman is executed it will pick up its instructions from a device-tree
file, typically <soc>-u-boot.dtsi, where <soc> is your CONFIG_SYS_SOC value.
You can use other, more specific CONFIG options - see 'Automatic .dtsi
inclusion' below.


Access to binman entry offsets at run time (symbols)
----------------------------------------------------

Binman assembles images and determines where each entry is placed in the image.
This information may be useful to U-Boot at run time. For example, in SPL it
is useful to be able to find the location of U-Boot so that it can be executed
when SPL is finished.

Binman allows you to declare symbols in the SPL image which are filled in
with their correct values during the build. For example::

    binman_sym_declare(ulong, u_boot_any, image_pos);

declares a ulong value which will be assigned to the image-pos of any U-Boot
image (u-boot.bin, u-boot.img, u-boot-nodtb.bin) that is present in the image.
You can access this value with something like::

    ulong u_boot_offset = binman_sym(ulong, u_boot_any, image_pos);

Thus u_boot_offset will be set to the image-pos of U-Boot in memory, assuming
that the whole image has been loaded, or is available in flash. You can then
jump to that address to start U-Boot.

At present this feature is only supported in SPL and TPL. In principle it is
possible to fill in such symbols in U-Boot proper, as well, but a future C
library is planned for this instead, to read from the device tree.

As well as image-pos, it is possible to read the size of an entry and its
offset (which is the start position of the entry within its parent).

A small technical note: Binman automatically adds the base address of the image
(i.e. __image_copy_start) to the value of the image-pos symbol, so that when the
image is loaded to its linked address, the value will be correct and actually
point into the image.

For example, say SPL is at the start of the image and linked to start at address
80108000. If U-Boot's image-pos is 0x8000 then binman will write an image-pos
for U-Boot of 80110000 into the SPL binary, since it assumes the image is loaded
to 80108000, with SPL at 80108000 and U-Boot at 80110000.

For x86 devices (with the end-at-4gb property) this base address is not added
since it is assumed that images are XIP and the offsets already include the
address.


Access to binman entry offsets at run time (fdt)
------------------------------------------------

Binman can update the U-Boot FDT to include the final position and size of
each entry in the images it processes. The option to enable this is -u and it
causes binman to make sure that the 'offset', 'image-pos' and 'size' properties
are set correctly for every entry. Since it is not necessary to specify these in
the image definition, binman calculates the final values and writes these to
the device tree. These can be used by U-Boot at run-time to find the location
of each entry.

Alternatively, an FDT map entry can be used to add a special FDT containing
just the information about the image. This is preceded by a magic string so can
be located anywhere in the image. An image header (typically at the start or end
of the image) can be used to point to the FDT map. See fdtmap and image-header
entries for more information.


Map files
---------

The -m option causes binman to output a .map file for each image that it
generates. This shows the offset and size of each entry. For example::

      Offset      Size  Name
    00000000  00000028  main-section
     00000000  00000010  section@0
      00000000  00000004  u-boot
     00000010  00000010  section@1
      00000000  00000004  u-boot

This shows a hierarchical image with two sections, each with a single entry. The
offsets of the sections are absolute hex byte offsets within the image. The
offsets of the entries are relative to their respective sections. The size of
each entry is also shown, in bytes (hex). The indentation shows the entries
nested inside their sections.


Passing command-line arguments to entries
-----------------------------------------

Sometimes it is useful to pass binman the value of an entry property from the
command line. For example some entries need access to files and it is not
always convenient to put these filenames in the image definition (device tree).

The-a option supports this::

    -a<prop>=<value>

where::

    <prop> is the property to set
    <value> is the value to set it to

Not all properties can be provided this way. Only some entries support it,
typically for filenames.


Image description format
========================

The binman node is called 'binman'. An example image description is shown
below::

    binman {
        filename = "u-boot-sunxi-with-spl.bin";
        pad-byte = <0xff>;
        blob {
            filename = "spl/sunxi-spl.bin";
        };
        u-boot {
            offset = <CONFIG_SPL_PAD_TO>;
        };
    };


This requests binman to create an image file called u-boot-sunxi-with-spl.bin
consisting of a specially formatted SPL (spl/sunxi-spl.bin, built by the
normal U-Boot Makefile), some 0xff padding, and a U-Boot legacy image. The
padding comes from the fact that the second binary is placed at
CONFIG_SPL_PAD_TO. If that line were omitted then the U-Boot binary would
immediately follow the SPL binary.

The binman node describes an image. The sub-nodes describe entries in the
image. Each entry represents a region within the overall image. The name of
the entry (blob, u-boot) tells binman what to put there. For 'blob' we must
provide a filename. For 'u-boot', binman knows that this means 'u-boot.bin'.

Entries are normally placed into the image sequentially, one after the other.
The image size is the total size of all entries. As you can see, you can
specify the start offset of an entry using the 'offset' property.

Note that due to a device tree requirement, all entries must have a unique
name. If you want to put the same binary in the image multiple times, you can
use any unique name, with the 'type' property providing the type.

The attributes supported for entries are described below.

offset:
    This sets the offset of an entry within the image or section containing
    it. The first byte of the image is normally at offset 0. If 'offset' is
    not provided, binman sets it to the end of the previous region, or the
    start of the image's entry area (normally 0) if there is no previous
    region.

align:
    This sets the alignment of the entry. The entry offset is adjusted
    so that the entry starts on an aligned boundary within the containing
    section or image. For example 'align = <16>' means that the entry will
    start on a 16-byte boundary. This may mean that padding is added before
    the entry. The padding is part of the containing section but is not
    included in the entry, meaning that an empty space may be created before
    the entry starts. Alignment should be a power of 2. If 'align' is not
    provided, no alignment is performed.

size:
    This sets the size of the entry. The contents will be padded out to
    this size. If this is not provided, it will be set to the size of the
    contents.

pad-before:
    Padding before the contents of the entry. Normally this is 0, meaning
    that the contents start at the beginning of the entry. This can be used
    to offset the entry contents a little. While this does not affect the
    contents of the entry within binman itself (the padding is performed
    only when its parent section is assembled), the end result will be that
    the entry starts with the padding bytes, so may grow. Defaults to 0.

pad-after:
    Padding after the contents of the entry. Normally this is 0, meaning
    that the entry ends at the last byte of content (unless adjusted by
    other properties). This allows room to be created in the image for
    this entry to expand later. While this does not affect the contents of
    the entry within binman itself (the padding is performed only when its
    parent section is assembled), the end result will be that the entry ends
    with the padding bytes, so may grow. Defaults to 0.

align-size:
    This sets the alignment of the entry size. For example, to ensure
    that the size of an entry is a multiple of 64 bytes, set this to 64.
    While this does not affect the contents of the entry within binman
    itself (the padding is performed only when its parent section is
    assembled), the end result is that the entry ends with the padding
    bytes, so may grow. If 'align-size' is not provided, no alignment is
    performed.

align-end:
    This sets the alignment of the end of an entry with respect to the
    containing section. Some entries require that they end on an alignment
    boundary, regardless of where they start. This does not move the start
    of the entry, so the contents of the entry will still start at the
    beginning. But there may be padding at the end. While this does not
    affect the contents of the entry within binman itself (the padding is
    performed only when its parent section is assembled), the end result
    is that the entry ends with the padding bytes, so may grow.
    If 'align-end' is not provided, no alignment is performed.

filename:
    For 'blob' types this provides the filename containing the binary to
    put into the entry. If binman knows about the entry type (like
    u-boot-bin), then there is no need to specify this.

type:
    Sets the type of an entry. This defaults to the entry name, but it is
    possible to use any name, and then add (for example) 'type = "u-boot"'
    to specify the type.

offset-unset:
    Indicates that the offset of this entry should not be set by placing
    it immediately after the entry before. Instead, is set by another
    entry which knows where this entry should go. When this boolean
    property is present, binman will give an error if another entry does
    not set the offset (with the GetOffsets() method).

image-pos:
    This cannot be set on entry (or at least it is ignored if it is), but
    with the -u option, binman will set it to the absolute image position
    for each entry. This makes it easy to find out exactly where the entry
    ended up in the image, regardless of parent sections, etc.

expand-size:
    Expand the size of this entry to fit available space. This space is only
    limited by the size of the image/section and the position of the next
    entry.

compress:
    Sets the compression algortihm to use (for blobs only). See the entry
    documentation for details.

missing-msg:
    Sets the tag of the message to show if this entry is missing. This is
    used for external blobs. When they are missing it is helpful to show
    information about what needs to be fixed. See missing-blob-help for the
    message for each tag.

The attributes supported for images and sections are described below. Several
are similar to those for entries.

size:
    Sets the image size in bytes, for example 'size = <0x100000>' for a
    1MB image.

offset:
    This is similar to 'offset' in entries, setting the offset of a section
    within the image or section containing it. The first byte of the section
    is normally at offset 0. If 'offset' is not provided, binman sets it to
    the end of the previous region, or the start of the image's entry area
    (normally 0) if there is no previous region.

align-size:
    This sets the alignment of the image size. For example, to ensure
    that the image ends on a 512-byte boundary, use 'align-size = <512>'.
    If 'align-size' is not provided, no alignment is performed.

pad-before:
    This sets the padding before the image entries. The first entry will
    be positioned after the padding. This defaults to 0.

pad-after:
    This sets the padding after the image entries. The padding will be
    placed after the last entry. This defaults to 0.

pad-byte:
    This specifies the pad byte to use when padding in the image. It
    defaults to 0. To use 0xff, you would add 'pad-byte = <0xff>'.

filename:
    This specifies the image filename. It defaults to 'image.bin'.

sort-by-offset:
    This causes binman to reorder the entries as needed to make sure they
    are in increasing positional order. This can be used when your entry
    order may not match the positional order. A common situation is where
    the 'offset' properties are set by CONFIG options, so their ordering is
    not known a priori.

    This is a boolean property so needs no value. To enable it, add a
    line 'sort-by-offset;' to your description.

multiple-images:
    Normally only a single image is generated. To create more than one
    image, put this property in the binman node. For example, this will
    create image1.bin containing u-boot.bin, and image2.bin containing
    both spl/u-boot-spl.bin and u-boot.bin::

        binman {
            multiple-images;
            image1 {
                u-boot {
                };
            };

            image2 {
                spl {
                };
                u-boot {
                };
            };
        };

end-at-4gb:
    For x86 machines the ROM offsets start just before 4GB and extend
    up so that the image finished at the 4GB boundary. This boolean
    option can be enabled to support this. The image size must be
    provided so that binman knows when the image should start. For an
    8MB ROM, the offset of the first entry would be 0xfff80000 with
    this option, instead of 0 without this option.

skip-at-start:
    This property specifies the entry offset of the first entry.

    For PowerPC mpc85xx based CPU, CONFIG_SYS_TEXT_BASE is the entry
    offset of the first entry. It can be 0xeff40000 or 0xfff40000 for
    nor flash boot, 0x201000 for sd boot etc.

    'end-at-4gb' property is not applicable where CONFIG_SYS_TEXT_BASE +
    Image size != 4gb.

Examples of the above options can be found in the tests. See the
tools/binman/test directory.

It is possible to have the same binary appear multiple times in the image,
either by using a unit number suffix (u-boot@0, u-boot@1) or by using a
different name for each and specifying the type with the 'type' attribute.


Sections and hierachical images
-------------------------------

Sometimes it is convenient to split an image into several pieces, each of which
contains its own set of binaries. An example is a flash device where part of
the image is read-only and part is read-write. We can set up sections for each
of these, and place binaries in them independently. The image is still produced
as a single output file.

This feature provides a way of creating hierarchical images. For example here
is an example image with two copies of U-Boot. One is read-only (ro), intended
to be written only in the factory. Another is read-write (rw), so that it can be
upgraded in the field. The sizes are fixed so that the ro/rw boundary is known
and can be programmed::

    binman {
        section@0 {
            read-only;
            name-prefix = "ro-";
            size = <0x100000>;
            u-boot {
            };
        };
        section@1 {
            name-prefix = "rw-";
            size = <0x100000>;
            u-boot {
            };
        };
    };

This image could be placed into a SPI flash chip, with the protection boundary
set at 1MB.

A few special properties are provided for sections:

read-only:
    Indicates that this section is read-only. This has no impact on binman's
    operation, but his property can be read at run time.

name-prefix:
    This string is prepended to all the names of the binaries in the
    section. In the example above, the 'u-boot' binaries which actually be
    renamed to 'ro-u-boot' and 'rw-u-boot'. This can be useful to
    distinguish binaries with otherwise identical names.


Image Properties
----------------

Image nodes act like sections but also have a few extra properties:

filename:
    Output filename for the image. This defaults to image.bin (or in the
    case of multiple images <nodename>.bin where <nodename> is the name of
    the image node.

allow-repack:
    Create an image that can be repacked. With this option it is possible
    to change anything in the image after it is created, including updating
    the position and size of image components. By default this is not
    permitted since it is not possibly to know whether this might violate a
    constraint in the image description. For example, if a section has to
    increase in size to hold a larger binary, that might cause the section
    to fall out of its allow region (e.g. read-only portion of flash).

    Adding this property causes the original offset and size values in the
    image description to be stored in the FDT and fdtmap.


Hashing Entries
---------------

It is possible to ask binman to hash the contents of an entry and write that
value back to the device-tree node. For example::

    binman {
        u-boot {
            hash {
                algo = "sha256";
            };
        };
    };

Here, a new 'value' property will be written to the 'hash' node containing
the hash of the 'u-boot' entry. Only SHA256 is supported at present. Whole
sections can be hased if desired, by adding the 'hash' node to the section.

The has value can be chcked at runtime by hashing the data actually read and
comparing this has to the value in the device tree.


Expanded entries
----------------

Binman automatically replaces 'u-boot' with an expanded version of that, i.e.
'u-boot-expanded'. This means that when you write::

    u-boot {
    };

you actually get::

    u-boot {
        type = "u-boot-expanded';
    };

which in turn expands to::

    u-boot {
        type = "section";

        u-boot-nodtb {
        };

        u-boot-dtb {
        };
    };

U-Boot's various phase binaries actually comprise two or three pieces.
For example, u-boot.bin has the executable followed by a devicetree.

With binman we want to be able to update that devicetree with full image
information so that it is accessible to the executable. This is tricky
if it is not clear where the devicetree starts.

The above feature ensures that the devicetree is clearly separated from the
U-Boot executable and can be updated separately by binman as needed. It can be
disabled with the --no-expanded flag if required.

The same applies for u-boot-spl and u-boot-spl. In those cases, the expansion
includes the BSS padding, so for example::

    spl {
        type = "u-boot-spl"
    };

you actually get::

    spl {
        type = "u-boot-expanded';
    };

which in turn expands to::

    spl {
        type = "section";

        u-boot-spl-nodtb {
        };

        u-boot-spl-bss-pad {
        };

        u-boot-spl-dtb {
        };
    };

Of course we should not expand SPL if it has no devicetree. Also if the BSS
padding is not needed (because BSS is in RAM as with CONFIG_SPL_SEPARATE_BSS),
the 'u-boot-spl-bss-pad' subnode should not be created. The use of the expaned
entry type is controlled by the UseExpanded() method. In the SPL case it checks
the 'spl-dtb' entry arg, which is 'y' or '1' if SPL has a devicetree.

For the BSS case, a 'spl-bss-pad' entry arg controls whether it is present. All
entry args are provided by the U-Boot Makefile.


Compression
-----------

Binman support compression for 'blob' entries (those of type 'blob' and
derivatives). To enable this for an entry, add a 'compress' property::

    blob {
        filename = "datafile";
        compress = "lz4";
    };

The entry will then contain the compressed data, using the 'lz4' compression
algorithm. Currently this is the only one that is supported. The uncompressed
size is written to the node in an 'uncomp-size' property, if -u is used.

Compression is also supported for sections. In that case the entire section is
compressed in one block, including all its contents. This means that accessing
an entry from the section required decompressing the entire section. Also, the
size of a section indicates the space that it consumes in its parent section
(and typically the image). With compression, the section may contain more data,
and the uncomp-size property indicates that, as above. The contents of the
section is compressed first, before any padding is added. This ensures that the
padding itself is not compressed, which would be a waste of time.


Automatic .dtsi inclusion
-------------------------

It is sometimes inconvenient to add a 'binman' node to the .dts file for each
board. This can be done by using #include to bring in a common file. Another
approach supported by the U-Boot build system is to automatically include
a common header. You can then put the binman node (and anything else that is
specific to U-Boot, such as u-boot,dm-pre-reloc properies) in that header
file.

Binman will search for the following files in arch/<arch>/dts::

   <dts>-u-boot.dtsi where <dts> is the base name of the .dts file
   <CONFIG_SYS_SOC>-u-boot.dtsi
   <CONFIG_SYS_CPU>-u-boot.dtsi
   <CONFIG_SYS_VENDOR>-u-boot.dtsi
   u-boot.dtsi

U-Boot will only use the first one that it finds. If you need to include a
more general file you can do that from the more specific file using #include.
If you are having trouble figuring out what is going on, you can uncomment
the 'warning' line in scripts/Makefile.lib to see what it has found::

   # Uncomment for debugging
   # This shows all the files that were considered and the one that we chose.
   # u_boot_dtsi_options_debug = $(u_boot_dtsi_options_raw)


Entry Documentation
===================

For details on the various entry types supported by binman and how to use them,
see entries.rst which is generated from the source code using:

    binman entry-docs >tools/binman/entries.rst

.. toctree::
   :maxdepth: 2

   entries


Managing images
===============

Listing images
--------------

It is possible to list the entries in an existing firmware image created by
binman, provided that there is an 'fdtmap' entry in the image. For example::

    $ binman ls -i image.bin
    Name              Image-pos  Size  Entry-type    Offset  Uncomp-size
    ----------------------------------------------------------------------
    main-section                  c00  section            0
      u-boot                  0     4  u-boot             0
      section                     5fc  section            4
        cbfs                100   400  cbfs               0
          u-boot            138     4  u-boot            38
          u-boot-dtb        180   108  u-boot-dtb        80          3b5
        u-boot-dtb          500   1ff  u-boot-dtb       400          3b5
      fdtmap                6fc   381  fdtmap           6fc
      image-header          bf8     8  image-header     bf8

This shows the hierarchy of the image, the position, size and type of each
entry, the offset of each entry within its parent and the uncompressed size if
the entry is compressed.

It is also possible to list just some files in an image, e.g.::

    $ binman ls -i image.bin section/cbfs
    Name              Image-pos  Size  Entry-type  Offset  Uncomp-size
    --------------------------------------------------------------------
        cbfs                100   400  cbfs             0
          u-boot            138     4  u-boot          38
          u-boot-dtb        180   108  u-boot-dtb      80          3b5

or with wildcards::

    $ binman ls -i image.bin "*cb*" "*head*"
    Name              Image-pos  Size  Entry-type    Offset  Uncomp-size
    ----------------------------------------------------------------------
        cbfs                100   400  cbfs               0
          u-boot            138     4  u-boot            38
          u-boot-dtb        180   108  u-boot-dtb        80          3b5
      image-header          bf8     8  image-header     bf8


Extracting files from images
----------------------------

You can extract files from an existing firmware image created by binman,
provided that there is an 'fdtmap' entry in the image. For example::

    $ binman extract -i image.bin section/cbfs/u-boot

which will write the uncompressed contents of that entry to the file 'u-boot' in
the current directory. You can also extract to a particular file, in this case
u-boot.bin::

    $ binman extract -i image.bin section/cbfs/u-boot -f u-boot.bin

It is possible to extract all files into a destination directory, which will
put files in subdirectories matching the entry hierarchy::

    $ binman extract -i image.bin -O outdir

or just a selection::

    $ binman extract -i image.bin "*u-boot*" -O outdir


Replacing files in an image
---------------------------

You can replace files in an existing firmware image created by binman, provided
that there is an 'fdtmap' entry in the image. For example:

    $ binman replace -i image.bin section/cbfs/u-boot

which will write the contents of the file 'u-boot' from the current directory
to the that entry, compressing if necessary. If the entry size changes, you must
add the 'allow-repack' property to the original image before generating it (see
above), otherwise you will get an error.

You can also use a particular file, in this case u-boot.bin::

    $ binman replace -i image.bin section/cbfs/u-boot -f u-boot.bin

It is possible to replace all files from a source directory which uses the same
hierarchy as the entries::

    $ binman replace -i image.bin -I indir

Files that are missing will generate a warning.

You can also replace just a selection of entries::

    $ binman replace -i image.bin "*u-boot*" -I indir


Logging
-------

Binman normally operates silently unless there is an error, in which case it
just displays the error. The -D/--debug option can be used to create a full
backtrace when errors occur. You can use BINMAN_DEBUG=1 when building to select
this.

Internally binman logs some output while it is running. This can be displayed
by increasing the -v/--verbosity from the default of 1:

   0: silent
   1: warnings only
   2: notices (important messages)
   3: info about major operations
   4: detailed information about each operation
   5: debug (all output)

You can use BINMAN_VERBOSE=5 (for example) when building to select this.


Technical details
=================

Order of image creation
-----------------------

Image creation proceeds in the following order, for each entry in the image.

1. AddMissingProperties() - binman can add calculated values to the device
tree as part of its processing, for example the offset and size of each
entry. This method adds any properties associated with this, expanding the
device tree as needed. These properties can have placeholder values which are
set later by SetCalculatedProperties(). By that stage the size of sections
cannot be changed (since it would cause the images to need to be repacked),
but the correct values can be inserted.

2. ProcessFdt() - process the device tree information as required by the
particular entry. This may involve adding or deleting properties. If the
processing is complete, this method should return True. If the processing
cannot complete because it needs the ProcessFdt() method of another entry to
run first, this method should return False, in which case it will be called
again later.

3. GetEntryContents() - the contents of each entry are obtained, normally by
reading from a file. This calls the Entry.ObtainContents() to read the
contents. The default version of Entry.ObtainContents() calls
Entry.GetDefaultFilename() and then reads that file. So a common mechanism
to select a file to read is to override that function in the subclass. The
functions must return True when they have read the contents. Binman will
retry calling the functions a few times if False is returned, allowing
dependencies between the contents of different entries.

4. GetEntryOffsets() - calls Entry.GetOffsets() for each entry. This can
return a dict containing entries that need updating. The key should be the
entry name and the value is a tuple (offset, size). This allows an entry to
provide the offset and size for other entries. The default implementation
of GetEntryOffsets() returns {}.

5. PackEntries() - calls Entry.Pack() which figures out the offset and
size of an entry. The 'current' image offset is passed in, and the function
returns the offset immediately after the entry being packed. The default
implementation of Pack() is usually sufficient.

Note: for sections, this also checks that the entries do not overlap, nor extend
outside the section. If the section does not have a defined size, the size is
set large enough to hold all the entries.

6. SetImagePos() - sets the image position of every entry. This is the absolute
position 'image-pos', as opposed to 'offset' which is relative to the containing
section. This must be done after all offsets are known, which is why it is quite
late in the ordering.

7. SetCalculatedProperties() - update any calculated properties in the device
tree. This sets the correct 'offset' and 'size' vaues, for example.

8. ProcessEntryContents() - this calls Entry.ProcessContents() on each entry.
The default implementatoin does nothing. This can be overriden to adjust the
contents of an entry in some way. For example, it would be possible to create
an entry containing a hash of the contents of some other entries. At this
stage the offset and size of entries should not be adjusted unless absolutely
necessary, since it requires a repack (going back to PackEntries()).

9. ResetForPack() - if the ProcessEntryContents() step failed, in that an entry
has changed its size, then there is no alternative but to go back to step 5 and
try again, repacking the entries with the updated size. ResetForPack() removes
the fixed offset/size values added by binman, so that the packing can start from
scratch.

10. WriteSymbols() - write the value of symbols into the U-Boot SPL binary.
See 'Access to binman entry offsets at run time' below for a description of
what happens in this stage.

11. BuildImage() - builds the image and writes it to a file

12. WriteMap() - writes a text file containing a map of the image. This is the
final step.


External tools
--------------

Binman can make use of external command-line tools to handle processing of
entry contents or to generate entry contents. These tools are executed using
the 'tools' module's Run() method. The tools generally must exist on the PATH,
but the --toolpath option can be used to specify additional search paths to
use. This option can be specified multiple times to add more than one path.

For some compile tools binman will use the versions specified by commonly-used
environment variables like CC and HOSTCC for the C compiler, based on whether
the tool's output will be used for the target or for the host machine. If those
aren't given, it will also try to derive target-specific versions from the
CROSS_COMPILE environment variable during a cross-compilation.


Code coverage
-------------

Binman is a critical tool and is designed to be very testable. Entry
implementations target 100% test coverage. Run 'binman test -T' to check this.

To enable Python test coverage on Debian-type distributions (e.g. Ubuntu)::

   $ sudo apt-get install python-coverage python3-coverage python-pytest


Concurrent tests
----------------

Binman tries to run tests concurrently. This means that the tests make use of
all available CPUs to run.

 To enable this::

   $ sudo apt-get install python-subunit python3-subunit

Use '-P 1' to disable this. It is automatically disabled when code coverage is
being used (-T) since they are incompatible.


Debugging tests
---------------

Sometimes when debugging tests it is useful to keep the input and output
directories so they can be examined later. Use -X or --test-preserve-dirs for
this.


Running tests on non-x86 architectures
--------------------------------------

Binman's tests have been written under the assumption that they'll be run on a
x86-like host and there hasn't been an attempt to make them portable yet.
However, it's possible to run the tests by cross-compiling to x86.

To install an x86 cross-compiler on Debian-type distributions (e.g. Ubuntu)::

  $ sudo apt-get install gcc-x86-64-linux-gnu

Then, you can run the tests under cross-compilation::

  $ CROSS_COMPILE=x86_64-linux-gnu- binman test -T

You can also use gcc-i686-linux-gnu similar to the above.


Writing new entries and debugging
---------------------------------

The behaviour of entries is defined by the Entry class. All other entries are
a subclass of this. An important subclass is Entry_blob which takes binary
data from a file and places it in the entry. In fact most entry types are
subclasses of Entry_blob.

Each entry type is a separate file in the tools/binman/etype directory. Each
file contains a class called Entry_<type> where <type> is the entry type.
New entry types can be supported by adding new files in that directory.
These will automatically be detected by binman when needed.

Entry properties are documented in entry.py. The entry subclasses are free
to change the values of properties to support special behaviour. For example,
when Entry_blob loads a file, it sets content_size to the size of the file.
Entry classes can adjust other entries. For example, an entry that knows
where other entries should be positioned can set up those entries' offsets
so they don't need to be set in the binman decription. It can also adjust
entry contents.

Most of the time such essoteric behaviour is not needed, but it can be
essential for complex images.

If you need to specify a particular device-tree compiler to use, you can define
the DTC environment variable. This can be useful when the system dtc is too
old.

To enable a full backtrace and other debugging features in binman, pass
BINMAN_DEBUG=1 to your build::

   make qemu-x86_defconfig
   make BINMAN_DEBUG=1

To enable verbose logging from binman, base BINMAN_VERBOSE to your build, which
adds a -v<level> option to the call to binman::

   make qemu-x86_defconfig
   make BINMAN_VERBOSE=5


History / Credits
-----------------

Binman takes a lot of inspiration from a Chrome OS tool called
'cros_bundle_firmware', which I wrote some years ago. That tool was based on
a reasonably simple and sound design but has expanded greatly over the
years. In particular its handling of x86 images is convoluted.

Quite a few lessons have been learned which are hopefully applied here.


Design notes
------------

On the face of it, a tool to create firmware images should be fairly simple:
just find all the input binaries and place them at the right place in the
image. The difficulty comes from the wide variety of input types (simple
flat binaries containing code, packaged data with various headers), packing
requirments (alignment, spacing, device boundaries) and other required
features such as hierarchical images.

The design challenge is to make it easy to create simple images, while
allowing the more complex cases to be supported. For example, for most
images we don't much care exactly where each binary ends up, so we should
not have to specify that unnecessarily.

New entry types should aim to provide simple usage where possible. If new
core features are needed, they can be added in the Entry base class.


To do
-----

Some ideas:

- Use of-platdata to make the information available to code that is unable
  to use device tree (such as a very small SPL image). For now, limited info is
  available via linker symbols
- Allow easy building of images by specifying just the board name
- Support building an image for a board (-b) more completely, with a
  configurable build directory
- Detect invalid properties in nodes
- Sort the fdtmap by offset
- Output temporary files to a different directory

--
Simon Glass <sjg@chromium.org>
7/7/2016
