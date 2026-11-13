.. SPDX-License-Identifier: GPL-2.0-or-later
.. Copyright (C) 2026 Yao Zi <me@ziyao.cc>

LoongArch
=========

LoongArch is a RISC ISA developed by Loongson Technology. It currently defines
three variants: a reduced 32-bit version (LA32R), a standard 32-bit version
(LA32S) for embedded devices, and a 64-bit version (LA64).

Notes
-----

1. Currently U-Boot only runs on LA64 variant with the highest privilege level
   (PLV0).

2. CONFIG_ARCH_LA64=y selects support for 64-bit LoongArch variant.

3. U-Boot requires hardware-maintained coherency of different levels of
   instruction and data caches. It determines the cache topology by reading
   CPUCFG registers; L2 and L3 caches, if present, must be unified or
   instruction-only. This is true for all commercial LA64 hardware. Refer to
   arch/loongarch/lib/cache.c for further description.

Toolchain
---------

LoongArch is supported in upstream binutils, gcc, and LLVM. GCC 12.1 started to
support LA64, and GCC 16 started to support LA32.
`kernel.org <https://mirrors.edge.kernel.org/pub/tools/crosstool/>`_ provides
pre-built toolchain for both 32-bit and 64-bit LoongArch.
