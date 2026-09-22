.. SPDX-License-Identifier: GPL-2.0+

Multiple configurations sharing one payload (image-data)
========================================================

A FIT that supports several boards often needs to offer the same binary
at different load addresses, for example one kernel that board A loads
at one address and board B at another. Each load address needs its own
image node, and without further help each node carries its own copy of
the binary, doubling the size of the FIT.

The 'image-data' property avoids the duplication. An image node names
another image node under /images whose binary data it shares, while
keeping its own load, entry and remaining metadata::

    /dts-v1/;

    / {
        description = "Kernel shared between two boards";
        #address-cells = <1>;

        images {
            kernel-1 {
                description = "Linux kernel";
                data = /incbin/("./Image");
                type = "kernel";
                arch = "arm64";
                os = "linux";
                compression = "none";
                load = <0x40200000>;
                entry = <0x40200000>;
                hash-1 {
                    algo = "sha256";
                };
            };

            kernel-board-b {
                description = "Linux kernel at the board B address";
                image-data = "kernel-1";
                type = "kernel";
                arch = "arm64";
                os = "linux";
                compression = "none";
                load = <0x80200000>;
                entry = <0x80200000>;
                hash-1 {
                    algo = "sha256";
                };
            };

            fdt-board-a {
                description = "Board A device tree";
                data = /incbin/("./board-a.dtb");
                type = "flat_dt";
                arch = "arm64";
                compression = "none";
                hash-1 {
                    algo = "sha256";
                };
            };

            fdt-board-b {
                description = "Board B device tree";
                data = /incbin/("./board-b.dtb");
                type = "flat_dt";
                arch = "arm64";
                compression = "none";
                hash-1 {
                    algo = "sha256";
                };
            };
        };

        configurations {
            default = "conf-board-a";

            conf-board-a {
                kernel = "kernel-1";
                fdt = "fdt-board-a";
            };

            conf-board-b {
                kernel = "kernel-board-b";
                fdt = "fdt-board-b";
            };
        };
    };

The sharing node does not include a 'data' property; only the binary
payload is shared and no other property is inherited, so type, arch and
the other fields must be spelled out. The target of the reference must
not itself carry 'image-data'; chains of references are not permitted.

mkimage resolves the reference while building the FIT. With embedded
data the payload is copied into both nodes, so sharing pays off
together with external data (-E or -p), where both nodes point at a
single copy of the payload through identical data-offset or
data-position and data-size values. The 'image-data' property is
retained in the resolved FIT as a record of the relationship, and
re-processing such a FIT with 'mkimage -F' keeps it intact. Each node
carries its own hash and takes part in configuration signing like any
other image, so verified boot needs no special handling.

Encrypted images can be shared as well, with one restriction: both
nodes must describe the same ciphertext, meaning the same algorithm,
the same key-name-hint and an explicitly shared IV, either the same
iv-name-hint or, when no hint is used, the same iv value in both cipher
nodes. mkimage rejects a combination that would produce different
ciphertexts, such as leaving both nodes without an IV so that each
would receive an independent random one.

The full description of the property lives in the 'Shared image data'
section of the Flat Image Tree specification,
https://github.com/open-source-firmware/flat-image-tree.
