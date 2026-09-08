.. SPDX-License-Identifier: GPL-2.0+

Renesas R-Car Gen5 X5H Ironhide board
=====================================

Renesas R-Car Gen5 X5H Ironhide board U-Boot can be built for two separate cores:

- Cortex-M33 RSIP core, which is the boot core
- Cortex-A720AE core, which is the application core

Cortex-A720AE target
^^^^^^^^^^^^^^^^^^^^

Build U-Boot
------------

Please follow :doc:`Renesas 64-bit ARM SoC build environment setup <build-env-aarch64>`
to correctly set up the build environment before attempting to build U-Boot.

Clone up to date U-Boot source code and change directory into the
newly cloned source directory:

.. code-block:: console

    $ git clone https://git.u-boot-project.org/u-boot/u-boot.git
    $ cd u-boot

Configure U-Boot:

.. code-block:: console

    $ make r8a78000_ironhide_defconfig

Compile U-Boot:

.. code-block:: console

    $ make

To speed up build process, -jN option can be passed to make to start
multiple jobs at the same time, this is beneficial especially on SMP
systems. The following example starts up to number of CPUs in the
system jobs, which is the recommended amount:

.. code-block:: console

    $ make -j$(nproc)

Install U-Boot
--------------

In order to install U-Boot for Cortex-A720AE into UFS, first build U-Boot
for this target and collect ``u-boot-elf.srec`` build artifact.

Next, configure the board for SCIF loader boot. Upload IPL flash_writer
mot binary. Use the tool to write ``u-boot-elf.srec`` into HyperFlash
at offset 0x8e300000 . Finally, power off the board and configure the
board back to HyperFlash boot mode.

Cortex-M33 RSIP target
^^^^^^^^^^^^^^^^^^^^^^

Build U-Boot
------------

Please follow :doc:`Renesas 32-bit ARM SoC build environment setup <build-env-aarch32>`
to correctly set up the build environment before attempting to build U-Boot.

Clone up to date U-Boot source code and change directory into the
newly cloned source directory:

.. code-block:: console

    $ git clone https://git.u-boot-project.org/u-boot/u-boot.git
    $ cd u-boot

Configure U-Boot:

.. code-block:: console

    $ make r8a78000_ironhide_cm33_defconfig

Compile U-Boot:

.. code-block:: console

    $ make

To speed up build process, -jN option can be passed to make to start
multiple jobs at the same time, this is beneficial especially on SMP
systems. The following example starts up to number of CPUs in the
system jobs, which is the recommended amount:

.. code-block:: console

    $ make -j$(nproc)

Install U-Boot
--------------

In order to install U-Boot for RSIP into HyperFlash, first build U-Boot
for this target and collect ``u-boot-elf.shdr`` and ``u-boot-elf.srec``
build artifacts.

Next, configure the board for SCIF loader boot. Upload IPL flash_writer
mot binary. Use the tool to write ``u-boot-elf.shdr`` into HyperFlash
at offset 0, and ``u-boot-elf.srec`` into HyperFlash at offset 0x40000 .
Finally, power off the board and configure the board back to HyperFlash
boot mode.

Power on the board, U-Boot on RSIP will start. Interaction with U-Boot
on RSIP is possible via HSCIF1, which is the second serial console that
is available on the USB-to-Serial adapter port. HSCIF1 is used in order
to avoid interference with software running on the Cortex-A720AE cores,
which uses HSCIF0.

Ethernet boot of bootloader components
--------------------------------------

The U-Boot for RSIP is capable of ethernet access, which allows download
of bootloader components via TFTP. This is useful during development and
can be used for fast iterative testing of either SCP firmware, TFA BL31,
OPTEE-OS, U-Boot or Linux on the SCP core and Cortex-A720AE cores
respectively.

An example U-Boot environment applicable to ``include/configs/rcar-gen5-common.h``
or executable manually is listed below. The environment script ``rsip_ipl_boot_ca0``
implements download of SCP firmware ``scp.bin``, TFA BL31 ``bl31.bin``,
U-Boot ``u-boot.bin`` and Linux ``fitImage`` from TFTP server at address
``192.168.1.1/24`` and starts those components on the SCP and Cortex-A720AE
cores respectively. OPTEE-OS is loaded from UFS to retain at least this
example of UFS loading, however, it perfectly fine to download OPTEE-OS
via TFTP in the same manner as the other components are downloaded:

.. code-block:: console

    rsip_ipl_params_base=0x8c100000
    rsip_ipl_params_optee=0x8c100088
    rsip_ipl_params_uboot=0x8c100030
    rsip_ipl_scp_ep=0x8c180000
    rsip_ipl_optee_ep=0x8c400000
    rsip_ipl_tfa_ep=0x8c200000
    rsip_ipl_uboot_ep=0x8e300000
    rsip_ipl_linux_ep=0x91000000

    rsip_ipl_params_write=                       /* Build handoff structure */ \
            base ${rsip_ipl_params_base} ;                                     \
            mw 0x00 0 0x9e ;                     /* Clear the area */          \
            mw 0x00 0x00300103 ;                 /* type, version, size */     \
            mw 0x20 0x${rsip_ipl_params_uboot} ; /* U-Boot descriptor */       \
                                                                               \
            base ${rsip_ipl_params_uboot} ;                                    \
            mw 0x00 0x00580101 ;                 /* type, version, size */     \
            mw 0x04 0x00000001 ;                 /* attr */                    \
            mw 0x08 ${rsip_ipl_uboot_ep} ;       /* U-Boot entry point */      \
            mw 0x10 0x000003c5 ;                 /* SPSR */                    \
                                                                               \
            base ${rsip_ipl_params_optee} ;                                    \
            mw 0x00 0x00580201 ;                 /* type, version, size */     \
            mw 0x04 0x00000008 ;                 /* attr */                    \
            mw 0x08 ${rsip_ipl_optee_ep} ;       /* OPTEE-OS entry point */    \
            mw 0x10 0x000003c5 ;                 /* SPSR */                    \
                                                                               \
            base 0

    rsip_ipl_boot_ca0= /* Start TFA BL31, OPTEE-OS, U-Boot, Linux on Cortex-A720AE core 0 */   \
            env set ipaddr 192.168.1.10 &&                                                     \
            env set serverip 192.168.1.1 &&                                                    \
            env set netmask 255.255.255.0 &&                                                   \
                                                                                               \
            tftp ${rsip_ipl_scp_ep} scp.bin &&                                                 \
            tftp ${rsip_ipl_tfa_ep} bl31.bin &&                                                \
            tftp ${rsip_ipl_uboot_ep} u-boot.bin &&                                            \
            tftp ${rsip_ipl_linux_ep} fitImage &&                                              \
                                                                                               \
            scsi scan &&                                   /* Scan for UFS devices */          \
            rproc init &&                                  /* Start remoteproc */              \
            rproc load 0 ${rsip_ipl_scp_ep} 0x60000 &&     /* Load SCP STCM */                 \
            rproc start 0 &&                               /* Start SCP */                     \
            scsi read ${rsip_ipl_optee_ep} 0x5200 0x200 && /* Load OPTEE-OS from UFS */        \
            run rsip_ipl_params_write &&                   /* Write entry point descriptors */ \
                                                                                               \
            rproc load 13 ${rsip_ipl_tfa_ep} 4 &&          /* Set up Cortex-A720AE Core 0 */   \
            rproc start 13                                 /* Start Cortex-A720AE Core 0 */

.. note::

    U-Boot on RSIP environment is not persistent across reboots,
    but this will likely change in the upcoming U-Boot release.

.. note::

    U-Boot on RSIP can start the SCP core via rproc command, but
    it can not stop SCP after it was started. This was intended
    as a safety mechanism, since SCP is central component of the
    system, however, this will likely change in the upcoming U-Boot
    release.

.. note::

    U-Boot on RSIP can start non-SCP cores via ``rproc`` command only
    after the SCP got started, because those cores are started via
    SCMI calls to the SCP.

Cortex-R52 core start
---------------------

The U-Boot for RSIP remoteproc implementation is capable of starting
the SCP core, and using the SCP it is capable of starting additional
CPU cores in the SoC, Cortex-R52 and Cortex-A720AE. This subchapter
demonstrates how to start example code on Cortex-R52 cores using the
U-Boot on RSIP remoteproc and SCP.

The piece of position independent assembler code below can be compiled
for the Cortex-R52 core, can be started from any 4-Byte aligned address
and prints CPU core program counter (PC) address and MPIDR onto HSCIF1.
The MPIDR register encodes CPU cluster and core position and is useful
when identifying on which specific CPU core does the code execute. The
program counter (PC) can be used to determine from which address did
the code running on the Cortex-R52 core start executing.

.. code-block:: console

    #define HSTDR        0xc
    #define HSFSR        0x10
    #define TEND         0x40

    .macro mprintchr, chr, rt
            mov          \rt, \chr
            bl           printchr
    .endm

    .macro mprintnl, rt
            mprintchr    #'\r', \rt
            mprintchr    #'\n', \rt
    .endm

    start:

    # Set up HSCIF1 output
    mov                  r0,     #0xc0000000
    orr                  r0, r0, #0x00710000
    orr                  r0, r0, #0x00004000

    # Print newline
    mprintnl             r1

    # Print PC:
    mprintchr            #'P', r1
    mprintchr            #'C', r1
    mprintchr            #':', r1
    mprintchr            #' ', r1
    bl                   printchr
    bl                   printchr
    bl                   printchr
    mprintchr            #'0', r1
    mprintchr            #'x', r1

    # Figure out start address and print it
    adr                  r2, start
    bl                   printhex

    # Print newline
    mprintnl             r1

    # Print MPIDR:
    mprintchr            #'M', r1
    mprintchr            #'P', r1
    mprintchr            #'I', r1
    mprintchr            #'D', r1
    mprintchr            #'R', r1
    mprintchr            #':', r1
    mprintchr            #' ', r1
    mprintchr            #'0', r1
    mprintchr            #'x', r1

    # Read MPIDR
    mrc                  p15, 0, r2, c0, c0, 5
    bl                   printhex

    # Print newline
    mprintnl             r1

    # Halt and do nothing
    3:      wfi
            b            3b

    # Print char byte of register r1 (clobber: r9)
    printchr:
    2:      ldrh         r9, [r0, #HSFSR]
            tst          r9, #TEND
            beq          2b
            strb         r1, [r0, #HSTDR]
            ldrh         r9, [r0, #HSFSR]
            bic          r9, #TEND
            strh         r9, [r0, #HSFSR]

            bx           lr

    # Print hex content of register r2 (clobber: r6, r7, r8, r9)
    printhex:
            mov          r8, #8

    1:      and          r1, r2, #0xf0000000
            lsl          r2, r2, #4
            lsr          r1, r1, #28
            add          r1, r1, #'0'
            cmp          r1, #'9'
            # >= '9' => add 'a' - ':'
            addgt        r1, r1, #('a' - ':')

            mov          r6, lr
            bl           printchr
            mov          lr, r6

            sub          r8, r8, #1
            cmp          r8, #0
            bne          1b

            bx           lr

Compile the piece of position independent assembler code above for
the Cortex-R52 core as follows:

.. code-block:: console

    $ cpp code.s > temp.S
    $ arm-linux-gnueabi-as -march=armv8-r -o temp.o temp.S
    $ arm-linux-gnueabi-objcopy -O binary temp.o output.bin

Load the piece of compiled assembler code into memory, and start the
code on the Cortex-R52 cluster 0 core 0 as follows:

.. important::

    It is mandatory for the SCP core to be started before the following
    procedure can be performed, because the Cortex-R52 core is started
    by the SCP. The SCP is usually started by the default boot command
    of the U-Boot on RSIP, which contains ``rproc start 0`` invocation to
    start the SCP core.

.. note::

    The example below does use ethernet loading of the Cortex-R52
    firmware, however, it is perfectly fine to load the firmware from
    either HyperFlash, UFS, or other storage media.

.. note::

    The example below does use load address 0x86000000 for the Cortex-R52
    firmware. This address is located in DRAM and is only ever used as a
    temporary load buffer. The firmware is copied into the RT-VRAM which
    is accessible from the RSIP at remapped address 0xb0200000 and from
    the Cortex-R52 at address 0x10200000, which is also the address from
    which the firmware executes on Cortex-R52.

.. code-block:: console

    # Configure ethernet address, adjust as needed:
    => env set ipaddr 192.168.1.10
    => env set serverip 192.168.1.1
    => env set netmask 255.255.255.0

    # Load Cortex-R52 firmware into DRAM at address 0x86000000
    => tftp 0x86000000 output.bin
    # Copy Cortex-R52 firmware into RT-VRAM
    => cp.b 0x86000000 0xb0200000 ${filesize}
    # Configure Cortex-R52 cluster 0 core 0 entry point
    => rproc load 1 0x10200000 ${filesize}
    # Start Cortex-R52 cluster 0 core 0
    => rproc start 1

The expected output of ``rproc start 1`` a print of both MPIDR and PC register
values on HSCIF1.

.. code-block:: console

    Load Remote Processor 1 with data@addr=0x10200000 304 bytes: Success!

    PC:    0x10200000
    MPIDR: 0x80000000

.. note::

    The MPIDR register value above is 0x80000000 . Cortex-R52 MPIDR register
    bit 31 is always set to 1, bitfields AFF2[23:16], AFF1[15:8], AFF0[7:0]
    describe core affinity, in this case this is cluster 0, core 0.

Other Cortex-R52 cores in other clusters can be started by passing a
matching core number to both ``rproc load`` and ``rproc start``. Example:

.. code-block:: console

    => rproc list
    ...
    1 - Name:'rcar-rsip-cr.0-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    2 - Name:'rcar-rsip-cr.1-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    3 - Name:'rcar-rsip-cr.2-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    4 - Name:'rcar-rsip-cr.3-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    5 - Name:'rcar-rsip-cr.4-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    6 - Name:'rcar-rsip-cr.5-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    7 - Name:'rcar-rsip-cr.6-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    8 - Name:'rcar-rsip-cr.7-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    9 - Name:'rcar-rsip-cr.8-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    10 - Name:'rcar-rsip-cr.9-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    11 - Name:'rcar-rsip-cr.10-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    12 - Name:'rcar-rsip-cr.11-scp@c1340000' type:'internal memory mapped' supports: load start stop reset is_running
    ...
    => rproc load 10 0x10200000 ${filesize} && rproc start 10
    Load Remote Processor 10 with data@addr=0x10200000 304 bytes: Success!

    PC:    0x10200000
    MPIDR: 0x80000201

.. note::

    The MPIDR register value above is 0x80000201 . Cortex-R52 MPIDR register
    bit 31 is always set to 1, bitfields AFF2[23:16], AFF1[15:8], AFF0[7:0]
    describe core affinity, in this case this is cluster 2, core 1.
