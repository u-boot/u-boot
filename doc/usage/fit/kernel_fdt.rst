.. SPDX-License-Identifier: GPL-2.0+

Single kernel and FDT blob
==========================

::

    /dts-v1/;

    / {
        description = "Simple image with single Linux kernel and FDT blob";
        #address-cells = <1>;

        images {
            kernel {
                description = "Vanilla Linux kernel";
                data = /incbin/("./vmlinux.bin.gz");
                type = "kernel";
                arch = "ppc";
                os = "linux";
                compression = "gzip";
                load = <00000000>;
                entry = <00000000>;
                hash-1 {
                    algo = "crc32";
                };
                hash-2 {
                    algo = "sha256";
                };
            };
            fdt-1 {
                description = "Flattened Device Tree blob";
                data = /incbin/("./target.dtb");
                type = "flat_dt";
                arch = "ppc";
                compression = "none";
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
            };
        };
    };
