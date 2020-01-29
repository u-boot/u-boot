.. SPDX-License-Identifier: GPL-2.0+

NDS32
=====

NDS32 is a new high-performance 32-bit RISC microprocessor core.

http://www.andestech.com/

AndeStar ISA
------------
AndeStar is a patent-pending 16-bit/32-bit mixed-length instruction set to
achieve optimal system performance, code density, and power efficiency.

It contains the following features:
 - Intermixable 32-bit and 16-bit instruction sets without the need for
   mode switch.
 - 16-bit instructions as a frequently used subset of 32-bit instructions.
 - RISC-style register-based instruction set.
 - 32 32-bit General Purpose Registers (GPR).
 - Upto 1024 User Special Registers (USR) for existing and extension
   instructions.
 - Rich load/store instructions for...
      - Single memory access with base address update.
      - Multiple aligned and unaligned memory accesses for memory copy and stack
        operations.
      - Data prefetch to improve data cache performance.
      - Non-bus locking synchronization instructions.
 - PC relative jump and PC read instructions for efficient position independent
   code.
 - Multiply-add and multiple-sub with 64-bit accumulator.
 - Instruction for efficient power management.
 - Bi-endian support.
 - Three instruction extension space for application acceleration:
      - Performance extension.
      - Andes future extensions (for floating-point, multimedia, etc.)
      - Customer extensions.

AndesCore CPU
-------------
Andes Technology has 4 families of CPU cores: N12, N10, N9, N8.

For details about N12 CPU family, please check below N1213 features.
N1213 is a configurable hard/soft core of NDS32's N12 CPU family.

N1213 Features
^^^^^^^^^^^^^^

CPU Core
 - 16-/32-bit mixable instruction format.
 - 32 general-purpose 32-bit registers.
 - 8-stage pipeline.
 - Dynamic branch prediction.
 - 32/64/128/256 BTB.
 - Return address stack (RAS).
 - Vector interrupts for internal/external.
   interrupt controller with 6 hardware interrupt signals.
 - 3 HW-level nested interruptions.
 - User and super-user mode support.
 - Memory-mapped I/O.
 - Address space up to 4GB.

Memory Management Unit
 - TLB
      - 4/8-entry fully associative iTLB/dTLB.
      - 32/64/128-entry 4-way set-associati.ve main TLB.
      - TLB locking support
 - Optional hardware page table walker.
 - Two groups of page size support.
     - 4KB & 1MB.
     - 8KB & 1MB.

Memory Subsystem
 - I & D cache.
      - Virtually indexed and physically tagged.
      - Cache size: 8KB/16KB/32KB/64KB.
      - Cache line size: 16B/32B.
      - Set associativity: 2-way, 4-way or direct-mapped.
      - Cache locking support.
 - I & D local memory (LM).
      - Size: 4KB to 1MB.
      - Bank numbers: 1 or 2.
      - Optional 1D/2D DMA engine.
      - Internal or external to CPU core.

Bus Interface
 - Synchronous/Asynchronous AHB bus: 0, 1 or 2 ports.
 - Synchronous High speed memory port.
   (HSMP): 0, 1 or 2 ports.

Debug
 - JTAG debug interface.
 - Embedded debug module (EDM).
 - Optional embedded program tracer interface.

Miscellaneous
 - Programmable data endian control.
 - Performance monitoring mechanism.

The NDS32 ports of u-boot, the Linux kernel, the GNU toolchain and other
associated software are actively supported by Andes Technology Corporation.
