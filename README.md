# u-boot-asn1
## Introduction
The project "u-boot-asn1" serves as a proof of concept for passing Unified Discovery Data to the kernel. It converts the Unified Discovery Data into properties of every FDT /cpus/cpu@n node within U-Boot. During kernel boot, the Unified Discovery Data is handed off to the kernel along with the FDT file.

## Reference
1. https://github.com/riscv/configuration-structure/tree/master - RISC-V Unified Discovery Task Group Repository.
2. https://lionet.info/asn1c/documentation.html - ASN.1 documentation
3. https://lionet.info/asn1c/download.html - Online ASN.1 compiler
4. https://www.kernel.org/doc/Documentation/devicetree/bindings/riscv/extensions.yaml - RISCV-ISA extensions

## Source code
- `cmd/asn2fdt.c` - The core implementation of inserting data structure into FDT file.

## Build
1. Add cmd/asn2fdt.c source file to corrosponding Makefile.
2. Make configurations and build U-Boot image.
3. Burn U-Boot image to the board.

## Run
1. Boot the Light A board. Press any key to enter into U-Boot shell.
2. Print command `asn2fdt` in U-Boot shell as following:
```
SYS_PROMPT# asn2fdt
91039 bytes read in 1 ms (86.8 MiB/s)
```
3. Boot kernel
```
C910 SYS_PROMPT# boot
```
4. when entering kernel, log in 'root' account. Then check the `/cpus/cpu@0` node in modified FDT file:
```
root@light-a-val:~# cd  /sys/firmware/devicetree/base/cpus/cpu@0
root@light-a-val:/sys/firmware/devicetree/base/cpus/cpu@0# ls
 clock-latency     cpu-freq       dvdd-supply                    phandle
 clock-names       cpu-icache     dynamic-power-coefficient      reg
 clocks            cpu-l2cache    interrupt-controller           riscv,isa
 compatible        cpu-tlb        light,dvddm-operating-points   riscv,isa-extension-config
'#cooling-cells'   cpu-vector     mmu-type                       riscv,isa-extensions
 cpu-cacheline     device_type    name                           status
 cpu-dcache        dvddm-supply   operating-points               version

```
However, if not open the asn2 configuration and not execute `asn2fdt`` command, the FDT /cpus/cpu@0 node will look like:
```
> cd  /sys/firmware/devicetree/base/cpus/cpu@0
/sys/firmware/devicetree/base/cpus/cpu@0 > ls
 clock-latency     cpu-cacheline   cpu-tlb        dynamic-power-coefficient      operating-points
 clock-names       cpu-dcache      cpu-vector     interrupt-controller           phandle
 clocks            cpu-freq        device_type    light,dvddm-operating-points   reg
 compatible        cpu-icache      dvddm-supply   mmu-type                       riscv,isa
'#cooling-cells'   cpu-l2cache     dvdd-supply    name                           status

```
It can be observed that some new properties, such as `riscv,isa-extensions` has been newly inserted to every /cpus/cpu@n node.

