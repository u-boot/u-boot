.. SPDX-License-Identifier: GPL-2.0+

Compiled-in Device Tree / Platform Data
=======================================


Introduction
------------

Device tree is the standard configuration method in U-Boot. It is used to
define what devices are in the system and provide configuration information
to these devices.

The overhead of adding devicetree access to U-Boot is fairly modest,
approximately 3KB on Thumb 2 (plus the size of the DT itself). This means
that in most cases it is best to use devicetree for configuration.

However there are some very constrained environments where U-Boot needs to
work. These include SPL with severe memory limitations. For example, some
SoCs require a 16KB SPL image which must include a full MMC stack. In this
case the overhead of devicetree access may be too great.

It is possible to create platform data manually by defining C structures
for it, and reference that data in a `U_BOOT_DRVINFO()` declaration. This
bypasses the use of devicetree completely, effectively creating a parallel
configuration mechanism. But it is an available option for SPL.

As an alternative, the 'of-platdata' feature is provided. This converts the
devicetree contents into C code which can be compiled into the SPL binary.
This saves the 3KB of code overhead and perhaps a few hundred more bytes due
to more efficient storage of the data.


How it works
------------

The feature is enabled by CONFIG OF_PLATDATA. This is only available in
SPL/TPL and should be tested with:

.. code-block:: c

    #if CONFIG_IS_ENABLED(OF_PLATDATA)

A tool called 'dtoc' converts a devicetree file either into a set of
struct declarations, one for each compatible node, and a set of
`U_BOOT_DRVINFO()` declarations along with the actual platform data for each
device. As an example, consider this MMC node:

.. code-block:: none

    sdmmc: dwmmc@ff0c0000 {
            compatible = "rockchip,rk3288-dw-mshc";
            clock-freq-min-max = <400000 150000000>;
            clocks = <&cru HCLK_SDMMC>, <&cru SCLK_SDMMC>,
                     <&cru SCLK_SDMMC_DRV>, <&cru SCLK_SDMMC_SAMPLE>;
            clock-names = "biu", "ciu", "ciu_drv", "ciu_sample";
            fifo-depth = <0x100>;
            interrupts = <GIC_SPI 32 IRQ_TYPE_LEVEL_HIGH>;
            reg = <0xff0c0000 0x4000>;
            bus-width = <4>;
            cap-mmc-highspeed;
            cap-sd-highspeed;
            card-detect-delay = <200>;
            disable-wp;
            num-slots = <1>;
            pinctrl-names = "default";
            pinctrl-0 = <&sdmmc_clk>, <&sdmmc_cmd>, <&sdmmc_cd>, <&sdmmc_bus4>;
                vmmc-supply = <&vcc_sd>;
                status = "okay";
                u-boot,dm-pre-reloc;
        };


Some of these properties are dropped by U-Boot under control of the
CONFIG_OF_SPL_REMOVE_PROPS option. The rest are processed. This will produce
the following C struct declaration:

.. code-block:: c

    struct dtd_rockchip_rk3288_dw_mshc {
            fdt32_t         bus_width;
            bool            cap_mmc_highspeed;
            bool            cap_sd_highspeed;
            fdt32_t         card_detect_delay;
            fdt32_t         clock_freq_min_max[2];
            struct phandle_1_arg clocks[4];
            bool            disable_wp;
            fdt32_t         fifo_depth;
            fdt32_t         interrupts[3];
            fdt32_t         num_slots;
            fdt32_t         reg[2];
            fdt32_t         vmmc_supply;
    };

and the following device declarations:

.. code-block:: c

    /* Node /clock-controller@ff760000 index 0 */
    ...

    /* Node /dwmmc@ff0c0000 index 2 */
    static struct dtd_rockchip_rk3288_dw_mshc dtv_dwmmc_at_ff0c0000 = {
            .fifo_depth             = 0x100,
            .cap_sd_highspeed       = true,
            .interrupts             = {0x0, 0x20, 0x4},
            .clock_freq_min_max     = {0x61a80, 0x8f0d180},
            .vmmc_supply            = 0xb,
            .num_slots              = 0x1,
            .clocks                 = {{0, 456},
                                       {0, 68},
                                       {0, 114},
                                       {0, 118}},
            .cap_mmc_highspeed      = true,
            .disable_wp             = true,
            .bus_width              = 0x4,
            .u_boot_dm_pre_reloc    = true,
            .reg                    = {0xff0c0000, 0x4000},
            .card_detect_delay      = 0xc8,
    };

    U_BOOT_DRVINFO(dwmmc_at_ff0c0000) = {
            .name           = "rockchip_rk3288_dw_mshc",
            .plat       = &dtv_dwmmc_at_ff0c0000,
            .plat_size  = sizeof(dtv_dwmmc_at_ff0c0000),
            .parent_idx     = -1,
    };

The device is then instantiated at run-time and the platform data can be
accessed using:

.. code-block:: c

    struct udevice *dev;
    struct dtd_rockchip_rk3288_dw_mshc *plat = dev_get_plat(dev);

This avoids the code overhead of converting the devicetree data to
platform data in the driver. The `of_to_plat()` method should
therefore do nothing in such a driver.

Note that for the platform data to be matched with a driver, the 'name'
property of the `U_BOOT_DRVINFO()` declaration has to match a driver declared
via `U_BOOT_DRIVER()`. This effectively means that a `U_BOOT_DRIVER()` with a
'name' corresponding to the devicetree 'compatible' string (after converting
it to a valid name for C) is needed, so a dedicated driver is required for
each 'compatible' string.

In order to make this a bit more flexible, the `DM_DRIVER_ALIAS()` macro can be
used to declare an alias for a driver name, typically a 'compatible' string.
This macro produces no code, but is used by dtoc tool. It must be located in the
same file as its associated driver, ideally just after it.

The parent_idx is the index of the parent `driver_info` structure within its
linker list (instantiated by the `U_BOOT_DRVINFO()` macro). This is used to
support `dev_get_parent()`.

During the build process dtoc parses both `U_BOOT_DRIVER()` and
`DM_DRIVER_ALIAS()` to build a list of valid driver names and driver aliases.
If the 'compatible' string used for a device does not not match a valid driver
name, it will be checked against the list of driver aliases in order to get the
right driver name to use. If in this step there is no match found a warning is
issued to avoid run-time failures.

Where a node has multiple compatible strings, dtoc generates a `#define` to
make them equivalent, e.g.:

.. code-block:: c

    #define dtd_rockchip_rk3299_dw_mshc dtd_rockchip_rk3288_dw_mshc


Converting of-platdata to a useful form
---------------------------------------

Of course it would be possible to use the of-platdata directly in your driver
whenever configuration information is required. However this means that the
driver will not be able to support devicetree, since the of-platdata
structure is not available when devicetree is used. It would make no sense
to use this structure if devicetree were available, since the structure has
all the limitations metioned in caveats below.

Therefore it is recommended that the of-platdata structure should be used
only in the `probe()` method of your driver. It cannot be used in the
`of_to_plat()` method since this is not called when platform data is
already present.


How to structure your driver
----------------------------

Drivers should always support devicetree as an option. The of-platdata
feature is intended as a add-on to existing drivers.

Your driver should convert the plat struct in its `probe()` method. The
existing devicetree decoding logic should be kept in the
`of_to_plat()` method and wrapped with `#if`.

For example:

.. code-block:: c

    #include <dt-structs.h>

    struct mmc_plat {
    #if CONFIG_IS_ENABLED(OF_PLATDATA)
            /* Put this first since driver model will copy the data here */
            struct dtd_mmc dtplat;
    #endif
            /*
             * Other fields can go here, to be filled in by decoding from
             * the devicetree (or the C structures when of-platdata is used).
             */
            int fifo_depth;
    };

    static int mmc_of_to_plat(struct udevice *dev)
    {
    #if !CONFIG_IS_ENABLED(OF_PLATDATA)
            /* Decode the devicetree data */
            struct mmc_plat *plat = dev_get_plat(dev);
            const void *blob = gd->fdt_blob;
            int node = dev_of_offset(dev);

            plat->fifo_depth = fdtdec_get_int(blob, node, "fifo-depth", 0);
    #endif

            return 0;
    }

    static int mmc_probe(struct udevice *dev)
    {
            struct mmc_plat *plat = dev_get_plat(dev);

    #if CONFIG_IS_ENABLED(OF_PLATDATA)
            /* Decode the of-platdata from the C structures */
            struct dtd_mmc *dtplat = &plat->dtplat;

            plat->fifo_depth = dtplat->fifo_depth;
    #endif
            /* Set up the device from the plat data */
            writel(plat->fifo_depth, ...)
    }

    static const struct udevice_id mmc_ids[] = {
            { .compatible = "vendor,mmc" },
            { }
    };

    U_BOOT_DRIVER(mmc_drv) = {
            .name           = "mmc_drv",
            .id             = UCLASS_MMC,
            .of_match       = mmc_ids,
            .of_to_plat = mmc_of_to_plat,
            .probe          = mmc_probe,
            .priv_auto = sizeof(struct mmc_priv),
            .plat_auto = sizeof(struct mmc_plat),
    };

    DM_DRIVER_ALIAS(mmc_drv, vendor_mmc) /* matches compatible string */

Note that `struct mmc_plat` is defined in the C file, not in a header. This
is to avoid needing to include dt-structs.h in a header file. The idea is to
keep the use of each of-platdata struct to the smallest possible code area.
There is just one driver C file for each struct, that can convert from the
of-platdata struct to the standard one used by the driver.

In the case where SPL_OF_PLATDATA is enabled, `plat_auto` is
still used to allocate space for the platform data. This is different from
the normal behaviour and is triggered by the use of of-platdata (strictly
speaking it is a non-zero `plat_size` which triggers this).

The of-platdata struct contents is copied from the C structure data to the
start of the newly allocated area. In the case where devicetree is used,
the platform data is allocated, and starts zeroed. In this case the
`of_to_plat()` method should still set up the platform data (and the
of-platdata struct will not be present).

SPL must use either of-platdata or devicetree. Drivers cannot use both at
the same time, but they must support devicetree. Supporting of-platdata is
optional.

The devicetree becomes inaccessible when CONFIG_SPL_OF_PLATDATA is enabled,
since the devicetree access code is not compiled in. A corollary is that
a board can only move to using of-platdata if all the drivers it uses support
it. There would be little point in having some drivers require the device
tree data, since then libfdt would still be needed for those drivers and
there would be no code-size benefit.

Internals
---------

The dt-structs.h file includes the generated file
`(include/generated/dt-structs.h`) if CONFIG_SPL_OF_PLATDATA is enabled.
Otherwise (such as in U-Boot proper) these structs are not available. This
prevents them being used inadvertently. All usage must be bracketed with
`#if CONFIG_IS_ENABLED(OF_PLATDATA)`.

The dt-plat.c file contains the device declarations and is is built in
spl/dt-plat.c.

The dm_populate_phandle_data() function that was previous needed has now been
removed, since dtoc can address the drivers directly from dt-plat.c and does
not need to fix up things at runtime.

The pylibfdt Python module is used to access the devicetree.


Credits
-------

This is an implementation of an idea by Tom Rini <trini@konsulko.com>.


Future work
-----------
- Consider programmatically reading binding files instead of devicetree
  contents
- Allow IS_ENABLED() to be used in the C code instead of #if


.. Simon Glass <sjg@chromium.org>
.. Google, Inc
.. 6/6/16
.. Updated Independence Day 2016
.. Updated 1st October 2020
.. Updated 5th February 2021
