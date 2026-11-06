.. SPDX-License-Identifier: GPL-2.0+

Free Mobile Nodebox v3 CPU Module (nbxv3)
=========================================

The Nodebox v3 CPU Module is an NXP LX2160A based board.
The module plugs onto several carrier boards (NBV30, NBV32, ...) which share
one kernel image but differ in their device tree and DPAA2 data path
configuration.

The U-Boot port is for the CPU module that handles those carriers.
It lives next to the LX2160ARDB/QDS boards under
``board/nxp/lx2160a/`` (``TARGET_NBXV3``, ``configs/nbxv3_tfa_defconfig``)
and reuses their SoC level code.
The board specific behaviour is wired
through ``EVENT_SPY`` hooks in ``board/nxp/lx2160a/nbxv3/`` and the
dependencies each hook needs are forced on by hidden ``default y`` Kconfig
bools under ``if TARGET_NBXV3`` in ``board/nxp/lx2160a/Kconfig``.

LX2160A SoC overview
--------------------

Please refer to doc/board/nxp/layerscape.rst for the
LX2160A SoC overview.

CPU Module overview
-------------------

- Boot: TF-A (BL2 trains the DDR, BL31 hands over) then U-Boot as the
  BL33 payload, from a single FlexSPI NOR.
- DDR: two DDR4 SODIMMs with JC42 SPD temperature sensors.
- Storage: on module eMMC on eSDHC2.
- Ethernet: DPMAC17 through RGMII1 to a Realtek RTL8211FD-CG
  (management port, ``lanconsole0``), the LX2160A management complex
  (MC) drives the DPAA2 data path.
- PCIe: PCIE3..PCIE6 root complexes routed to the carrier connector.
- USB: xHCI host controllers.
- I2C: IIC1 on module management bus, IIC2..IIC4 and IIC6 to the carrier
- SPI: DSPI1 on module management SPI (ZL30733 DPLL), DSPI3 to the
  carrier.
- Power: MPS MPQ8785 PMBus regulator on the +0V8_VDD core rail, per die
  VID fuse voltage trim.
- Watchdog: SBSA generic watchdog, updated to 120 s timeout default.

Boot NOR layout
---------------

The layout is shared with TF-A and the flash assembler:

============ ==========================================
Offset       Content
============ ==========================================
0x000000     PBL (RCW + PBI + XIP BL2)
0x100000     FIP (BL31 + BL33 U-Boot)
0x500000     U-Boot environment
0x800000     DDR PHY firmware FIP
0x1000000    Kernel FIT
0x2000000    free
============ ==========================================

The environment is stored in the boot NOR.
SPI bus aliases at the prompt:
``spi0`` = DSPI1
``spi2`` = DSPI3
``spi3`` = FlexSPI (boot NOR)
so the boot NOR is always ``sf probe 3:0``.

Kernel FIT
----------

The kernel and the LX2160A management complex artefacts ship as a single
FIT image at NOR offset 0x1000000::

  /images/kernel        Linux Image (zstd compressed)
  /images/fdt-<carrier> Linux DTB, one per carrier
  /images/ramdisk       initramfs cpio (optional)
  /images/mc            MC firmware blob        (loaded at 0xa8000000)
  /images/dpc-<carrier> Data Path Container     (loaded at 0xa9000000)
  /images/dpl-<carrier> Data Path Layout        (loaded at 0xaa000000)
  /configurations/conf-<carrier>

``${carrier}`` selects the carrier set (``conf-<carrier>``, ``dpc-``,
``dpl-``). It is set on every boot, from the first of:

1. the environment. For debug, to enforce another carrier::

     => setenv carrier nbv32 && saveenv && reset

2. the NOR overlay, imported by ``${cfg_import}`` when the environment
   defines it, for boards without GPIO or EEPROM identification,

3. the RCW build stamp, see below,

4. ``nbv30``, the built-in default.

NOR overlay
^^^^^^^^^^^

The NOR overlay is an environment image of ``${cfg_nor_size}`` bytes
with ``carrier=<name>``.
It is outside the environment slot, so wiping the
saved environment keeps the board identity.

``${cfg_import}``, ``${cfg_nor_offset}`` and ``${cfg_nor_size}`` are
defined by the board's default environment
(CONFIG_ENV_USE_DEFAULT_ENV_TEXT_FILE + CONFIG_ENV_DEFAULT_ENV_TEXT_FILE),
or by a saved one.

::

  cfg_nor_offset=0x520000
  cfg_nor_size=0x1000
  cfg_import=sf probe 3:0 && \
    sf read ${loadaddr} ${cfg_nor_offset} ${cfg_nor_size} && \
    env import -c ${loadaddr} ${cfg_nor_size}

RCW build stamp
^^^^^^^^^^^^^^^

The RCW sets the SerDes protocols and the pinmux.
Its PBI writes the carrier it was built for into
DCFG SCRATCHRW13 (0x01e00230)::

   31   28 27   24 23   16 15    8 7     0
  +-------+-------+-------+-------+-------+
  |CARRIER|  TAG  |  YY   |  MM   |  DD   |
  +-------+-------+-------+-------+-------+

``TAG`` is 0xB; any other value, such as the reset value 0, means no
stamp. ``YY MM DD`` is the RCW build date in BCD, for the operator only.

======= ==================================================
CARRIER ``${carrier}``
======= ==================================================
0       none: one RCW for every carrier, it names none
1       ``nbv30`` (NBV30, NBV31)
2       ``nbv32``
3       ``nbv32-40g``: NBV32 with SerDes 1 protocol 20 (2x 40GE)
4..E    unassigned, treated like 0
F       none: bring-up RCW, see below
======= ==================================================

The bring-up RCW (CARRIER F) leaves every SerDes without IO, so there is
no data path for the MC to manage and starting it hangs the boot.
U-Boot clears ``mcinitcmd`` for that boot, whatever ``${carrier}`` is,
and does not save it.

SerDes 1 per carrier
^^^^^^^^^^^^^^^^^^^^

Before the MC starts, U-Boot should set the serdes 1 lane grouping
that the MC's DPC and the DPL of ``${carrier}`` expect, using
the ``serdes1`` hwconfig option. It can be used to
override the RCW's protocol grouping for SerDes 1.

==================== ================= ==============================
``${carrier}``       serdes1 mode      SerDes 1
==================== ================= ==============================
nbv30                (none)            as built by the RCW
nbv32                100g4x25g         1x 100GE + 4x 25G
nbv32-40g            2x40g             2x 40GE
nbv32-40g-4x10g      40g4x10g          1x 40GE + 4x 10G
nbv32-100g-4x10g     100g4x10g         1x 100GE + 4x 10G
==================== ================= ==============================

Example to switch an NBV32 to 1x40GE + 4x10G::

  => setenv carrier nbv32-40g-4x10g && saveenv && reset

Boot flow
---------

It follows the same than any lx2160 board.

``mcinitcmd`` runs automatically at ``initr_net`` through the shared
``mc_env_boot()`` hook: it reads the FIT from the boot NOR, extracts MC,
DPC and DPL to DDR, starts the MC and queues the DPL with
``fsl_mc lazyapply``. By the time U-Boot prints its
prompt the DPMACs are live and ``ldpaa_eth`` has bound ``lanconsole0``, so
``ping`` / ``dhcp`` / ``tftp`` work immediately. At ``bootm`` time the
queued DPL is applied and the kernel boots with the full topology.

``bootcmd`` is ``run mc_init || run provision_openocd_fit``: a module
whose FIT slot is blank or corrupted falls through to the semihosting
provisioning path.

Environment variables
---------------------

========================== ==================================================
Variable                   Purpose
========================== ==================================================
mcinitcmd                  Load the FIT, start the MC, queue the DPL
                           (auto-run at initr_net). Set it empty
                           (``setenv mcinitcmd``) and ``saveenv`` to opt out
                           of the automatic MC bring-up, e.g. while the FIT
                           slot is intentionally blank.
mc_init                    ``bootm ${kernel_addr_r}#conf-${carrier}`` of the
                           FIT already loaded by ``mcinitcmd``.
linux_boot_fit             Read the FIT from NOR and boot it; assumes the
                           MC is already running.
host_boot                  Load the FIT over ARM semihosting (JTAG probe)
                           and boot it. Mainly for bench iteration.
provision_openocd_fit      Load the FIT over semihosting, erase the FIT
                           slot, write it, reset. Recovers a module with a
                           blank or corrupted FIT slot.
xspi_bootcmd               Copy the FIT from the AHB mapped FlexSPI window
                           (0x21000000) with ``cp.b`` and boot it. Fallback
                           when the ``sf`` driver state is suspect.
mcmemsize                  MC private RAM carve out at the top of DDR,
                           0x70000000 (1.75 GiB).
kernel_addr_r              DDR load address of the FIT (0xc0000000).
fit_nor_offset             Offset (0x1000000) and size cap (0x1000000, 16 MiB)
fit_nor_size               of the kernel FIT slot in the boot NOR, used by
                           every macro that reads or writes the FIT.
carrier                    Carrier selection, see above.
ethprime                   ``lanconsole0`` (CONFIG_ETHPRIME): the RGMII
                           management port is the default device for
                           ``dhcp`` / ``tftp`` / ``ping``.
nbxv3_vdd_mv               Millivolt integer in [600, 1100]: bypass the
                           VID fuse and pin the +0V8_VDD core rail at that
                           voltage. Unset = follow the fuse.
pci_iommu_extra            Hotplug entries for the PCIe endpoints
                           (1.0.0 behind each root complex)
mdio_list                  ``mdio list`` with a banner, run from
                           ``preboot``, so the boot log shows which PHY
                           drivers bound.
PS1 / ps_refresh           Interactive prompt (``nbxv3> ``); ``ps_refresh``
                           rebakes PS1 from ``${ethact}`` and ``${ipaddr}``
                           (run from ``preboot``, and by hand after ``dhcp``).
preboot                    ``run mdio_list ; run ps_refresh``.
========================== ==================================================

Commands
--------

Board specific:

- ``dpll_info``: re-run the DPLL identification probe (ZL30733 on
  ``spi0:0``, ZL30643 on ``spi2:1``); a DPLL whose bus is not described
  is reported "not reachable".

Tree commands the board relies on:

- ``pmbus telemetry`` / ``pmbus status`` / ``pmbus mps last``: the MPQ8785
  core regulator is pre-selected at boot, no ``pmbus dev`` needed.
- ``regulator list`` / ``regulator info +0V8_VDD`` / ``regulator value``
- ``temperature list`` / ``temperature get``
- ``i2c bus`` / ``i2c probe``
- ``mdio list`` / ``mii info``
- ``gpio status``
- ``sf`` (boot NOR, ``sf probe 3:0``), ``mmc`` (eMMC), ``pci``, ``usb``,
  ``eeprom`` (board ID EEPROM at 0x52), ``wdt``, ``rng``, ``dm tree``.

Boot log
--------

The board hooks print one banner each before the prompt::

  Carrier:       nbv30 (default, not saved - `setenv carrier <name> ...)
  MPQ8785 @ i2c0:0x10  MFR_ID="MPS" ...
    +0V8_VDD: VOUT=... uV  enabled=1
  MPQ8785 (+0V8_VDD): VID handoff fuse=0x.. target=... mV ...
  DPLL: ZL30733 @ spi0:0  info=0xa1 (EEPROM boot) id=0x0e95 rev=0x03
  DPLL: ZL30643 spi2:1 not reachable (...)
  === Nbxv3 probed PHYs ===
  ...
  nbxv3 lanconsole0 192.168.x.y>

Building
--------

.. code-block:: bash

  $ make nbxv3_tfa_defconfig
  $ make CROSS_COMPILE=aarch64-linux-gnu-

``u-boot.bin`` is the BL33 payload packaged in the TF-A FIP at NOR
offset 0x100000.

When ``${carrier}`` is not in the environment the board falls back to the
compile time ``NBXV3_CARRIER_DEFAULT``, which defaults to ``nbv30``. To
build a U-Boot whose fallback is the NBV32 carrier instead::

  $ make CROSS_COMPILE=aarch64-linux-gnu- \
         KCPPFLAGS='-DNBXV3_CARRIER_DEFAULT=\"nbv32\"'

Provisioning a blank module over JTAG
-------------------------------------

With a debugger attached and semihosting enabled on the host, a module
with an empty FIT slot boots to the prompt and ``bootcmd`` runs
``provision_openocd_fit``, which loads ``fit.itb`` from the host
directory over semihosting, writes it to NOR offset 0x1000000 and
resets. ``host_boot`` boots a FIT from the host without writing it.
