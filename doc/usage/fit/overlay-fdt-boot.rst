.. SPDX-License-Identifier: GPL-2.0+

U-Boot FDT Overlay FIT usage
============================

Introduction
------------

In many cases it is desirable to have a single FIT image support a multitude
of similar boards and their expansion options. The same kernel on DT enabled
platforms can support this easily enough by providing a DT blob upon boot
that matches the desired configuration.

This document focuses on specifically using overlays as part of a FIT image.
General information regarding overlays including its syntax and building it
can be found in doc/README.fdt-overlays

Configuration without overlays
------------------------------

Take a hypothetical board named 'foo' where there are different supported
revisions, reva and revb. Assume that both board revisions can use add a bar
add-on board, while only the revb board can use a baz add-on board.

Without using overlays the configuration would be as follows for every case::

	/dts-v1/;
	/ {
	    images {
	    kernel {
		data = /incbin/("./zImage");
		type = "kernel";
		arch = "arm";
		os = "linux";
		load = <0x82000000>;
		entry = <0x82000000>;
	    };
	    fdt-1 {
		data = /incbin/("./foo-reva.dtb");
		type = "flat_dt";
		arch = "arm";
	    };
	    fdt-2 {
		data = /incbin/("./foo-revb.dtb");
		type = "flat_dt";
		arch = "arm";
	    };
	    fdt-3 {
		data = /incbin/("./foo-reva-bar.dtb");
		type = "flat_dt";
		arch = "arm";
	    };
	    fdt-4 {
		data = /incbin/("./foo-revb-bar.dtb");
		type = "flat_dt";
		arch = "arm";
	    };
	    fdt-5 {
		data = /incbin/("./foo-revb-baz.dtb");
		type = "flat_dt";
		arch = "arm";
	    };
	    fdt-6 {
		data = /incbin/("./foo-revb-bar-baz.dtb");
		type = "flat_dt";
		arch = "arm";
	    };
	    };

	    configurations {
	    default = "foo-reva.dtb;
	    foo-reva.dtb {
		kernel = "kernel";
		fdt = "fdt-1";
	    };
	    foo-revb.dtb {
		kernel = "kernel";
		fdt = "fdt-2";
	    };
	    foo-reva-bar.dtb {
		kernel = "kernel";
		fdt = "fdt-3";
	    };
	    foo-revb-bar.dtb {
		kernel = "kernel";
		fdt = "fdt-4";
	    };
	    foo-revb-baz.dtb {
		kernel = "kernel";
		fdt = "fdt-5";
	    };
	    foo-revb-bar-baz.dtb {
		kernel = "kernel";
		fdt = "fdt-6";
	    };
	    };
	};

Note the blob needs to be compiled for each case and the combinatorial explosion of
configurations. A typical device tree blob is in the low hunderds of kbytes so a
multitude of configuration grows the image quite a bit.

Booting this image is done by using::

    # bootm <addr>#<config>

Where config is one of::

    foo-reva.dtb, foo-revb.dtb, foo-reva-bar.dtb, foo-revb-bar.dtb,
    foo-revb-baz.dtb, foo-revb-bar-baz.dtb

This selects the DTB to use when booting.

.. _fit_configuration_using_overlays:

Configuration using overlays
----------------------------

Device tree overlays can be applied to a base DT and result in the same blob
being passed to the booting kernel. This saves on space and avoid the combinatorial
explosion problem::

    /dts-v1/;
    / {
        images {
            kernel {
                data = /incbin/("./zImage");
                type = "kernel";
                arch = "arm";
                os = "linux";
                load = <0x82000000>;
                entry = <0x82000000>;
            };
            fdt-1 {
                data = /incbin/("./foo.dtb");
                type = "flat_dt";
                arch = "arm";
                load = <0x87f00000>;
            };
            fdt-2 {
                data = /incbin/("./reva.dtbo");
                type = "flat_dt";
                arch = "arm";
                load = <0x87fc0000>;
            };
            fdt-3 {
                data = /incbin/("./revb.dtbo");
                type = "flat_dt";
                arch = "arm";
                load = <0x87fc0000>;
            };
            fdt-4 {
                data = /incbin/("./bar.dtbo");
                type = "flat_dt";
                arch = "arm";
                load = <0x87fc0000>;
            };
            fdt-5 {
                data = /incbin/("./baz.dtbo");
                type = "flat_dt";
                arch = "arm";
                load = <0x87fc0000>;
            };
        };

        configurations {
            default = "foo-reva.dtb;
            foo-reva.dtb {
                kernel = "kernel";
                fdt = "fdt-1", "fdt-2";
            };
            foo-revb.dtb {
                kernel = "kernel";
                fdt = "fdt-1", "fdt-3";
            };
            foo-reva-bar.dtb {
                kernel = "kernel";
                fdt = "fdt-1", "fdt-2", "fdt-4";
            };
            foo-revb-bar.dtb {
                kernel = "kernel";
                fdt = "fdt-1", "fdt-3", "fdt-4";
            };
            foo-revb-baz.dtb {
                kernel = "kernel";
                fdt = "fdt-1", "fdt-3", "fdt-5";
            };
            foo-revb-bar-baz.dtb {
                kernel = "kernel";
                fdt = "fdt-1", "fdt-3", "fdt-4", "fdt-5";
            };
            bar {
                fdt = "fdt-4";
            };
            baz {
                fdt = "fdt-5";
            };
        };
    };

Booting this image is exactly the same as the non-overlay example.
u-boot will retrieve the base blob and apply the overlays in sequence as
they are declared in the configuration.

Note the minimum amount of different DT blobs, as well as the requirement for
the DT blobs to have a load address; the overlay application requires the blobs
to be writeable.

Configuration using overlays and feature selection
--------------------------------------------------

Although the configuration in the previous section works is a bit inflexible
since it requires all possible configuration options to be laid out before
hand in the FIT image. For the add-on boards the extra config selection method
might make sense.

Note the two bar & baz configuration nodes. To boot a reva board with
the bar add-on board enabled simply use::

    => bootm <addr>#foo-reva.dtb#bar

While booting a revb with bar and baz is as follows::

    => bootm <addr>#foo-revb.dtb#bar#baz

The limitation for a feature selection configuration node is that a single
fdt option is currently supported.

.. sectionauthor:: Pantelis Antoniou <pantelis.antoniou@konsulko.com>, 12/6/2017
