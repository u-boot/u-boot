.. SPDX-License-Identifier: GPL-2.0-or-later
.. _falcon-mode:

Falcon Mode
===========

Introduction
------------

This document provides an overview of how to add support for Falcon Mode
to a board.

Falcon Mode is introduced to speed up the booting process, allowing
to boot a Linux kernel (or whatever image) without a full blown U-Boot.

Falcon Mode relies on the SPL framework. In fact, to make booting faster,
U-Boot is split into two parts: the SPL (Secondary Program Loader) and U-Boot
image. In most implementations, SPL is used to start U-Boot when booting from
a mass storage, such as NAND or SD-Card. SPL has now support for other media,
and can generally be seen as a way to start an image performing the minimum
required initialization. SPL mainly initializes the RAM controller, and then
copies U-Boot image into the memory.

The Falcon Mode extends this way allowing to start the Linux kernel directly
from SPL. A new command is added to U-Boot to prepare the parameters that SPL
must pass to the kernel using a Device Tree.

In normal mode, these parameters are generated each time before
loading the kernel, passing to Linux the address in memory where
the parameters can be read.
With Falcon Mode, this snapshot can be saved into persistent storage and SPL is
informed to load it before running the kernel.

To boot the kernel, these steps under a Falcon-aware U-Boot are required:

1. Boot the board into U-Boot.
    After loading the desired legacy-format kernel image into memory (and DT as
    well, if used), use the "spl export" command to generate the kernel
    parameters area or the DT.  U-Boot runs as when it boots the kernel, but
    stops before passing the control to the kernel.

2. Save the prepared snapshot into persistent media.
    The address where to save it must be configured into board configuration
    file (CONFIG_CMD_SPL_NAND_OFS for NAND).

3. Boot the board into Falcon Mode. SPL will load the kernel and copy
    the parameters which are saved in the persistent area to the required
    address. If a valid uImage is not found at the defined location, U-Boot
    will be booted instead.

It is required to implement a custom mechanism to select if SPL loads U-Boot
or another image.

The value of a GPIO is a simple way to operate the selection, as well as
reading a character from the SPL console if CONFIG_SPL_CONSOLE is set.

Falcon Mode is generally activated by setting CONFIG_SPL_OS_BOOT. This tells
SPL that U-Boot is not the only available image that SPL is able to start.

Configuration
-------------

CONFIG_CMD_SPL
    Enable the "spl export" command.
    The command "spl export" is then available in U-Boot mode.

CONFIG_SPL_PAYLOAD_ARGS_ADDR
    Address in RAM where the parameters must be copied by SPL.
    In most cases, it is <start_of_ram> + 0x100.

CONFIG_SYS_NAND_SPL_KERNEL_OFFS
    Offset in NAND where the kernel is stored

CONFIG_CMD_SPL_NAND_OFS
    Offset in NAND where the parameters area was saved.

CONFIG_CMD_SPL_NOR_OFS
    Offset in NOR where the parameters area was saved.

CONFIG_CMD_SPL_WRITE_SIZE
    Size of the parameters area to be copied

CONFIG_SPL_OS_BOOT
    Activate Falcon Mode.

CONFIG_SPL_OS_BOOT_ARGS
    Allow SPL to load args file for the kernel in Falcon Mode. This option can
    be disabled if the device-tree is packaged directly in the FIT payload.

CONFIG_SPL_OS_BOOT_SECURE
    Enable secure boot for Falcon Mode, which provides an additional layer of
    security by authenticating the boot process using a signed FIT image.

Function that a board must implement
------------------------------------

void spl_board_prepare_for_linux(void)
    optional, called from SPL before starting the kernel

spl_start_uboot()
    required, returns "0" if SPL should start the kernel, "1" if U-Boot
    must be started.

Environment variables
---------------------

A board may chose to look at the environment for decisions about falcon
mode.  In this case the following variables may be supported:

boot_os
    Set to yes/Yes/true/True/1 to enable booting to OS,
    any other value to fall back to U-Boot (including unset)

falcon_args_file
    Filename to load as the 'args' portion of falcon mode rather than the
    hard-coded value.

falcon_image_file
    Filename to load as the OS image portion of falcon mode rather than the
    hard-coded value.

Using spl command
-----------------

spl - SPL configuration

Usage::

    spl export fdt [kernel_addr] [initrd_addr] [fdt_addr ]

kernel_addr
    kernel is loaded as part of the boot process, but it is not started.
    This is the address where a kernel image is stored.

initrd_addr
    Address of initial ramdisk
    can be set to "-" if fdt_addr without initrd_addr is used

fdt_addr
    in case of fdt, the address of the device tree.

The *spl export* command does not write to a storage media. The user is
responsible to transfer the gathered information (prepared FDT) from temporary
storage in RAM into persistent storage after each run of *spl export*.
Unfortunately the position of temporary storage can not be predicted nor
provided at command line, it depends highly on your system setup and your
provided device tree.
However at the end of an successful *spl export* run it will print the
RAM address of temporary storage. The RAM address of FDT will also be
set in the environment variable *fdtargsaddr*, the new length of the
prepared FDT will be set in the environment variable *fdtargslen*.
These environment variables can be used in scripts for writing updated
FDT to persistent storage.

Now the user have to save the generated BLOB from that printed address
to the pre-defined address in persistent storage
(CONFIG_CMD_SPL_NAND_OFS in case of NAND).
The following example shows how to prepare the data for Falcon Mode on
twister board with ATAGS BLOB.

Example with FDT: a3m071 board
------------------------------

To boot the Linux kernel from the SPL, the DT blob (fdt) needs to get
prepared/patched first. U-Boot usually inserts some dynamic values into
the DT binary (blob), e.g. autodetected memory size, MAC addresses,
clocks speeds etc. To generate this patched DT blob, you can use
the following command:

1. Load fdt blob to SDRAM::

        => tftp 1800000 a3m071/a3m071.dtb

2. Set bootargs as desired for Linux booting (e.g. flash_mtd)::

        => run mtdargs addip2 addtty

3. Use "fdt" commands to patch the DT blob::

        => fdt addr 1800000
        => fdt boardsetup
        => fdt chosen

4. Display patched DT blob (optional)::

        => fdt print

5. Save fdt to NOR flash::

        => erase fc060000 fc07ffff
        => cp.b 1800000 fc060000 10000
        ...


Falcon Mode was presented at the RMLL 2012. Slides are available at:

http://schedule2012.rmll.info/IMG/pdf/LSM2012_UbootFalconMode_Babic.pdf

Secure Falcon Mode
------------------

Introduction
~~~~~~~~~~~~

Secure Falcon Mode is an enhancement to Falcon Mode that provides additional
security features. It authenticates the boot process using a signed FIT Image
and restricts certain features that are inherently insecure.

Configuration
~~~~~~~~~~~~~

To enable Secure Falcon Mode, the ``CONFIG_SPL_OS_BOOT_SECURE`` option must be
set. This option modifies the behavior of Falcon Mode in the following ways:

1. Fallback Mechanism:
^^^^^^^^^^^^^^^^^^^^^^

Unlike regular Falcon Mode, which falls back to the standard U-Boot boot flow
if kernel booting fails, Secure Falcon Mode disables this fallback mechanism. If
the secure boot process fails, the boot process will not proceed.

2. Signed FIT Image:
^^^^^^^^^^^^^^^^^^^^

Secure Falcon Mode requires a signed FIT, which contains the kernel and
device tree, to boot the system. The ``falcon_args_file`` environment variable
is ignored, and instead, the device tree is read from the signed FIT. This
ensures the authenticity and integrity of the boot process.

Example
~~~~~~~

Secure falcon mode can be enabled on TI AM62x EVM as follows with SD boot mode:

1. Prepare the device-tree:
^^^^^^^^^^^^^^^^^^^^^^^^^^^

To optimize performance, the SPL in Falcon Mode expects the FIT to contain a
device-tree with fixups already applied. Such a device-tree can be generated
using the spl export command as follows:

**Setting bootargs**

Set the bootargs environment variable to the desired value:

.. prompt:: bash =>

        env set bootargs 'console=ttyS2,115200n8 root=/dev/mmcblk1p2 rw rootfstype=ext4 rootwait'

**Read FIT from SD**

Load the FIT image from the SD card:

.. prompt:: bash =>

        load mmc 1:2 0x90000000 /boot/fitImage

**Generate device-tree**

Use the ``spl export`` command to generate a device-tree with fixups applied:

.. prompt:: bash =>

        spl export fdt 0x90000000

**Save the device-tree**

Write the generated device-tree to the SD card:

.. prompt:: bash =>

        fatwrite mmc 1:1 $fdtargsaddr k3-am625-sk-falcon.dtb $fdtargslen

2. Create the FIT Image:
^^^^^^^^^^^^^^^^^^^^^^^^

Create a new FIT image that includes the fixed device-tree generated in the
previous step. You will also need to add a signature node to the SPL's DTB
containing the keys to authenticate the new FIT.

Create a ``fitImage.its`` file with the following contents:

.. code-block:: dts

    /dts-v1/;

    / {
        description = "Kernel fitImage";
        #address-cells = <1>;

        images {
            kernel {
                description = "Linux kernel";
                data = /incbin/("Image");
                type = "kernel";
                arch = "arm64";
                os = "linux";
                compression = "none";
                load = <0x82000000>;
                entry = <0x82000000>;
                hash-1 {
                    algo = "sha512";
                };
            };

            fdt-falcon {
                description = "Flattened Device Tree blob";
                data = /incbin/("k3-am625-sk-falcon.dtb");
                type = "flat_dt";
                arch = "arm64";
                compression = "none";
                load = <0x88000000>;
                entry = <0x88000000>;
                hash-1 {
                    algo = "sha512";
                };
            };
        };

        configurations {
            default = "conf-ti_am625";

            conf-ti_am625 {
                description = "Linux kernel, FDT blob";
                kernel = "kernel";
                fdt = "fdt-falcon";
                hash-1 {
                    algo = "sha512";
                };

                signature-1 {
                    algo = "sha512,rsa4096";
                    key-name-hint = "custMpk";
                    padding = "pkcs-1.5";
                    sign-images = "kernel", "fdt-falcon";
                };
            };
        };
    };

Then, use the mkimage tool to create the FIT image and modify SPL's DTB:

.. prompt:: bash $

        tools/mkimage -f fitImage.its -K build/spl/dts/ti/k3-am625-sk.dtb -k arch/arm/mach-k3/keys -r fitImage

3. Rebuild U-Boot SPL:
^^^^^^^^^^^^^^^^^^^^^^

With the newly created ``fitImage`` written to the boot partition of the SD card
and the keys added to the SPL's device-tree, you can rebuild the SPL with the
following configuration fragment to enable Falcon Mode:

::

        CONFIG_SPL_OS_BOOT=y
        CONFIG_SPL_OS_BOOT_SECURE=y
        CONFIG_SPL_FIT_SIGNATURE=y
        CONFIG_SPL_RSA=y

        # Only support MMC falcon mode
        CONFIG_SPL_SPI_FLASH_SUPPORT=n
        CONFIG_SPL_NOR_SUPPORT=n
        CONFIG_SPL_NAND_SUPPORT=n

        # We don't need TIFS authenticating the FIT
        CONFIG_SPL_FIT_IMAGE_POST_PROCESS=n

        # Modify memory map to allow more space for the larger FIT
        CONFIG_SPL_STACK_R_ADDR=0x88000000
        CONFIG_SPL_LOAD_FIT_ADDRESS=0x82000000
        CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN=0x4000000

Console Log
~~~~~~~~~~~

The following console log output shows the boot process with Secure Falcon Mode
enabled:

::

        U-Boot SPL 2025.10-rc5-00482-ge14055bfa9d1-dirty (Oct 09 2025 - 14:31:50 +0530)
        SYSFW ABI: 4.0 (firmware rev 0x000b '11.0.7--v11.00.07 (Fancy Rat)')
        SPL initial stack usage: 1968 bytes
        Trying to boot from MMC2
        ## Checking hash(es) for config conf-ti_am625 ... sha512,rsa4096:custMpk+ OK
        ## Checking hash(es) for Image kernel ... sha512+ OK
        ## Checking hash(es) for Image fdt-falcon ... sha512+ OK
        [    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd034]
        [    0.000000] Linux version 6.6.58-ti-01497-ga7758da17c28-dirty (oe-user@oe-host) (aarch64-oe-linux-gcc (GCC) 14.2.0, GNU ld (GNU Binutils) 2.43.1
        .20241111) #1 SMP PREEMPT Wed Nov 27 13:23:15 UTC 2024
        [    0.000000] KASLR enabled
        [    0.000000] Machine model: Texas Instruments AM625 SK
        [    0.000000] efi: UEFI not found.
        [    0.000000] Reserved memory: created CMA memory pool at 0x00000000f8000000, size 128 MiB
        [    0.000000] OF: reserved mem: initialized node linux,cma, compatible id shared-dma-pool
        [    0.000000] OF: reserved mem: 0x00000000f8000000..0x00000000ffffffff (131072 KiB) map reusable linux,cma
        [    0.000000] OF: reserved mem: 0x0000000080000000..0x000000008007ffff (512 KiB) nomap non-reusable tfa@80000000
        [    0.000000] OF: reserved mem: 0x000000009c700000..0x000000009c7fffff (1024 KiB) map non-reusable ramoops@9c700000
        [    0.000000] Reserved memory: created DMA memory pool at 0x000000009c800000, size 3 MiB
        [    0.000000] OF: reserved mem: initialized node ipc-memories@9c800000, compatible id shared-dma-pool
        [    0.000000] OF: reserved mem: 0x000000009c800000..0x000000009cafffff (3072 KiB) nomap non-reusable ipc-memories@9c800000
        [    0.000000] Reserved memory: created DMA memory pool at 0x000000009cb00000, size 1 MiB
        [    0.000000] OF: reserved mem: initialized node m4f-dma-memory@9cb00000, compatible id shared-dma-pool
        [    0.000000] OF: reserved mem: 0x000000009cb00000..0x000000009cbfffff (1024 KiB) nomap non-reusable m4f-dma-memory@9cb00000
        [    0.000000] Reserved memory: created DMA memory pool at 0x000000009cc00000, size 14 MiB
        [    0.000000] OF: reserved mem: initialized node m4f-memory@9cc00000, compatible id shared-dma-pool
        [    0.000000] OF: reserved mem: 0x000000009cc00000..0x000000009d9fffff (14336 KiB) nomap non-reusable m4f-memory@9cc00000
        [    0.000000] Reserved memory: created DMA memory pool at 0x000000009da00000, size 1 MiB
        [    0.000000] OF: reserved mem: initialized node r5f-dma-memory@9da00000, compatible id shared-dma-pool
        [    0.000000] OF: reserved mem: 0x000000009da00000..0x000000009dafffff (1024 KiB) nomap non-reusable r5f-dma-memory@9da00000
        [    0.000000] Reserved memory: created DMA memory pool at 0x000000009db00000, size 12 MiB
        [    0.000000] OF: reserved mem: initialized node r5f-memory@9db00000, compatible id shared-dma-pool
        [    0.000000] OF: reserved mem: 0x000000009db00000..0x000000009e6fffff (12288 KiB) nomap non-reusable r5f-memory@9db00000
        [    0.000000] OF: reserved mem: 0x000000009e800000..0x000000009fffffff (24576 KiB) nomap non-reusable optee@9e800000
        [    0.000000] Zone ranges:
        [    0.000000]   DMA      [mem 0x0000000080000000-0x00000000ffffffff]
        [    0.000000]   DMA32    empty
        [    0.000000]   Normal   empty
        [    0.000000] Movable zone start for each node
        [    0.000000] Early memory node ranges
        [    0.000000]   node   0: [mem 0x0000000080000000-0x000000008007ffff]
        [    0.000000]   node   0: [mem 0x0000000080080000-0x000000009c7fffff]
        [    0.000000]   node   0: [mem 0x000000009c800000-0x000000009e6fffff]
        [    0.000000]   node   0: [mem 0x000000009e700000-0x000000009e7fffff]
        [    0.000000]   node   0: [mem 0x000000009e800000-0x000000009fffffff]
        [    0.000000]   node   0: [mem 0x00000000a0000000-0x00000000ffffffff]
        [    0.000000] Initmem setup node 0 [mem 0x0000000080000000-0x00000000ffffffff]
        [    0.000000] psci: probing for conduit method from DT.
        [    0.000000] psci: PSCIv1.1 detected in firmware.
        [    0.000000] psci: Using standard PSCI v0.2 function IDs
        [    0.000000] psci: Trusted OS migration not required
        [    0.000000] psci: SMC Calling Convention v1.5
        [    0.000000] percpu: Embedded 20 pages/cpu s43176 r8192 d30552 u81920
        [    0.000000] Detected VIPT I-cache on CPU0
        [    0.000000] CPU features: detected: GIC system register CPU interface
        [    0.000000] CPU features: kernel page table isolation forced ON by KASLR
        [    0.000000] CPU features: detected: Kernel page table isolation (KPTI)
        [    0.000000] CPU features: detected: ARM erratum 845719
        [    0.000000] alternatives: applying boot alternatives
        [    0.000000] Kernel command line: console=ttyS2,115200n8 root=/dev/mmcblk1p2 rw rootfstype=ext4 rootwait

Falcon Mode Boot on RISC-V
--------------------------

Introduction
~~~~~~~~~~~~

In the RISC-V environment, OpenSBI is required to enable a supervisor mode
binary to execute certain privileged operations. The typical boot sequence on
RISC-V is SPL -> OpenSBI -> U-Boot -> Linux kernel. SPL will load and start
the OpenSBI initializations, then OpenSBI will bring up the next image, U-Boot
proper. The OpenSBI binary must be prepared in advance of the U-Boot build
process and it will be packed together with U-Boot into a file called
u-boot.itb.

The Falcon Mode on RISC-V platforms is a distinct boot sequence. Borrowing
ideas from the U-Boot Falcon Mode on ARM, it skips the U-Boot proper phase
in the normal boot process and allows OpenSBI to load and start the Linux
kernel. Its boot sequence is SPL -> OpenSBI -> Linux kernel. The OpenSBI
binary and Linux kernel binary must be prepared prior to the U-Boot build
process and they will be packed together as a FIT image named linux.itb in
this process.

CONFIG_SPL_LOAD_FIT_OPENSBI_OS_BOOT enables the Falcon Mode boot on RISC-V.
This configuration setting tells OpenSBI that Linux kernel is its next OS
image and makes it load and start the kernel afterwards.

Note that the Falcon Mode boot bypasses a lot of initializations by U-Boot.
If the Linux kernel expects hardware initializations by U-Boot, make sure to
port the relevant code to the SPL build process.

Configuration
~~~~~~~~~~~~~

CONFIG_SPL_LOAD_FIT_ADDRESS
    Specifies the address to load u-boot.itb in a normal boot. When the Falcon
    Mode boot is enabled, it specifies the load address of linux.itb.

CONFIG_SYS_TEXT_BASE
    Specifies the address of the text section for a u-boot proper in a normal
    boot. When the Falcon Mode boot is enabled, it specifies the text section
    address for the Linux kernel image.

CONFIG_SPL_PAYLOAD_ARGS_ADDR
    The address in the RAM to which the FDT blob is to be moved by the SPL.
    SPL places the FDT blob right after the kernel. As the kernel does not
    include the BSS section in its size calculation, SPL ends up placing
    the FDT blob within the BSS section of the kernel. This may cause the
    FDT blob to be cleared during kernel BSS initialization. To avoid the
    issue, be sure to move the FDT blob out of the kernel first.

CONFIG_SPL_LOAD_FIT_OPENSBI_OS_BOOT
    Activates the Falcon Mode boot on RISC-V.

Example for Andes AE350 Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A FDT blob is required to boot the Linux kernel from the SPL. Andes AE350
platforms generally come with a builtin dtb. To load a custom DTB, follow
these steps:

1. Load the custom DTB to SDRAM::

        => fatload mmc 0:1 0x20000000 user_custom.dtb

2. Set the SPI speed::

        => sf probe 0:0 50000000 0

3. Erase sectors from the SPI Flash::

        => sf erase 0xf0000 0x10000

4. Write the FDT blob to the erased sectors of the Flash::

        => sf write 0x20000000 0xf0000 0x10000

Console Log of AE350 Falcon Mode Boot
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

        U-Boot SPL 2023.01-00031-g777ecdea66 (Oct 31 2023 - 18:41:36 +0800)
        Trying to boot from RAM

        OpenSBI v1.2-51-g7304e42
           ____                    _____ ____ _____
          / __ \                  / ____|  _ \_   _|
         | |  | |_ __   ___ _ __ | (___ | |_) || |
         | |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
         | |__| | |_) |  __/ | | |____) | |_) || |_
          \____/| .__/ \___|_| |_|_____/|____/_____|
                | |
                |_|

        Platform Name             : andestech,ax25
        Platform Features         : medeleg
        Platform HART Count       : 1
        Platform IPI Device       : andes_plicsw
        Platform Timer Device     : andes_plmt @ 60000000Hz
        Platform Console Device   : uart8250
        Platform HSM Device       : andes_smu
        Platform PMU Device       : andes_pmu
        Platform Reboot Device    : atcwdt200
        Platform Shutdown Device  : ---
        Firmware Base             : 0x0
        Firmware Size             : 196 KB
        Runtime SBI Version       : 1.0

        Domain0 Name              : root
        Domain0 Boot HART         : 0
        Domain0 HARTs             : 0*
        Domain0 Region00          : 0x0000000000000000-0x000000000003ffff ()
        Domain0 Region01          : 0x00000000e6000000-0x00000000e60fffff (I,R)
        Domain0 Region02          : 0x00000000e6400000-0x00000000e67fffff (I)
        Domain0 Region03          : 0x0000000000000000-0xffffffffffffffff (R,W,X)
        Domain0 Next Address      : 0x0000000001800000
        Domain0 Next Arg1         : 0x0000000001700000
        Domain0 Next Mode         : S-mode
        Domain0 SysReset          : yes

        Boot HART ID              : 0
        Boot HART Domain          : root
        Boot HART Priv Version    : v1.11
        Boot HART Base ISA        : rv64imafdcx
        Boot HART ISA Extensions  : none
        Boot HART PMP Count       : 8
        Boot HART PMP Granularity : 4
        Boot HART PMP Address Bits: 31
        Boot HART MHPM Count      : 4
        Boot HART MHPM Bits       : 64
        Boot HART MIDELEG         : 0x0000000000000222
        Boot HART MEDELEG         : 0x000000000000b109
        [    0.000000] Linux version 6.1.47-09019-g0584b09ad862-dirty
        [    0.000000] OF: fdt: Ignoring memory range 0x0 - 0x1800000
        [    0.000000] Machine model: andestech,ax25
        [    0.000000] earlycon: sbi0 at I/O port 0x0 (options '')
        [    0.000000] printk: bootconsole [sbi0] enabled
        [    0.000000] Disabled 4-level and 5-level paging
        [    0.000000] efi: UEFI not found.
        [    0.000000] Zone ranges:
        [    0.000000]   DMA32    [mem 0x0000000001800000-0x000000003fffffff]
        [    0.000000]   Normal   empty
        [    0.000000] Movable zone start for each node
        [    0.000000] Early memory node ranges
        [    0.000000]   node   0: [mem 0x0000000001800000-0x000000003fffffff]
        [    0.000000] Initmem setup node 0 [mem 0x0000000001800000-0x000000003fffffff]
        [    0.000000] SBI specification v1.0 detected
        [    0.000000] SBI implementation ID=0x1 Version=0x10002
        [    0.000000] SBI TIME extension detected
        [    0.000000] SBI IPI extension detected
        [    0.000000] SBI RFENCE extension detected
        [    0.000000] SBI SRST extension detected
        [    0.000000] SBI HSM extension detected
        [    0.000000] riscv: base ISA extensions acim
        [    0.000000] riscv: ELF capabilities acim
        [    0.000000] percpu: Embedded 18 pages/cpu s35000 r8192 d30536 u73728
        [    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 252500
