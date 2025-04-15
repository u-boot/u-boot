.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (c) 2024 Alexander Dahl

Debugging U-Boot with GDB
=========================

Using a JTAG adapter it is possible to debug a running U-Boot with GDB.
A common way is to connect a debug adapter to the JTAG connector of your
board, run a GDB server, connect GDB to the GDB server, and use GDB as usual.

Similarly, QEMU can provide a GDB server.

Preparing build
---------------

Building U-Boot with reduced optimization (-Og) and without link time
optimization is recommended for easier debugging::

    CONFIG_CC_OPTIMIZE_FOR_DEBUG=y
    CONFIG_LTO=n

Otherwise build, install, and run U-Boot as usual.

Using OpenOCD as GDB server
---------------------------

`OpenOCD <https://openocd.org/>`_ is an open-source tool supporting hardware
debug probes, and providing a GDB server. It is readily available in major Linux
distributions or you can build it from source.

Here is example of starting OpenOCD on Debian using a J-Link adapter and a
board with an AT91 SAMA5D2 SoC:

.. code-block:: console

    $ openocd -f interface/jlink.cfg -f target/at91sama5d2.cfg -c 'adapter speed 4000'
    Open On-Chip Debugger 0.12.0
    Licensed under GNU GPL v2
    For bug reports, read
            http://openocd.org/doc/doxygen/bugs.html
    Info : auto-selecting first available session transport "jtag". To override use 'transport select <transport>'.
    adapter speed: 4000 kHz

    Info : Listening on port 6666 for tcl connections
    Info : Listening on port 4444 for telnet connections
    Info : J-Link V10 compiled Jan 30 2023 11:28:07
    Info : Hardware version: 10.10
    Info : VTarget = 3.244 V
    Info : clock speed 4000 kHz
    Info : JTAG tap: at91sama5d2.cpu tap/device found: 0x5ba00477 (mfg: 0x23b (ARM Ltd), part: 0xba00, ver: 0x5)
    Info : at91sama5d2.cpu_a5.0: hardware has 3 breakpoints, 2 watchpoints
    Info : at91sama5d2.cpu_a5.0: MPIDR level2 0, cluster 0, core 0, mono core, no SMT
    Info : starting gdb server for at91sama5d2.cpu_a5.0 on 3333
    Info : Listening on port 3333 for gdb connections

Notice that OpenOCD is listening on port 3333 for GDB connections.

Using QEMU as GDB server
------------------------

When running U-Boot on QEMU you can used the '-gdb' parameter to provide a
GDB server:

     qemu-system-riscv64 -M virt -nographic -gdb tcp::3333 -kernel u-boot

Running a GDB session
----------------------

You need a GDB suited for your target. This can be the GDB coming with your
toolchain or *gdb-multiarch* available in your Linux distribution.

.. prompt:: bash $

    gdb-multiarch u-boot

In the above command-line *u-boot* is the U-boot binary in your build
directory. You may need to adjust the path when calling GDB.

Connect to the GDB server like this:

.. code-block:: console

    (gdb) target extended-remote :3333
    Remote debugging using :3333
    0x27fa9ac6 in ?? ()
    (gdb)

This is fine for debugging before U-Boot relocates itself.

For debugging U-Boot after relocation you need to indicate the relocation
address to GDB. You can retrieve the relocation address from the U-Boot shell
with the command *bdinfo*:

.. code-block:: console

    U-Boot> bdinfo
    boot_params = 0x20000100
    DRAM bank   = 0x00000000
    -> start    = 0x20000000
    -> size     = 0x08000000
    flashstart  = 0x00000000
    flashsize   = 0x00000000
    flashoffset = 0x00000000
    baudrate    = 115200 bps
    relocaddr   = 0x27f7a000
    reloc off   = 0x0607a000
    Build       = 32-bit
    current eth = ethernet@f8008000
    ethaddr     = 00:50:c2:31:58:d4
    IP addr     = <NULL>
    fdt_blob    = 0x27b36060
    new_fdt     = 0x27b36060
    fdt_size    = 0x00003e40
    lmb_dump_all:
     memory.cnt = 0x1 / max = 0x10
     memory[0]      [0x20000000-0x27ffffff], 0x08000000 bytes flags: 0
     reserved.cnt = 0x1 / max = 0x10
     reserved[0]    [0x27b31d00-0x27ffffff], 0x004ce300 bytes flags: 0
    devicetree  = separate
    arch_number = 0x00000000
    TLB addr    = 0x27ff0000
    irq_sp      = 0x27b36050
    sp start    = 0x27b36040
    Early malloc usage: cd8 / 2000

Look out for the line starting with *relocaddr* which has the address
you need, ``0x27f7a000`` in this case.

On most architectures (not sandbox, x86, Xtensa) the global data pointer is
stored in a fixed register:

============ ========
Architecture Register
============ ========
arc          r25
arm          r9
arm64        x18
m68k         d7
microblaze   r31
mips         k0
nios2        gp
powerpc      r2
riscv        gp
sh           r13
============ ========

On these architectures the relocation address can be determined by
dereferencing the global data pointer stored in register, *r9* in the example:

.. code-block:: console

     (gdb) p/x (*(struct global_data*)$r9)->relocaddr
     $1 = 0x27f7a000

In the GDB shell discard the previously loaded symbol file and add it once
again, with the relocation address like this:

.. code-block:: console

    (gdb) symbol-file
    Discard symbol table from `/home/adahl/build/u-boot/v2024.04.x/u-boot'? (y or n) y
    No symbol file now.
    (gdb) add-symbol-file u-boot 0x27f7a000
    add symbol table from file "u-boot" at
            .text_addr = 0x27f7a000
    (y or n) y
    Reading symbols from u-boot...
    (gdb)

You can now use GDB as usual, setting breakpoints, printing backtraces,
inspecting variables, stepping through the code, etc.
