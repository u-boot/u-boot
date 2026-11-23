.. SPDX-License-Identifier: GPL-2.0+

Retronix R-Car Gen4 V4H Sparrow Hawk board
==========================================

- Retronix R-Car V4H Sparrow Hawk board: https://www.retronix.com.tw/en/product_sbc.html
- Retronix R-Car V4H Sparrow Hawk documenation: https://rcar-community.github.io/Sparrow-Hawk/index.html

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

    $ make r8a779g3_sparrowhawk_defconfig

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

In order to install U-Boot using write into SPI NOR, first build U-Boot
for this target and collect ``flash.bin`` build artifact. Then start the
target, drop into U-Boot shell, and load the build artifact into DRAM at
well known address:

.. code-block:: console

    => tftp 0x50000000 flash.bin

Finally, write U-Boot into SPI NOR:

.. code-block:: console

    => sf probe && sf update 0x50000000 0 ${filesize}

SPI NOR layout
--------------

The Retronix R-Car V4H Sparrow Hawk board 64 MiB SPI NOR layout is listed below.

.. table:: Retronix R-Car Gen4 V4H Sparrow Hawk SPI NOR layout

   +----------------+----------------+-----------+---------------------------------------------------------------+
   | Start          | End            | Size      | Content                                                       |
   +================+================+===========+===============================================================+
   | ``0x000_0000`` | ``0x02f_ffff`` |  3072 kiB | U-Boot bootloader ``flash.bin``                               |
   +----------------+----------------+-----------+---------------------------------------------------------------+
   | ``0x030_0000`` | ``0x033_ffff`` |   256 kiB | PCIe controller firmware ``rcar_gen4_pcie.bin`` (32 kiB used) |
   +----------------+----------------+-----------+---------------------------------------------------------------+
   | ``0x034_0000`` | ``0x3f7_ffff`` | 61696 kiB | UNUSED                                                        |
   +----------------+----------------+-----------+---------------------------------------------------------------+
   | ``0x3f8_0000`` | ``0x3fb_ffff`` |   256 kiB | U-Boot bootloader environment copy 1                          |
   +----------------+----------------+-----------+---------------------------------------------------------------+
   | ``0x3fc_0000`` | ``0x3ff_ffff`` |   256 kiB | U-Boot bootloader environment copy 2                          |
   +----------------+----------------+-----------+---------------------------------------------------------------+

Bundle TFA BL31 into Linux kernel fitImage
------------------------------------------

The Retronix R-Car V4H Sparrow Hawk board starts both TFA BL31 and Linux
kernel from U-Boot. Both TFA BL31, Linux kernel and DT blob have to be
bundled into the fitImage.

Perform the following steps to build TFA at least v2.14.y:

.. code-block:: console

    $ git clone https://github.com/ARM-software/arm-trusted-firmware.git
    $ cd arm-trusted-firmware
    $ make -j$(nproc) bl31 \
        PLAT=rcar_gen4 ARCH=aarch64 LSI=V4H SPD=none \
        CTX_INCLUDE_AARCH32_REGS=0 MBEDTLS_COMMON_MK=1 \
        PTP_NONSECURE_ACCESS=1 LOG_LEVEL=20 DEBUG=0 \
        ENABLE_ASSERTIONS=0

The bundling is done using U-Boot ``mkimage`` tool:

.. code-block:: console

    mkimage \
        -f auto -E -A arm64 -C none -e 0x50200000 -a 0x50200000 \
        -d /path/to/linux/arch/arm64/boot/Image \
        -b /path/to/linux/arch/arm64/boot/dts/renesas/r8a779g3-sparrow-hawk.dtb \
        -y /path/to/arm-trusted-firmware/build/rcar_gen4/*/bl31.bin \
        -Y 0x46400000 \
        /path/to/output/fitImage

Cortex-R52 core start
---------------------

The U-Boot for Cortex-A76 remoteproc implementation is capable of
starting additional CPU cores in the SoC, Cortex-R52 and Cortex-A76.
This subchapter demonstrates how to start example code on Cortex-R52
cores using the U-Boot on Cortex-A76 remoteproc.

The piece of position independent assembler code below can be compiled
for the Cortex-R52 core, can be started from any 4-Byte aligned address
and prints CPU core program counter (PC) address and MPIDR onto HSCIF0.
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

    # Wait a bit before printing to avoid corrupting active HSCIF output
            mov          r0,     #0x00040000
    1:      sub          r0, r0, #0x00000001
            cmp          r0,     #0x00000000
            bne 1b

    # Set up HSCIF output
    mov                  r0,     #0xe0000000
    orr                  r0, r0, #0x06000000
    orr                  r0, r0, #0x00500000
    orr                  r0, r0, #0x00040000

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

.. note::

    The example below does use ethernet loading of the Cortex-R52
    firmware, however, it is perfectly fine to load the firmware from
    either SD card, NVMe, or other storage media.

.. note::

    The example below does use load address 0x4b000000 for the Cortex-R52
    firmware. This address is located in DRAM and may be overwritten by
    other software while the Cortex-R52 is running.

.. code-block:: console

    # Configure ethernet address, adjust as needed:
    => env set ipaddr 192.168.1.10
    => env set serverip 192.168.1.1
    => env set netmask 255.255.255.0

    # Load Cortex-R52 firmware into DRAM at address 0x4b000000
    => tftp 0x4b000000 output.bin
    # Configure Cortex-R52 cluster 0 core 0 entry point
    => rproc load 0 0x4b000000 ${filesize}
    # Start Cortex-R52 cluster 0 core 0
    => rproc start 0

The expected output of ``rproc start 0`` a print of both MPIDR and PC register
values on HSCIF0.

.. code-block:: console

    Load Remote Processor 0 with data@addr=0x4b000000 324 bytes: Success!

    PC:    0x4b000000
    MPIDR: 0x80000000

.. note::

    The MPIDR register value above is 0x80000000 . Cortex-R52 MPIDR register
    bit 31 is always set to 1, bitfields AFF2[23:16], AFF1[15:8], AFF0[7:0]
    describe core affinity, in this case this is cluster 0, core 0.

Other Cortex-R52 cores in other clusters can be started by passing a
matching core number to both ``rproc load`` and ``rproc start``. Example:

.. code-block:: console

    => rproc list
    0 - Name:'rcar-apmu-cr52.0-apmu@e6170000' type:'internal memory mapped' supports: load start stop reset is_running
    1 - Name:'rcar-apmu-cr52.1-apmu@e6170000' type:'internal memory mapped' supports: load start stop reset is_running
    2 - Name:'rcar-apmu-cr52.2-apmu@e6170000' type:'internal memory mapped' supports: load start stop reset is_running
    => rproc load 2 0x4b000000 ${filesize} && rproc start 2
    Load Remote Processor 2 with data@addr=0x4b000000 324 bytes: Success!

    PC:    0x4b000000
    MPIDR: 0x80000200

.. note::

    The MPIDR register value above is 0x80000200 . Cortex-R52 MPIDR register
    bit 31 is always set to 1, bitfields AFF2[23:16], AFF1[15:8], AFF0[7:0]
    describe core affinity, in this case this is cluster 2, core 0.

Cortex-A76 core start
---------------------

The U-Boot for Cortex-A76 remoteproc implementation is capable of
starting additional CPU cores in the SoC, Cortex-R52 and Cortex-A76.
This subchapter demonstrates how to start example code on Cortex-A76
cores using the U-Boot on Cortex-A76 remoteproc.

The piece of position independent assembler code below can be compiled
for the Cortex-A76 core, can be started from any 4-Byte aligned address
and prints CPU core program counter (PC) address and MPIDR onto HSCIF0.
The MPIDR register encodes CPU cluster and core position and is useful
when identifying on which specific CPU core does the code execute. The
program counter (PC) can be used to determine from which address did
the code running on the Cortex-A76 core start executing.

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

    # Wait a bit before printing to avoid corrupting active HSCIF output
            mov          x0,     #0x00040000
    1:      sub          x0, x0, #0x00000001
            cmp          x0,     #0x00000000
            bne 1b

    # Set up HSCIF output
    mov                  x0,     #0xe0000000
    orr                  x0, x0, #0x06000000
    orr                  x0, x0, #0x00400000
    orr                  x0, x0, #0x00100000
    orr                  x0, x0, #0x00040000

    # Print newline
    mprintnl             w1

    # Print PC:
    mprintchr            #'P', w1
    mprintchr            #'C', w1
    mprintchr            #':', w1
    mprintchr            #' ', w1
    bl                   printchr
    bl                   printchr
    bl                   printchr
    mprintchr            #'0', w1
    mprintchr            #'x', w1

    # Figure out start address and print it
    adr                  x2, start
    bl                   printhex16

    # Print newline
    mprintnl             w1

    # Print MPIDR:
    mprintchr            #'M', w1
    mprintchr            #'P', w1
    mprintchr            #'I', w1
    mprintchr            #'D', w1
    mprintchr            #'R', w1
    mprintchr            #':', w1
    mprintchr            #' ', w1
    mprintchr            #'0', w1
    mprintchr            #'x', w1

    # Read MPIDR
    mrs                  x2, mpidr_el1
    bl                   printhex8

    # Print newline
    mprintnl             w1

    # Prepare core power off, used by rproc stop
    mov                  x2, #1
    msr                  S3_0_C15_C2_7, x2

    # Halt and do nothing
    3:      wfi
            b            3b

    # Print char byte of register w1 (clobber: w9)
    printchr:
    2:      ldrh         w9, [x0, #HSFSR]
            tst          w9, #TEND
            beq          2b
            strb         w1, [x0, #HSTDR]
            ldrh         w9, [x0, #HSFSR]
            bic          w9, w9, #TEND
            strh         w9, [x0, #HSFSR]

            ret

    # Print hex content of register x2 (clobber: x6, x7, x8, x9)
    printhex16:
            mov          x8, #16

    1:      and          x1, x2, #0xf000000000000000
            lsl          x2, x2, #4
            lsr          x1, x1, #60
            add          x1, x1, #'0'
            cmp          x1, #'9'
            # >= '9' => add 'a' - ':'
            blt          4f
            add          x1, x1, #('a' - ':')
    4:
            mov          x6, lr
            bl           printchr
            mov          lr, x6

            sub          x8, x8, #1
            cmp          x8, #0
            bne          1b

            ret

    # Print hex content of register x2 (clobber: x6, x7, x8, x9)
    printhex8:
            mov          x8, #8

    1:      and          x1, x2, #0xf0000000
            lsl          x2, x2, #4
            lsr          x1, x1, #28
            add          x1, x1, #'0'
            cmp          x1, #'9'
            # >= '9' => add 'a' - ':'
            blt          4f
            add          x1, x1, #('a' - ':')
    4:
            mov          x6, lr
            bl           printchr
            mov          lr, x6

            sub          x8, x8, #1
            cmp          x8, #0
            bne          1b

            ret

Compile the piece of position independent assembler code above for
the Cortex-A76 core as follows:

.. code-block:: console

    $ cpp code.s > temp.S
    $ aarch64-linux-gnu-as -march=armv8-a -o temp.o temp.S
    $ aarch64-linux-gnu-objcopy -O binary temp.o output.bin

Load the piece of compiled assembler code into memory, and start the
code on the Cortex-A76 cluster 1 core 1 as follows:

.. note::

    The example below does use ethernet loading of the Cortex-A76
    firmware, however, it is perfectly fine to load the firmware from
    either SD card, NVMe, or other storage media.

.. note::

    The example below does use load address 0x4b000000 for the Cortex-A76
    firmware. This address is located in DRAM and may be overwritten by
    other software while the Cortex-A76 is running.

.. note::

    The example code does invoke ``msr S3_0_C15_C2_7, x2`` instruction with
    register ``x2`` set to ``1`` before invoking the ``wfi`` instruction.
    This sets per-CPU ``CPUPWRCTRL`` register bit 0 ``CPUPWRCTLR_PWDN``,
    which must be set on the CPU core in order to power off the CPU core
    when the ``wfi`` instruction is invoked. This setting is mandatory for
    the correct behavior of ``rproc stop N`` of the CPU core N, in case of
    this example, that is ``rproc stop 6`` for CPU core 6. In case the bit
    ``CPUPWRCTLR_PWDN`` is not set, the CPU core remains in power on state.

.. code-block:: console

    # Configure ethernet address, adjust as needed:
    => env set ipaddr 192.168.1.10
    => env set serverip 192.168.1.1
    => env set netmask 255.255.255.0

    # Load Cortex-A76 firmware into DRAM at address 0x4b000000
    => tftp 0x4b000000 output.bin
    # Configure Cortex-A76 cluster 1 core 1 entry point
    => rproc load 6 0x4b000000 ${filesize}
    # Start Cortex-A76 cluster 1 core 1
    => rproc start 6

The expected output of ``rproc start 6`` a print of both MPIDR and PC register
values on HSCIF0.

.. code-block:: console

    Load Remote Processor 6 with data@addr=0x4b000000 448 bytes: Success!

    PC:    0x000000004b000000
    MPIDR: 0x81010100

.. note::

    The MPIDR register value above is 0x81010100 . Cortex-A76 MPIDR register
    bit 31 is always set to 1, bitfields AFF2[23:16], AFF1[15:8] describe
    cluster and core affinity, in this case this is cluster 1, core 1.

Other Cortex-A76 cores in other clusters can be started by passing a
matching core number to both ``rproc load`` and ``rproc start``. Example:

.. code-block:: console

    => rproc list
    ...
    3 - Name:'rcar-apmu-ca76.0-apmu@e6170000' type:'internal memory mapped' supports: load start stop reset is_running
    4 - Name:'rcar-apmu-ca76.1-apmu@e6170000' type:'internal memory mapped' supports: load start stop reset is_running
    5 - Name:'rcar-apmu-ca76.2-apmu@e6170000' type:'internal memory mapped' supports: load start stop reset is_running
    6 - Name:'rcar-apmu-ca76.3-apmu@e6170000' type:'internal memory mapped' supports: load start stop reset is_running
    => rproc load 5 0x4b000000 ${filesize} && rproc start 5
    Load Remote Processor 5 with data@addr=0x4b000000 448 bytes: Success!

    PC:    0x000000004b000000
    MPIDR: 0x81010000

.. note::

    The MPIDR register value above is 0x81010000 . Cortex-A76 MPIDR register
    bit 31 is always set to 1, bitfields AFF2[23:16], AFF1[15:8] describe
    cluster and core affinity, in this case this is cluster 1, core 0.
