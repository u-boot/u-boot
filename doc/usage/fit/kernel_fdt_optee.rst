.. SPDX-License-Identifier: GPL-2.0+

Single kernel, FDT blob and OPTEE-OS
====================================

Example FIT image description file demonstrating the usage of the
bootm command to launch OPTEE-OS before starting Linux kernel on
STM32MP13xx.

::

    /dts-v1/;

    / {
        description = "Simple image with single Linux kernel and FDT blob";
        #address-cells = <1>;

        images {
            kernel {
                description = "Vanilla Linux kernel";
                data = /incbin/("./arch/arm/boot/zImage");
                type = "kernel";
                arch = "arm";
                os = "linux";
                compression = "none";
                load = <0xc0008000>;
                entry = <0xc0008000>;
                hash-1 {
                    algo = "crc32";
                };
                hash-2 {
                    algo = "sha256";
                };
            };
            fdt-1 {
                description = "Flattened Device Tree blob";
                data = /incbin/("./arch/arm/boot/dts/st/stm32mp135f-dhcor-dhsbc.dtb");
                type = "flat_dt";
                arch = "arm";
                compression = "none";
                hash-1 {
                    algo = "crc32";
                };
                hash-2 {
                    algo = "sha256";
                };
            };
            /* Bundled OPTEE-OS */
            tee-1 {
                description = "OP-TEE";
                data = /incbin/("/path/to/optee_os/out/arm-plat-stm32mp1/core/tee-raw.bin");
                type = "tee";
                arch = "arm";
                compression = "none";
                os = "tee";
                load = <0xde000000>;
                entry = <0xde000000>;
                hash-1 {
                    algo = "crc32";
                };
                hash-2 {
                    algo = "sha256";
                };
            };
        };

        configurations {
            default = "conf-1";
            conf-1 {
                description = "Boot Linux kernel with FDT blob";
                kernel = "kernel";
                fdt = "fdt-1";
                loadables = "tee-1"; /* OPTEE-OS */
            };
        };
    };
