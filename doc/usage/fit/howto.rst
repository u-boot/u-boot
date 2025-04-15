.. SPDX-License-Identifier: GPL-2.0+

How to use images in the new image format
=========================================

Overview
--------

The new uImage format allows more flexibility in handling images of various
types (kernel, ramdisk, etc.), it also enhances integrity protection of images
with cryptographic checksums.

Two auxiliary tools are needed on the development host system in order to
create an uImage in the new format: mkimage and dtc, although only one
(mkimage) is invoked directly. dtc is called from within mkimage and operates
behind the scenes, but needs to be present in the $PATH nevertheless. It is
important that the dtc used has support for binary includes -- refer to::

    git://git.kernel.org/pub/scm/utils/dtc/dtc.git

for its latest version. mkimage (together with dtc) takes as input
an image source file, which describes the contents of the image and defines
its various properties used during booting. By convention, image source file
has the ".its" extension, also, the details of its format are provided in
:doc:`source_file_format`. The actual data that is to be included in
the uImage (kernel, ramdisk, etc.) is specified in the image source file in the
form of paths to appropriate data files. The outcome of the image creation
process is a binary file (by convention with the ".itb" extension) that
contains all the referenced data (kernel, ramdisk, etc.) and other information
needed by U-Boot to handle the uImage properly. The uImage file is then
transferred to the target (e.g., via tftp) and booted using the bootm command.

To summarize the prerequisites needed for new uImage creation:

- mkimage
- dtc (with support for binary includes)
- image source file (`*.its`)
- image data file(s)


Here's a graphical overview of the image creation and booting process::

    image source file     mkimage + dtc          transfer to target
        +          ---------------> image file --------------------> bootm
    image data file(s)

SPL usage
---------

The SPL can make use of the new image format as well, this traditionally
is used to ship multiple device tree files within one image. Code in the SPL
will choose the one matching the current board and append this to the
U-Boot proper binary to be automatically used up by it.
Aside from U-Boot proper and one device tree blob the SPL can load multiple,
arbitrary image files as well. These binaries should be specified in their
own subnode under the /images node, which should then be referenced from one or
multiple /configurations subnodes. The required images must be enumerated in
the "loadables" property as a list of strings.

The SPL also records to a DT all additional images (called loadables) which are
loaded. The information about loadables locations is passed via the DT node with
fit-images name.

Finally, if there are multiple xPL phases (e.g. SPL, VPL), images can be marked
as intended for a particular phase using the 'phase' property. For example, if
fit_image_load() is called with image_ph(IH_PHASE_SPL, IH_TYPE_FIRMWARE), then
only the image listed into the "firmware" property where phase is set to "spl"
will be loaded.

Loadables Example
-----------------
Consider the following case for an ARM64 platform where U-Boot runs in EL2
started by ATF where SPL is loading U-Boot (as loadables) and ATF (as firmware).

::

    /dts-v1/;

    / {
        description = "Configuration to load ATF before U-Boot";

        images {
            uboot {
                description = "U-Boot (64-bit)";
                data = /incbin/("u-boot-nodtb.bin");
                type = "firmware";
                os = "u-boot";
                arch = "arm64";
                compression = "none";
                load = <0x8 0x8000000>;
                entry = <0x8 0x8000000>;
                hash {
                    algo = "sha256";
                };
            };
            atf {
                description = "ARM Trusted Firmware";
                data = /incbin/("bl31.bin");
                type = "firmware";
                os = "arm-trusted-firmware";
                arch = "arm64";
                compression = "none";
                load = <0xfffea000>;
                entry = <0xfffea000>;
                hash {
                    algo = "sha256";
                };
            };
            fdt_1 {
                description = "zynqmp-zcu102-revA";
                data = /incbin/("arch/arm/dts/zynqmp-zcu102-revA.dtb");
                type = "flat_dt";
                arch = "arm64";
                compression = "none";
                load = <0x100000>;
                hash {
                    algo = "sha256";
                };
            };
        };
        configurations {
            default = "config_1";

            config_1 {
                description = "zynqmp-zcu102-revA";
                firmware = "atf";
                loadables = "uboot";
                fdt = "fdt_1";
            };
        };
    };

In this case the SPL records via fit-images DT node the information about
loadables U-Boot image::

    ZynqMP> fdt addr $fdtcontroladdr
    ZynqMP> fdt print /fit-images
    fit-images {
        uboot {
            os = "u-boot";
            type = "firmware";
            size = <0x001017c8>;
            entry = <0x00000008 0x08000000>;
            load = <0x00000008 0x08000000>;
        };
    };

As you can see entry and load properties are 64bit wide to support loading
images above 4GB (in past entry and load properties where just 32bit).


Example 1 -- old-style (non-FDT) kernel booting
-----------------------------------------------

Consider a simple scenario, where a PPC Linux kernel built from sources on the
development host is to be booted old-style (non-FDT) by U-Boot on an embedded
target. Assume that the outcome of the build is vmlinux.bin.gz, a file which
contains a gzip-compressed PPC Linux kernel (the only data file in this case).
The uImage can be produced using the image source file
doc/uImage.FIT/kernel.its (note that kernel.its assumes that vmlinux.bin.gz is
in the current working directory; if desired, an alternative path can be
specified in the kernel.its file). Here's how to create the image and inspect
its contents:

[on the host system]::

    $ mkimage -f kernel.its kernel.itb
    DTC: dts->dtb  on file "kernel.its"
    $
    $ mkimage -l kernel.itb
    FIT description: Simple image with single Linux kernel
    Created:     Tue Mar 11 17:26:15 2008
     Image 0 (kernel)
      Description:    Vanilla Linux kernel
      Type:        Kernel Image
      Compression:    gzip compressed
      Data Size:    943347 Bytes = 921.24 kB = 0.90 MB
      Architecture: PowerPC
      OS:        Linux
      Load Address: 0x00000000
      Entry Point:    0x00000000
      Hash algo:    crc32
      Hash value:    2ae2bb40
      Hash algo:    sha256
      Hash value:    c22f6bb5a3f96942507a37e7d6a9333ebdc7da57971bc4c082113fe082fdc40f
     Default Configuration: 'config-1'
     Configuration 0 (config-1)
      Description:    Boot Linux kernel
      Kernel:    kernel


The resulting image file kernel.itb can be now transferred to the target,
inspected and booted (note that first three U-Boot commands below are shown
for completeness -- they are part of the standard booting procedure and not
specific to the new image format).

[on the target system]::

    => print nfsargs
    nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath}
    => print addip
    addip=setenv bootargs ${bootargs} ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}:${netdev}:off panic=1
    => run nfsargs addip
    => tftp 900000 /path/to/tftp/location/kernel.itb
    Using FEC device
    TFTP from server 192.168.1.1; our IP address is 192.168.160.5
    Filename '/path/to/tftp/location/kernel.itb'.
    Load address: 0x900000
    Loading: #################################################################
    done
    Bytes transferred = 944464 (e6950 hex)
    => iminfo

    ## Checking Image at 00900000 ...
       FIT image found
       FIT description: Simple image with single Linux kernel
       Created:        2008-03-11    16:26:15 UTC
        Image 0 (kernel)
         Description:  Vanilla Linux kernel
         Type:       Kernel Image
         Compression:  gzip compressed
         Data Start:   0x009000e0
         Data Size:    943347 Bytes = 921.2 kB
         Architecture: PowerPC
         OS:       Linux
         Load Address: 0x00000000
         Entry Point:  0x00000000
         Hash algo:    crc32
         Hash value:   2ae2bb40
         Hash algo:    sha256
         Hash value:   c22f6bb5a3f96942507a37e7d6a9333ebdc7da57971bc4c082113fe082fdc40f
        Default Configuration: 'config-1'
        Configuration 0 (config-1)
         Description:  Boot Linux kernel
         Kernel:       kernel

    => bootm
    ## Booting kernel from FIT Image at 00900000 ...
       Using 'config-1' configuration
       Trying 'kernel' kernel subimage
         Description:  Vanilla Linux kernel
         Type:       Kernel Image
         Compression:  gzip compressed
         Data Start:   0x009000e0
         Data Size:    943347 Bytes = 921.2 kB
         Architecture: PowerPC
         OS:       Linux
         Load Address: 0x00000000
         Entry Point:  0x00000000
         Hash algo:    crc32
         Hash value:   2ae2bb40
         Hash algo:    sha256
         Hash value:   c22f6bb5a3f96942507a37e7d6a9333ebdc7da57971bc4c082113fe082fdc40f
       Verifying Hash Integrity ... crc32+ sha1+ OK
       Uncompressing Kernel Image ... OK
    Memory BAT mapping: BAT2=256Mb, BAT3=0Mb, residual: 0Mb
    Linux version 2.4.25 (m8@hekate) (gcc version 4.0.0 (DENX ELDK 4.0 4.0.0)) #2 czw lip 5 17:56:18 CEST 2007
    On node 0 totalpages: 65536
    zone(0): 65536 pages.
    zone(1): 0 pages.
    zone(2): 0 pages.
    Kernel command line: root=/dev/nfs rw nfsroot=192.168.1.1:/opt/eldk-4.1/ppc_6xx ip=192.168.160.5:192.168.1.1::255.255.0.0:lite5200b:eth0:off panic=1
    Calibrating delay loop... 307.20 BogoMIPS


Example 2 -- new-style (FDT) kernel booting
-------------------------------------------

Consider another simple scenario, where a PPC Linux kernel is to be booted
new-style, i.e., with a FDT blob. In this case there are two prerequisite data
files: vmlinux.bin.gz (Linux kernel) and target.dtb (FDT blob). The uImage can
be produced using image source file doc/uImage.FIT/kernel_fdt.its like this
(note again, that both prerequisite data files are assumed to be present in
the current working directory -- image source file kernel_fdt.its can be
modified to take the files from some other location if needed):

[on the host system]::

    $ mkimage -f kernel_fdt.its kernel_fdt.itb
    DTC: dts->dtb  on file "kernel_fdt.its"
    $
    $ mkimage -l kernel_fdt.itb
    FIT description: Simple image with single Linux kernel and FDT blob
    Created:     Tue Mar 11 16:29:22 2008
     Image 0 (kernel)
      Description:    Vanilla Linux kernel
      Type:        Kernel Image
      Compression:    gzip compressed
      Data Size:    1092037 Bytes = 1066.44 kB = 1.04 MB
      Architecture: PowerPC
      OS:        Linux
      Load Address: 0x00000000
      Entry Point:    0x00000000
      Hash algo:    crc32
      Hash value:    2c0cc807
      Hash algo:    sha256
      Hash value:    a3e9e18b793873827d27c97edfbca67c404a1972d9f36cf48e73ff85d69a422c
     Image 1 (fdt-1)
      Description:    Flattened Device Tree blob
      Type:        Flat Device Tree
      Compression:    uncompressed
      Data Size:    16384 Bytes = 16.00 kB = 0.02 MB
      Architecture: PowerPC
      Hash algo:    crc32
      Hash value:    0d655d71
      Hash algo:    sha256
      Hash value:    e9b9a40c5e2e12213ac819e7ccad7271ef43eb5edf9b421f0fa0b4b51bfdb214
     Default Configuration: 'conf-1'
     Configuration 0 (conf-1)
      Description:    Boot Linux kernel with FDT blob
      Kernel:    kernel
      FDT:        fdt-1


The resulting image file kernel_fdt.itb can be now transferred to the target,
inspected and booted:

[on the target system]::

    => tftp 900000 /path/to/tftp/location/kernel_fdt.itb
    Using FEC device
    TFTP from server 192.168.1.1; our IP address is 192.168.160.5
    Filename '/path/to/tftp/location/kernel_fdt.itb'.
    Load address: 0x900000
    Loading: #################################################################
         ###########
    done
    Bytes transferred = 1109776 (10ef10 hex)
    => iminfo

    ## Checking Image at 00900000 ...
       FIT image found
       FIT description: Simple image with single Linux kernel and FDT blob
       Created:        2008-03-11    15:29:22 UTC
        Image 0 (kernel)
         Description:  Vanilla Linux kernel
         Type:       Kernel Image
         Compression:  gzip compressed
         Data Start:   0x009000ec
         Data Size:    1092037 Bytes =  1 MB
         Architecture: PowerPC
         OS:       Linux
         Load Address: 0x00000000
         Entry Point:  0x00000000
         Hash algo:    crc32
         Hash value:   2c0cc807
         Hash algo:    sha256
         Hash value:   a3e9e18b793873827d27c97edfbca67c404a1972d9f36cf48e73ff85d69a422c
        Image 1 (fdt-1)
         Description:  Flattened Device Tree blob
         Type:       Flat Device Tree
         Compression:  uncompressed
         Data Start:   0x00a0abdc
         Data Size:    16384 Bytes = 16 kB
         Architecture: PowerPC
         Hash algo:    crc32
         Hash value:   0d655d71
         Hash algo:    sha256
         Hash value:   e9b9a40c5e2e12213ac819e7ccad7271ef43eb5edf9b421f0fa0b4b51bfdb214
        Default Configuration: 'conf-1'
        Configuration 0 (conf-1)
         Description:  Boot Linux kernel with FDT blob
         Kernel:       kernel
         FDT:       fdt-1
    => bootm
    ## Booting kernel from FIT Image at 00900000 ...
       Using 'conf-1' configuration
       Trying 'kernel' kernel subimage
         Description:  Vanilla Linux kernel
         Type:       Kernel Image
         Compression:  gzip compressed
         Data Start:   0x009000ec
         Data Size:    1092037 Bytes =  1 MB
         Architecture: PowerPC
         OS:       Linux
         Load Address: 0x00000000
         Entry Point:  0x00000000
         Hash algo:    crc32
         Hash value:   2c0cc807
         Hash algo:    sha1
         Hash value:   a3e9e18b793873827d27c97edfbca67c404a1972d9f36cf48e73ff85d69a422c
       Verifying Hash Integrity ... crc32+ sha1+ OK
       Uncompressing Kernel Image ... OK
    ## Flattened Device Tree from FIT Image at 00900000
       Using 'conf-1' configuration
       Trying 'fdt-1' FDT blob subimage
         Description:  Flattened Device Tree blob
         Type:       Flat Device Tree
         Compression:  uncompressed
         Data Start:   0x00a0abdc
         Data Size:    16384 Bytes = 16 kB
         Architecture: PowerPC
         Hash algo:    crc32
         Hash value:   0d655d71
         Hash algo:    sha1
         Hash value:   e9b9a40c5e2e12213ac819e7ccad7271ef43eb5edf9b421f0fa0b4b51bfdb214
       Verifying Hash Integrity ... crc32+ sha1+ OK
       Booting using the fdt blob at 0xa0abdc
       Loading Device Tree to 007fc000, end 007fffff ... OK
    [    0.000000] Using lite5200 machine description
    [    0.000000] Linux version 2.6.24-rc6-gaebecdfc (m8@hekate) (gcc version 4.0.0 (DENX ELDK 4.1 4.0.0)) #1 Sat Jan 12 15:38:48 CET 2008


Example 3 -- advanced booting
-----------------------------

Refer to :doc:`multi` for an image source file that allows more
sophisticated booting scenarios (multiple kernels, ramdisks and fdt blobs).

.. sectionauthor:: Bartlomiej Sieka <tur@semihalf.com>
