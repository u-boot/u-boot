Binman Entry Documentation
===========================

This file describes the entry types supported by binman. These entry types can
be placed in an image one by one to build up a final firmware image. It is
fairly easy to create new entry types. Just add a new file to the 'etype'
directory. You can use the existing entries as examples.

Note that some entries are subclasses of others, using and extending their
features to produce new behaviours.



Entry: atf-bl31: Entry containing an ARM Trusted Firmware (ATF) BL31 blob
-------------------------------------------------------------------------

Properties / Entry arguments:
    - atf-bl31-path: Filename of file to read into entry. This is typically
        called bl31.bin or bl31.elf

This entry holds the run-time firmware, typically started by U-Boot SPL.
See the U-Boot README for your architecture or board for how to use it. See
https://github.com/ARM-software/arm-trusted-firmware for more information
about ATF.



Entry: blob: Entry containing an arbitrary binary blob
------------------------------------------------------

Note: This should not be used by itself. It is normally used as a parent
class by other entry types.

Properties / Entry arguments:
    - filename: Filename of file to read into entry
    - compress: Compression algorithm to use:
        none: No compression
        lz4: Use lz4 compression (via 'lz4' command-line utility)

This entry reads data from a file and places it in the entry. The
default filename is often specified specified by the subclass. See for
example the 'u-boot' entry which provides the filename 'u-boot.bin'.

If compression is enabled, an extra 'uncomp-size' property is written to
the node (if enabled with -u) which provides the uncompressed size of the
data.



Entry: blob-dtb: A blob that holds a device tree
------------------------------------------------

This is a blob containing a device tree. The contents of the blob are
obtained from the list of available device-tree files, managed by the
'state' module.



Entry: blob-ext: Entry containing an externally built binary blob
-----------------------------------------------------------------

Note: This should not be used by itself. It is normally used as a parent
class by other entry types.

If the file providing this blob is missing, binman can optionally ignore it
and produce a broken image with a warning.

See 'blob' for Properties / Entry arguments.



Entry: blob-named-by-arg: A blob entry which gets its filename property from its subclass
-----------------------------------------------------------------------------------------

Properties / Entry arguments:
    - <xxx>-path: Filename containing the contents of this entry (optional,
        defaults to None)

where <xxx> is the blob_fname argument to the constructor.

This entry cannot be used directly. Instead, it is used as a parent class
for another entry, which defined blob_fname. This parameter is used to
set the entry-arg or property containing the filename. The entry-arg or
property is in turn used to set the actual filename.

See cros_ec_rw for an example of this.



Entry: blob-phase: Section that holds a phase binary
----------------------------------------------------

This is a base class that should not normally be used directly. It is used
when converting a 'u-boot' entry automatically into a 'u-boot-expanded'
entry; similarly for SPL.



Entry: cbfs: Entry containing a Coreboot Filesystem (CBFS)
----------------------------------------------------------

A CBFS provides a way to group files into a group. It has a simple directory
structure and allows the position of individual files to be set, since it is
designed to support execute-in-place in an x86 SPI-flash device. Where XIP
is not used, it supports compression and storing ELF files.

CBFS is used by coreboot as its way of orgnanising SPI-flash contents.

The contents of the CBFS are defined by subnodes of the cbfs entry, e.g.::

    cbfs {
        size = <0x100000>;
        u-boot {
            cbfs-type = "raw";
        };
        u-boot-dtb {
            cbfs-type = "raw";
        };
    };

This creates a CBFS 1MB in size two files in it: u-boot.bin and u-boot.dtb.
Note that the size is required since binman does not support calculating it.
The contents of each entry is just what binman would normally provide if it
were not a CBFS node. A blob type can be used to import arbitrary files as
with the second subnode below::

    cbfs {
        size = <0x100000>;
        u-boot {
            cbfs-name = "BOOT";
            cbfs-type = "raw";
        };

        dtb {
            type = "blob";
            filename = "u-boot.dtb";
            cbfs-type = "raw";
            cbfs-compress = "lz4";
            cbfs-offset = <0x100000>;
        };
    };

This creates a CBFS 1MB in size with u-boot.bin (named "BOOT") and
u-boot.dtb (named "dtb") and compressed with the lz4 algorithm.


Properties supported in the top-level CBFS node:

cbfs-arch:
    Defaults to "x86", but you can specify the architecture if needed.


Properties supported in the CBFS entry subnodes:

cbfs-name:
    This is the name of the file created in CBFS. It defaults to the entry
    name (which is the node name), but you can override it with this
    property.

cbfs-type:
    This is the CBFS file type. The following are supported:

    raw:
        This is a 'raw' file, although compression is supported. It can be
        used to store any file in CBFS.

    stage:
        This is an ELF file that has been loaded (i.e. mapped to memory), so
        appears in the CBFS as a flat binary. The input file must be an ELF
        image, for example this puts "u-boot" (the ELF image) into a 'stage'
        entry::

            cbfs {
                size = <0x100000>;
                u-boot-elf {
                    cbfs-name = "BOOT";
                    cbfs-type = "stage";
                };
            };

        You can use your own ELF file with something like::

            cbfs {
                size = <0x100000>;
                something {
                    type = "blob";
                    filename = "cbfs-stage.elf";
                    cbfs-type = "stage";
                };
            };

        As mentioned, the file is converted to a flat binary, so it is
        equivalent to adding "u-boot.bin", for example, but with the load and
        start addresses specified by the ELF. At present there is no option
        to add a flat binary with a load/start address, similar to the
        'add-flat-binary' option in cbfstool.

cbfs-offset:
    This is the offset of the file's data within the CBFS. It is used to
    specify where the file should be placed in cases where a fixed position
    is needed. Typical uses are for code which is not relocatable and must
    execute in-place from a particular address. This works because SPI flash
    is generally mapped into memory on x86 devices. The file header is
    placed before this offset so that the data start lines up exactly with
    the chosen offset. If this property is not provided, then the file is
    placed in the next available spot.

The current implementation supports only a subset of CBFS features. It does
not support other file types (e.g. payload), adding multiple files (like the
'files' entry with a pattern supported by binman), putting files at a
particular offset in the CBFS and a few other things.

Of course binman can create images containing multiple CBFSs, simply by
defining these in the binman config::


    binman {
        size = <0x800000>;
        cbfs {
            offset = <0x100000>;
            size = <0x100000>;
            u-boot {
                cbfs-type = "raw";
            };
            u-boot-dtb {
                cbfs-type = "raw";
            };
        };

        cbfs2 {
            offset = <0x700000>;
            size = <0x100000>;
            u-boot {
                cbfs-type = "raw";
            };
            u-boot-dtb {
                cbfs-type = "raw";
            };
            image {
                type = "blob";
                filename = "image.jpg";
            };
        };
    };

This creates an 8MB image with two CBFSs, one at offset 1MB, one at 7MB,
both of size 1MB.



Entry: cros-ec-rw: A blob entry which contains a Chromium OS read-write EC image
--------------------------------------------------------------------------------

Properties / Entry arguments:
    - cros-ec-rw-path: Filename containing the EC image

This entry holds a Chromium OS EC (embedded controller) image, for use in
updating the EC on startup via software sync.



Entry: fdtmap: An entry which contains an FDT map
-------------------------------------------------

Properties / Entry arguments:
    None

An FDT map is just a header followed by an FDT containing a list of all the
entries in the image. The root node corresponds to the image node in the
original FDT, and an image-name property indicates the image name in that
original tree.

The header is the string _FDTMAP_ followed by 8 unused bytes.

When used, this entry will be populated with an FDT map which reflects the
entries in the current image. Hierarchy is preserved, and all offsets and
sizes are included.

Note that the -u option must be provided to ensure that binman updates the
FDT with the position of each entry.

Example output for a simple image with U-Boot and an FDT map::

    / {
        image-name = "binman";
        size = <0x00000112>;
        image-pos = <0x00000000>;
        offset = <0x00000000>;
        u-boot {
            size = <0x00000004>;
            image-pos = <0x00000000>;
            offset = <0x00000000>;
        };
        fdtmap {
            size = <0x0000010e>;
            image-pos = <0x00000004>;
            offset = <0x00000004>;
        };
    };

If allow-repack is used then 'orig-offset' and 'orig-size' properties are
added as necessary. See the binman README.



Entry: files: Entry containing a set of files
---------------------------------------------

Properties / Entry arguments:
    - pattern: Filename pattern to match the files to include
    - files-compress: Compression algorithm to use:
        none: No compression
        lz4: Use lz4 compression (via 'lz4' command-line utility)
    - files-align: Align each file to the given alignment

This entry reads a number of files and places each in a separate sub-entry
within this entry. To access these you need to enable device-tree updates
at run-time so you can obtain the file positions.



Entry: fill: An entry which is filled to a particular byte value
----------------------------------------------------------------

Properties / Entry arguments:
    - fill-byte: Byte to use to fill the entry

Note that the size property must be set since otherwise this entry does not
know how large it should be.

You can often achieve the same effect using the pad-byte property of the
overall image, in that the space between entries will then be padded with
that byte. But this entry is sometimes useful for explicitly setting the
byte value of a region.



Entry: fit: Entry containing a FIT
----------------------------------

This calls mkimage to create a FIT (U-Boot Flat Image Tree) based on the
input provided.

Nodes for the FIT should be written out in the binman configuration just as
they would be in a file passed to mkimage.

For example, this creates an image containing a FIT with U-Boot SPL::

    binman {
        fit {
            description = "Test FIT";
            fit,fdt-list = "of-list";

            images {
                kernel@1 {
                    description = "SPL";
                    os = "u-boot";
                    type = "rkspi";
                    arch = "arm";
                    compression = "none";
                    load = <0>;
                    entry = <0>;

                    u-boot-spl {
                    };
                };
            };
        };
    };

U-Boot supports creating fdt and config nodes automatically. To do this,
pass an of-list property (e.g. -a of-list=file1 file2). This tells binman
that you want to generates nodes for two files: file1.dtb and file2.dtb
The fit,fdt-list property (see above) indicates that of-list should be used.
If the property is missing you will get an error.

Then add a 'generator node', a node with a name starting with '@'::

    images {
        @fdt-SEQ {
            description = "fdt-NAME";
            type = "flat_dt";
            compression = "none";
        };
    };

This tells binman to create nodes fdt-1 and fdt-2 for each of your two
files. All the properties you specify will be included in the node. This
node acts like a template to generate the nodes. The generator node itself
does not appear in the output - it is replaced with what binman generates.

You can create config nodes in a similar way::

    configurations {
        default = "@config-DEFAULT-SEQ";
        @config-SEQ {
            description = "NAME";
            firmware = "atf";
            loadables = "uboot";
            fdt = "fdt-SEQ";
        };
    };

This tells binman to create nodes config-1 and config-2, i.e. a config for
each of your two files.

Available substitutions for '@' nodes are:

SEQ:
    Sequence number of the generated fdt (1, 2, ...)
NAME
    Name of the dtb as provided (i.e. without adding '.dtb')

Note that if no devicetree files are provided (with '-a of-list' as above)
then no nodes will be generated.

The 'default' property, if present, will be automatically set to the name
if of configuration whose devicetree matches the 'default-dt' entry
argument, e.g. with '-a default-dt=sun50i-a64-pine64-lts'.

Available substitutions for '@' property values are

DEFAULT-SEQ:
    Sequence number of the default fdt,as provided by the 'default-dt' entry
    argument

Properties (in the 'fit' node itself):
    fit,external-offset: Indicates that the contents of the FIT are external
        and provides the external offset. This is passsed to mkimage via
        the -E and -p flags.




Entry: fmap: An entry which contains an Fmap section
----------------------------------------------------

Properties / Entry arguments:
    None

FMAP is a simple format used by flashrom, an open-source utility for
reading and writing the SPI flash, typically on x86 CPUs. The format
provides flashrom with a list of areas, so it knows what it in the flash.
It can then read or write just a single area, instead of the whole flash.

The format is defined by the flashrom project, in the file lib/fmap.h -
see www.flashrom.org/Flashrom for more information.

When used, this entry will be populated with an FMAP which reflects the
entries in the current image. Note that any hierarchy is squashed, since
FMAP does not support this. Also, CBFS entries appear as a single entry -
the sub-entries are ignored.



Entry: gbb: An entry which contains a Chromium OS Google Binary Block
---------------------------------------------------------------------

Properties / Entry arguments:
    - hardware-id: Hardware ID to use for this build (a string)
    - keydir: Directory containing the public keys to use
    - bmpblk: Filename containing images used by recovery

Chromium OS uses a GBB to store various pieces of information, in particular
the root and recovery keys that are used to verify the boot process. Some
more details are here:

    https://www.chromium.org/chromium-os/firmware-porting-guide/2-concepts

but note that the page dates from 2013 so is quite out of date. See
README.chromium for how to obtain the required keys and tools.



Entry: image-header: An entry which contains a pointer to the FDT map
---------------------------------------------------------------------

Properties / Entry arguments:
    location: Location of header ("start" or "end" of image). This is
        optional. If omitted then the entry must have an offset property.

This adds an 8-byte entry to the start or end of the image, pointing to the
location of the FDT map. The format is a magic number followed by an offset
from the start or end of the image, in twos-compliment format.

This entry must be in the top-level part of the image.

NOTE: If the location is at the start/end, you will probably need to specify
sort-by-offset for the image, unless you actually put the image header
first/last in the entry list.



Entry: intel-cmc: Entry containing an Intel Chipset Micro Code (CMC) file
-------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains microcode for some devices in a special format. An
example filename is 'Microcode/C0_22211.BIN'.

See README.x86 for information about x86 binary blobs.



Entry: intel-descriptor: Intel flash descriptor block (4KB)
-----------------------------------------------------------

Properties / Entry arguments:
    filename: Filename of file containing the descriptor. This is typically
        a 4KB binary file, sometimes called 'descriptor.bin'

This entry is placed at the start of flash and provides information about
the SPI flash regions. In particular it provides the base address and
size of the ME (Management Engine) region, allowing us to place the ME
binary in the right place.

With this entry in your image, the position of the 'intel-me' entry will be
fixed in the image, which avoids you needed to specify an offset for that
region. This is useful, because it is not possible to change the position
of the ME region without updating the descriptor.

See README.x86 for information about x86 binary blobs.



Entry: intel-fit: Intel Firmware Image Table (FIT)
--------------------------------------------------

This entry contains a dummy FIT as required by recent Intel CPUs. The FIT
contains information about the firmware and microcode available in the
image.

At present binman only supports a basic FIT with no microcode.



Entry: intel-fit-ptr: Intel Firmware Image Table (FIT) pointer
--------------------------------------------------------------

This entry contains a pointer to the FIT. It is required to be at address
0xffffffc0 in the image.



Entry: intel-fsp: Entry containing an Intel Firmware Support Package (FSP) file
-------------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains binary blobs which are used on some devices to make the
platform work. U-Boot executes this code since it is not possible to set up
the hardware using U-Boot open-source code. Documentation is typically not
available in sufficient detail to allow this.

An example filename is 'FSP/QUEENSBAY_FSP_GOLD_001_20-DECEMBER-2013.fd'

See README.x86 for information about x86 binary blobs.



Entry: intel-fsp-m: Entry containing Intel Firmware Support Package (FSP) memory init
-------------------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains a binary blob which is used on some devices to set up
SDRAM. U-Boot executes this code in SPL so that it can make full use of
memory. Documentation is typically not available in sufficient detail to
allow U-Boot do this this itself..

An example filename is 'fsp_m.bin'

See README.x86 for information about x86 binary blobs.



Entry: intel-fsp-s: Entry containing Intel Firmware Support Package (FSP) silicon init
--------------------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains a binary blob which is used on some devices to set up
the silicon. U-Boot executes this code in U-Boot proper after SDRAM is
running, so that it can make full use of memory. Documentation is typically
not available in sufficient detail to allow U-Boot do this this itself.

An example filename is 'fsp_s.bin'

See README.x86 for information about x86 binary blobs.



Entry: intel-fsp-t: Entry containing Intel Firmware Support Package (FSP) temp ram init
---------------------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains a binary blob which is used on some devices to set up
temporary memory (Cache-as-RAM or CAR). U-Boot executes this code in TPL so
that it has access to memory for its stack and initial storage.

An example filename is 'fsp_t.bin'

See README.x86 for information about x86 binary blobs.



Entry: intel-ifwi: Entry containing an Intel Integrated Firmware Image (IFWI) file
----------------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry. This is either the
        IFWI file itself, or a file that can be converted into one using a
        tool
    - convert-fit: If present this indicates that the ifwitool should be
        used to convert the provided file into a IFWI.

This file contains code and data used by the SoC that is required to make
it work. It includes U-Boot TPL, microcode, things related to the CSE
(Converged Security Engine, the microcontroller that loads all the firmware)
and other items beyond the wit of man.

A typical filename is 'ifwi.bin' for an IFWI file, or 'fitimage.bin' for a
file that will be converted to an IFWI.

The position of this entry is generally set by the intel-descriptor entry.

The contents of the IFWI are specified by the subnodes of the IFWI node.
Each subnode describes an entry which is placed into the IFWFI with a given
sub-partition (and optional entry name).

Properties for subnodes:
    - ifwi-subpart: sub-parition to put this entry into, e.g. "IBBP"
    - ifwi-entry: entry name t use, e.g. "IBBL"
    - ifwi-replace: if present, indicates that the item should be replaced
      in the IFWI. Otherwise it is added.

See README.x86 for information about x86 binary blobs.



Entry: intel-me: Entry containing an Intel Management Engine (ME) file
----------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code used by the SoC that is required to make it work.
The Management Engine is like a background task that runs things that are
not clearly documented, but may include keyboard, display and network
access. For platform that use ME it is not possible to disable it. U-Boot
does not directly execute code in the ME binary.

A typical filename is 'me.bin'.

The position of this entry is generally set by the intel-descriptor entry.

See README.x86 for information about x86 binary blobs.



Entry: intel-mrc: Entry containing an Intel Memory Reference Code (MRC) file
----------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code for setting up the SDRAM on some Intel systems. This
is executed by U-Boot when needed early during startup. A typical filename
is 'mrc.bin'.

See README.x86 for information about x86 binary blobs.



Entry: intel-refcode: Entry containing an Intel Reference Code file
-------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code for setting up the platform on some Intel systems.
This is executed by U-Boot when needed early during startup. A typical
filename is 'refcode.bin'.

See README.x86 for information about x86 binary blobs.



Entry: intel-vbt: Entry containing an Intel Video BIOS Table (VBT) file
-----------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code that sets up the integrated graphics subsystem on
some Intel SoCs. U-Boot executes this when the display is started up.

See README.x86 for information about Intel binary blobs.



Entry: intel-vga: Entry containing an Intel Video Graphics Adaptor (VGA) file
-----------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of file to read into entry

This file contains code that sets up the integrated graphics subsystem on
some Intel SoCs. U-Boot executes this when the display is started up.

This is similar to the VBT file but in a different format.

See README.x86 for information about Intel binary blobs.



Entry: mkimage: Entry containing a binary produced by mkimage
-------------------------------------------------------------

Properties / Entry arguments:
    - datafile: Filename for -d argument
    - args: Other arguments to pass

The data passed to mkimage is collected from subnodes of the mkimage node,
e.g.::

    mkimage {
        args = "-n test -T imximage";

        u-boot-spl {
        };
    };

This calls mkimage to create an imximage with u-boot-spl.bin as the input
file. The output from mkimage then becomes part of the image produced by
binman.



Entry: powerpc-mpc85xx-bootpg-resetvec: PowerPC mpc85xx bootpg + resetvec code for U-Boot
-----------------------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-br.bin (default 'u-boot-br.bin')

This entry is valid for PowerPC mpc85xx cpus. This entry holds
'bootpg + resetvec' code for PowerPC mpc85xx CPUs which needs to be
placed at offset 'RESET_VECTOR_ADDRESS - 0xffc'.



Entry: scp: Entry containing a System Control Processor (SCP) firmware blob
---------------------------------------------------------------------------

Properties / Entry arguments:
    - scp-path: Filename of file to read into the entry, typically scp.bin

This entry holds firmware for an external platform-specific coprocessor.



Entry: section: Entry that contains other entries
-------------------------------------------------

Properties / Entry arguments: (see binman README for more information):
    pad-byte: Pad byte to use when padding
    sort-by-offset: True if entries should be sorted by offset, False if
    they must be in-order in the device tree description

    end-at-4gb: Used to build an x86 ROM which ends at 4GB (2^32)

    skip-at-start: Number of bytes before the first entry starts. These
        effectively adjust the starting offset of entries. For example,
        if this is 16, then the first entry would start at 16. An entry
        with offset = 20 would in fact be written at offset 4 in the image
        file, since the first 16 bytes are skipped when writing.
    name-prefix: Adds a prefix to the name of every entry in the section
        when writing out the map

Properties:
    allow_missing: True if this section permits external blobs to be
        missing their contents. The second will produce an image but of
        course it will not work.

Since a section is also an entry, it inherits all the properies of entries
too.

A section is an entry which can contain other entries, thus allowing
hierarchical images to be created. See 'Sections and hierarchical images'
in the binman README for more information.



Entry: text: An entry which contains text
-----------------------------------------

The text can be provided either in the node itself or by a command-line
argument. There is a level of indirection to allow multiple text strings
and sharing of text.

Properties / Entry arguments:
    text-label: The value of this string indicates the property / entry-arg
        that contains the string to place in the entry
    <xxx> (actual name is the value of text-label): contains the string to
        place in the entry.
    <text>: The text to place in the entry (overrides the above mechanism).
        This is useful when the text is constant.

Example node::

    text {
        size = <50>;
        text-label = "message";
    };

You can then use:

    binman -amessage="this is my message"

and binman will insert that string into the entry.

It is also possible to put the string directly in the node::

    text {
        size = <8>;
        text-label = "message";
        message = "a message directly in the node"
    };

or just::

    text {
        size = <8>;
        text = "some text directly in the node"
    };

The text is not itself nul-terminated. This can be achieved, if required,
by setting the size of the entry to something larger than the text.



Entry: u-boot: U-Boot flat binary
---------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.bin (default 'u-boot.bin')

This is the U-Boot binary, containing relocation information to allow it
to relocate itself at runtime. The binary typically includes a device tree
blob at the end of it.

U-Boot can access binman symbols at runtime. See:

    'Access to binman entry offsets at run time (fdt)'

in the binman README for more information.

Note that this entry is automatically replaced with u-boot-expanded unless
--no-expanded is used.



Entry: u-boot-dtb: U-Boot device tree
-------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.dtb (default 'u-boot.dtb')

This is the U-Boot device tree, containing configuration information for
U-Boot. U-Boot needs this to know what devices are present and which drivers
to activate.

Note: This is mostly an internal entry type, used by others. This allows
binman to know which entries contain a device tree.



Entry: u-boot-dtb-with-ucode: A U-Boot device tree file, with the microcode removed
-----------------------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.dtb (default 'u-boot.dtb')

See Entry_u_boot_ucode for full details of the three entries involved in
this process. This entry provides the U-Boot device-tree file, which
contains the microcode. If the microcode is not being collated into one
place then the offset and size of the microcode is recorded by this entry,
for use by u-boot-with-ucode_ptr. If it is being collated, then this
entry deletes the microcode from the device tree (to save space) and makes
it available to u-boot-ucode.



Entry: u-boot-elf: U-Boot ELF image
-----------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot (default 'u-boot')

This is the U-Boot ELF image. It does not include a device tree but can be
relocated to any address for execution.



Entry: u-boot-env: An entry which contains a U-Boot environment
---------------------------------------------------------------

Properties / Entry arguments:
    - filename: File containing the environment text, with each line in the
        form var=value



Entry: u-boot-expanded: U-Boot flat binary broken out into its component parts
------------------------------------------------------------------------------

This is a section containing the U-Boot binary and a devicetree. Using this
entry type automatically creates this section, with the following entries
in it:

   u-boot-nodtb
   u-boot-dtb

Having the devicetree separate allows binman to update it in the final
image, so that the entries positions are provided to the running U-Boot.



Entry: u-boot-img: U-Boot legacy image
--------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.img (default 'u-boot.img')

This is the U-Boot binary as a packaged image, in legacy format. It has a
header which allows it to be loaded at the correct address for execution.

You should use FIT (Flat Image Tree) instead of the legacy image for new
applications.



Entry: u-boot-nodtb: U-Boot flat binary without device tree appended
--------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename to include (default 'u-boot-nodtb.bin')

This is the U-Boot binary, containing relocation information to allow it
to relocate itself at runtime. It does not include a device tree blob at
the end of it so normally cannot work without it. You can add a u-boot-dtb
entry after this one, or use a u-boot entry instead, normally expands to a
section containing u-boot and u-boot-dtb



Entry: u-boot-spl: U-Boot SPL binary
------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-spl.bin (default 'spl/u-boot-spl.bin')

This is the U-Boot SPL (Secondary Program Loader) binary. This is a small
binary which loads before U-Boot proper, typically into on-chip SRAM. It is
responsible for locating, loading and jumping to U-Boot. Note that SPL is
not relocatable so must be loaded to the correct address in SRAM, or written
to run from the correct address if direct flash execution is possible (e.g.
on x86 devices).

SPL can access binman symbols at runtime. See:

    'Access to binman entry offsets at run time (symbols)'

in the binman README for more information.

The ELF file 'spl/u-boot-spl' must also be available for this to work, since
binman uses that to look up symbols to write into the SPL binary.

Note that this entry is automatically replaced with u-boot-spl-expanded
unless --no-expanded is used.



Entry: u-boot-spl-bss-pad: U-Boot SPL binary padded with a BSS region
---------------------------------------------------------------------

Properties / Entry arguments:
    None

This holds the padding added after the SPL binary to cover the BSS (Block
Started by Symbol) region. This region holds the various variables used by
SPL. It is set to 0 by SPL when it starts up. If you want to append data to
the SPL image (such as a device tree file), you must pad out the BSS region
to avoid the data overlapping with U-Boot variables. This entry is useful in
that case. It automatically pads out the entry size to cover both the code,
data and BSS.

The contents of this entry will a certain number of zero bytes, determined
by __bss_size

The ELF file 'spl/u-boot-spl' must also be available for this to work, since
binman uses that to look up the BSS address.



Entry: u-boot-spl-dtb: U-Boot SPL device tree
---------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.dtb (default 'spl/u-boot-spl.dtb')

This is the SPL device tree, containing configuration information for
SPL. SPL needs this to know what devices are present and which drivers
to activate.



Entry: u-boot-spl-elf: U-Boot SPL ELF image
-------------------------------------------

Properties / Entry arguments:
    - filename: Filename of SPL u-boot (default 'spl/u-boot-spl')

This is the U-Boot SPL ELF image. It does not include a device tree but can
be relocated to any address for execution.



Entry: u-boot-spl-expanded: U-Boot SPL flat binary broken out into its component parts
--------------------------------------------------------------------------------------

Properties / Entry arguments:
    - spl-dtb: Controls whether this entry is selected (set to 'y' or '1' to
        select)

This is a section containing the U-Boot binary, BSS padding if needed and a
devicetree. Using this entry type automatically creates this section, with
the following entries in it:

   u-boot-spl-nodtb
   u-boot-spl-bss-pad
   u-boot-dtb

Having the devicetree separate allows binman to update it in the final
image, so that the entries positions are provided to the running U-Boot.

This entry is selected based on the value of the 'spl-dtb' entryarg. If
this is non-empty (and not 'n' or '0') then this expanded entry is selected.



Entry: u-boot-spl-nodtb: SPL binary without device tree appended
----------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename to include (default 'spl/u-boot-spl-nodtb.bin')

This is the U-Boot SPL binary, It does not include a device tree blob at
the end of it so may not be able to work without it, assuming SPL needs
a device tree to operate on your platform. You can add a u-boot-spl-dtb
entry after this one, or use a u-boot-spl entry instead' which normally
expands to a section containing u-boot-spl-dtb, u-boot-spl-bss-pad and
u-boot-spl-dtb

SPL can access binman symbols at runtime. See:

    'Access to binman entry offsets at run time (symbols)'

in the binman README for more information.

The ELF file 'spl/u-boot-spl' must also be available for this to work, since
binman uses that to look up symbols to write into the SPL binary.



Entry: u-boot-spl-with-ucode-ptr: U-Boot SPL with embedded microcode pointer
----------------------------------------------------------------------------

This is used when SPL must set up the microcode for U-Boot.

See Entry_u_boot_ucode for full details of the entries involved in this
process.



Entry: u-boot-tpl: U-Boot TPL binary
------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-tpl.bin (default 'tpl/u-boot-tpl.bin')

This is the U-Boot TPL (Tertiary Program Loader) binary. This is a small
binary which loads before SPL, typically into on-chip SRAM. It is
responsible for locating, loading and jumping to SPL, the next-stage
loader. Note that SPL is not relocatable so must be loaded to the correct
address in SRAM, or written to run from the correct address if direct
flash execution is possible (e.g. on x86 devices).

SPL can access binman symbols at runtime. See:

    'Access to binman entry offsets at run time (symbols)'

in the binman README for more information.

The ELF file 'tpl/u-boot-tpl' must also be available for this to work, since
binman uses that to look up symbols to write into the TPL binary.

Note that this entry is automatically replaced with u-boot-tpl-expanded
unless --no-expanded is used.



Entry: u-boot-tpl-bss-pad: U-Boot TPL binary padded with a BSS region
---------------------------------------------------------------------

Properties / Entry arguments:
    None

This holds the padding added after the TPL binary to cover the BSS (Block
Started by Symbol) region. This region holds the various variables used by
TPL. It is set to 0 by TPL when it starts up. If you want to append data to
the TPL image (such as a device tree file), you must pad out the BSS region
to avoid the data overlapping with U-Boot variables. This entry is useful in
that case. It automatically pads out the entry size to cover both the code,
data and BSS.

The contents of this entry will a certain number of zero bytes, determined
by __bss_size

The ELF file 'tpl/u-boot-tpl' must also be available for this to work, since
binman uses that to look up the BSS address.



Entry: u-boot-tpl-dtb: U-Boot TPL device tree
---------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot.dtb (default 'tpl/u-boot-tpl.dtb')

This is the TPL device tree, containing configuration information for
TPL. TPL needs this to know what devices are present and which drivers
to activate.



Entry: u-boot-tpl-dtb-with-ucode: U-Boot TPL with embedded microcode pointer
----------------------------------------------------------------------------

This is used when TPL must set up the microcode for U-Boot.

See Entry_u_boot_ucode for full details of the entries involved in this
process.



Entry: u-boot-tpl-elf: U-Boot TPL ELF image
-------------------------------------------

Properties / Entry arguments:
    - filename: Filename of TPL u-boot (default 'tpl/u-boot-tpl')

This is the U-Boot TPL ELF image. It does not include a device tree but can
be relocated to any address for execution.



Entry: u-boot-tpl-expanded: U-Boot TPL flat binary broken out into its component parts
--------------------------------------------------------------------------------------

Properties / Entry arguments:
    - tpl-dtb: Controls whether this entry is selected (set to 'y' or '1' to
        select)

This is a section containing the U-Boot binary, BSS padding if needed and a
devicetree. Using this entry type automatically creates this section, with
the following entries in it:

   u-boot-tpl-nodtb
   u-boot-tpl-bss-pad
   u-boot-dtb

Having the devicetree separate allows binman to update it in the final
image, so that the entries positions are provided to the running U-Boot.

This entry is selected based on the value of the 'tpl-dtb' entryarg. If
this is non-empty (and not 'n' or '0') then this expanded entry is selected.



Entry: u-boot-tpl-nodtb: TPL binary without device tree appended
----------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename to include (default 'tpl/u-boot-tpl-nodtb.bin')

This is the U-Boot TPL binary, It does not include a device tree blob at
the end of it so may not be able to work without it, assuming TPL needs
a device tree to operate on your platform. You can add a u-boot-tpl-dtb
entry after this one, or use a u-boot-tpl entry instead, which normally
expands to a section containing u-boot-tpl-dtb, u-boot-tpl-bss-pad and
u-boot-tpl-dtb

TPL can access binman symbols at runtime. See:

    'Access to binman entry offsets at run time (symbols)'

in the binman README for more information.

The ELF file 'tpl/u-boot-tpl' must also be available for this to work, since
binman uses that to look up symbols to write into the TPL binary.



Entry: u-boot-tpl-with-ucode-ptr: U-Boot TPL with embedded microcode pointer
----------------------------------------------------------------------------

See Entry_u_boot_ucode for full details of the entries involved in this
process.



Entry: u-boot-ucode: U-Boot microcode block
-------------------------------------------

Properties / Entry arguments:
    None

The contents of this entry are filled in automatically by other entries
which must also be in the image.

U-Boot on x86 needs a single block of microcode. This is collected from
the various microcode update nodes in the device tree. It is also unable
to read the microcode from the device tree on platforms that use FSP
(Firmware Support Package) binaries, because the API requires that the
microcode is supplied before there is any SRAM available to use (i.e.
the FSP sets up the SRAM / cache-as-RAM but does so in the call that
requires the microcode!). To keep things simple, all x86 platforms handle
microcode the same way in U-Boot (even non-FSP platforms). This is that
a table is placed at _dt_ucode_base_size containing the base address and
size of the microcode. This is either passed to the FSP (for FSP
platforms), or used to set up the microcode (for non-FSP platforms).
This all happens in the build system since it is the only way to get
the microcode into a single blob and accessible without SRAM.

There are two cases to handle. If there is only one microcode blob in
the device tree, then the ucode pointer it set to point to that. This
entry (u-boot-ucode) is empty. If there is more than one update, then
this entry holds the concatenation of all updates, and the device tree
entry (u-boot-dtb-with-ucode) is updated to remove the microcode. This
last step ensures that that the microcode appears in one contiguous
block in the image and is not unnecessarily duplicated in the device
tree. It is referred to as 'collation' here.

Entry types that have a part to play in handling microcode:

    Entry_u_boot_with_ucode_ptr:
        Contains u-boot-nodtb.bin (i.e. U-Boot without the device tree).
        It updates it with the address and size of the microcode so that
        U-Boot can find it early on start-up.
    Entry_u_boot_dtb_with_ucode:
        Contains u-boot.dtb. It stores the microcode in a
        'self.ucode_data' property, which is then read by this class to
        obtain the microcode if needed. If collation is performed, it
        removes the microcode from the device tree.
    Entry_u_boot_ucode:
        This class. If collation is enabled it reads the microcode from
        the Entry_u_boot_dtb_with_ucode entry, and uses it as the
        contents of this entry.



Entry: u-boot-with-ucode-ptr: U-Boot with embedded microcode pointer
--------------------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-nodtb.bin (default 'u-boot-nodtb.bin')
    - optional-ucode: boolean property to make microcode optional. If the
        u-boot.bin image does not include microcode, no error will
        be generated.

See Entry_u_boot_ucode for full details of the three entries involved in
this process. This entry updates U-Boot with the offset and size of the
microcode, to allow early x86 boot code to find it without doing anything
complicated. Otherwise it is the same as the u-boot entry.



Entry: vblock: An entry which contains a Chromium OS verified boot block
------------------------------------------------------------------------

Properties / Entry arguments:
    - content: List of phandles to entries to sign
    - keydir: Directory containing the public keys to use
    - keyblock: Name of the key file to use (inside keydir)
    - signprivate: Name of provide key file to use (inside keydir)
    - version: Version number of the vblock (typically 1)
    - kernelkey: Name of the kernel key to use (inside keydir)
    - preamble-flags: Value of the vboot preamble flags (typically 0)

Output files:
    - input.<unique_name> - input file passed to futility
    - vblock.<unique_name> - output file generated by futility (which is
        used as the entry contents)

Chromium OS signs the read-write firmware and kernel, writing the signature
in this block. This allows U-Boot to verify that the next firmware stage
and kernel are genuine.



Entry: x86-reset16: x86 16-bit reset code for U-Boot
----------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-x86-reset16.bin (default
        'u-boot-x86-reset16.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed at a particular address. This entry holds that code. It is
typically placed at offset CONFIG_RESET_VEC_LOC. The code is responsible
for jumping to the x86-start16 code, which continues execution.

For 64-bit U-Boot, the 'x86_reset16_spl' entry type is used instead.



Entry: x86-reset16-spl: x86 16-bit reset code for U-Boot
--------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-x86-reset16.bin (default
        'u-boot-x86-reset16.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed at a particular address. This entry holds that code. It is
typically placed at offset CONFIG_RESET_VEC_LOC. The code is responsible
for jumping to the x86-start16 code, which continues execution.

For 32-bit U-Boot, the 'x86_reset_spl' entry type is used instead.



Entry: x86-reset16-tpl: x86 16-bit reset code for U-Boot
--------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-x86-reset16.bin (default
        'u-boot-x86-reset16.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed at a particular address. This entry holds that code. It is
typically placed at offset CONFIG_RESET_VEC_LOC. The code is responsible
for jumping to the x86-start16 code, which continues execution.

For 32-bit U-Boot, the 'x86_reset_tpl' entry type is used instead.



Entry: x86-start16: x86 16-bit start-up code for U-Boot
-------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of u-boot-x86-start16.bin (default
        'u-boot-x86-start16.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed in the top 64KB of the ROM. The reset code jumps to it. This
entry holds that code. It is typically placed at offset
CONFIG_SYS_X86_START16. The code is responsible for changing to 32-bit mode
and jumping to U-Boot's entry point, which requires 32-bit mode (for 32-bit
U-Boot).

For 64-bit U-Boot, the 'x86_start16_spl' entry type is used instead.



Entry: x86-start16-spl: x86 16-bit start-up code for SPL
--------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of spl/u-boot-x86-start16-spl.bin (default
        'spl/u-boot-x86-start16-spl.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed in the top 64KB of the ROM. The reset code jumps to it. This
entry holds that code. It is typically placed at offset
CONFIG_SYS_X86_START16. The code is responsible for changing to 32-bit mode
and jumping to U-Boot's entry point, which requires 32-bit mode (for 32-bit
U-Boot).

For 32-bit U-Boot, the 'x86-start16' entry type is used instead.



Entry: x86-start16-tpl: x86 16-bit start-up code for TPL
--------------------------------------------------------

Properties / Entry arguments:
    - filename: Filename of tpl/u-boot-x86-start16-tpl.bin (default
        'tpl/u-boot-x86-start16-tpl.bin')

x86 CPUs start up in 16-bit mode, even if they are 32-bit CPUs. This code
must be placed in the top 64KB of the ROM. The reset code jumps to it. This
entry holds that code. It is typically placed at offset
CONFIG_SYS_X86_START16. The code is responsible for changing to 32-bit mode
and jumping to U-Boot's entry point, which requires 32-bit mode (for 32-bit
U-Boot).

If TPL is not being used, the 'x86-start16-spl or 'x86-start16' entry types
may be used instead.



