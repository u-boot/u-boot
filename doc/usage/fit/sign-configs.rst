.. SPDX-License-Identifier: GPL-2.0+

Signed configurations
=====================

::

    /dts-v1/;

    / {
        description = "Chrome OS kernel image with one or more FDT blobs";
        #address-cells = <1>;

        images {
            kernel {
                data = /incbin/("test-kernel.bin");
                type = "kernel_noload";
                arch = "sandbox";
                os = "linux";
                compression = "lzo";
                load = <0x4>;
                entry = <0x8>;
                kernel-version = <1>;
                hash-1 {
                    algo = "sha256";
                };
            };
            fdt-1 {
                description = "snow";
                data = /incbin/("sandbox-kernel.dtb");
                type = "flat_dt";
                arch = "sandbox";
                compression = "none";
                fdt-version = <1>;
                hash-1 {
                    algo = "sha256";
                };
            };
        };
        configurations {
            default = "conf-1";
            conf-1 {
                kernel = "kernel";
                fdt = "fdt-1";
                signature {
                    algo = "sha256,rsa2048";
                    key-name-hint = "dev";
                };
            };
        };
    };

For signed configurations, mkimage signs every image referenced by the
configuration node, such as ``kernel``, ``fdt``, ``ramdisk``, ``firmware`` and
``loadables`` entries. No ``sign-images`` property is required. Older FIT
source files may still include ``sign-images``, but current mkimage and U-Boot
verification do not use it to limit the signed image list. mkimage warns when
the property is present and signs every referenced image.

Every referenced image must have at least one hash subnode. The configuration
signature protects those hash values rather than the image data directly, so
mkimage rejects a signed configuration that references an image without a
hash.
