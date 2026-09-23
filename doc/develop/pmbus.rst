.. SPDX-License-Identifier: GPL-2.0+

PMBus support in U-Boot
=======================

This document describes U-Boot's PMBus 1.x support: what it is for,
how it is structured, and how to add support for a new PMBus chipn
either from scratch from a chip datasheet or by porting an existing
Linux ``drivers/hwmon/pmbus/`` driver.

.. contents::
   :local:
   :depth: 2

Intent and scope
----------------

U-Boot's PMBus layer is not a hardware monitoring (hwmon) clone of
the Linux kernel's ``drivers/hwmon/pmbus/`` subsystem. Linux owns the
runtime side: continuous polling, sysfs publication, alert IRQ
handling, fan control loops. U-Boot owns the boot time side. Concretely
the U-Boot PMBus support exists to:

* Identify the PMBus regulator(s) a board carries at boot:
  ``MFR_ID``, ``MFR_MODEL``, ``MFR_REVISION`` reads, plus a quick
  ``STATUS_WORD`` sanity check.
* Print telemetry so an operator can confirm rail voltages, input
  current, and temperature before handing off to the kernel. One shot
  reads, on demand, via the ``pmbus`` and ``regulator`` U-Boot
  commands (``pmbus dev <name>; pmbus telemetry``,
  ``regulator dev <name>; regulator value``).
* Decode chip alerts when a rail trips an over/under voltage,
  over current, or thermal threshold; so a boot log shows why the
  previous boot failed, before the kernel even comes up.
* Optionally trim a critical rail (typically the SoC core) before
  the kernel takes over; "set the voltage prior to a kernel boot
  to better protect the board". This is the existing
  ``board/nxp/common/vid.c`` AVS path and any future per board
  speed binning trim.

Out of scope, by design:

* No periodic polling. No worker thread. No background updates.
* No sysfs / procfs / userspace surface. U-Boot has none.
* No fan speed control loop. The kernel runs that.
* No long tail of virtual sensor registers (``PMBUS_VIRT_*``).
* No sensor caching / update timestamps.

If you find yourself wanting any of those, the answer is "wait until
Linux comes up". Keep U-Boot's PMBus surface minimal.

Architecture overview
---------------------

The framework is split into four layers (layer 3 comes in two
flavours, 3a and 3b),

::

    +----------------------------------------+
    |     Layer 1: include/pmbus.h           |
    |     Standard PMBus 1.x command codes,  |
    |     numeric format enum, sensor class  |
    |     enum, struct pmbus_driver_info,    |
    |     decoder + transport prototypes,    |
    |     STATUS_WORD bit names.             |
    +----------------------------------------+
    |     Layer 2: lib/pmbus.c               |
    |     Format decoders (LINEAR11/LINEAR16/|
    |     DIRECT) and encoder (LINEAR16),    |
    |     two stage SMBus block read helper, |
    |     STATUS_*-bit print tables, generic |
    |     dispatcher pmbus_reg2data().       |
    +----------------------------------------+
    |     Layer 3a: drivers/power/regulator/ |
    |               <chip>.c                 |
    |     UCLASS_REGULATOR per chip drivers  |
    |    ; one struct pmbus_driver_info    |
    |     plus regulator set_value/get_value |
    |     ops. Optional: per chip identify() |
    |     hook to refine format from the     |
    |     chip's own VOUT_MODE.              |
    +----------------------------------------+
    |     Layer 3b: drivers/power/regulator/ |
    |               pmbus_generic.c          |
    |     Catch all driver matching          |
    |     compatible = "pmbus".              |
    |     Auto detects format via VOUT_MODE  |
    |     and PMBUS_QUERY where supported.   |
    |     Use for compliant chips with no    |
    |     per chip driver yet; ship        |
    |     telemetry today, write a per chip  |
    |     driver later only if quirks demand |
    |     it.                                |
    +----------------------------------------+
    |     Layer 4: board/<vendor>/<board>/   |
    |              <chip>_diag.c             |
    |     Diagnostic commands only:          |
    |       <chip>_info / <chip>_raw         |
    |     Reads via regulator_get_value()    |
    |     and lib/pmbus.c helpers. LINEAR /  |
    |     DIRECT math NOT here.              |
    +----------------------------------------+

Generic vs. board specific separation rule. Layer 1, 2, and 3
files are tree level and platform agnostic. Their comments may
reference only:

* the PMBus 1.x specification, and
* chip manufacturer datasheets.

Never a specific board, SoC, or product. Board-specific quirks
(a particular bus number, a particular slave address, a particular
PCB feedback divider, board local design notes) live exclusively in
``board/<vendor>/<board>/`` files.

CLI commands
------------

The framework publishes one top level command, ``pmbus``, plus a
vendor namespace dispatcher so per chip code can register chip
specific extensions without touching the framework.

Active device model
~~~~~~~~~~~~~~~~~~~

``pmbus`` mirrors the ``regulator`` command: select an active device
once, then operate on it across subcommands. The active device is
selected by I2C bus (decimal sequence number) and 7 bit address (hex,
``0x`` optional, à la ``i2c`` convention)::

    => pmbus dev 0:10
    pmbus: active i2c0:0x10  MFR_ID="MPS"  MFR_MODEL="MPQ8785"  vendor=mps

The framework probes ``MFR_ID`` (in both natural and reverse byte
orders) at selection time, looks the result up in the chip match
registry populated by per chip code via ``pmbus_register_chip()``,
and caches the matched ``pmbus_driver_info``. All subsequent
subcommands consume that cached metadata.

Standard subcommands
~~~~~~~~~~~~~~~~~~~~

::

    pmbus list                       list UCLASS_REGULATOR devices (DM bound)
    pmbus dev [<bus>:<addr>]         show / select active PMBus device
    pmbus info                       identification banner + driver_info
    pmbus telemetry                  decoded VIN, VOUT, IIN, IOUT, TEMP
    pmbus status                     decode every STATUS_* register
    pmbus dump                       hex dump of every standard register
    pmbus read <reg> [b|w|s]         raw read (b=byte, w=word, s=string)
    pmbus write <reg> <val> [b|w]    raw write
    pmbus clear [faults]             issue CLEAR_FAULTS (03h)
    pmbus vout [<uV>]                read or set VOUT_COMMAND (microvolts)
    pmbus scan [<bus>]               PMBus aware probe of one or all I2C buses

The ``<reg>`` argument accepts either a hexadecimal address
(``88``, ``0x`` optional) or a symbolic name (``READ_VIN``,
``VOUT_MODE``, ``MFR_ID``); symbolic names win when both parsed.
``<val>`` is hexadecimal too. Only ``pmbus vout``'s microvolt
argument and bus numbers are decimal.
Format selectors after the register select the SMBus transaction width:
``b`` for byte, ``w`` for 16 bit little endian word,
``s`` for the SMBus block read used by string registers.

Decoded telemetry honours the active device's ``pmbus_driver_info``;
when no chip match has been registered, VOUT falls back to LINEAR16
driven by ``VOUT_MODE`` and the other sensors fall back to LINEAR11.

Vendor namespace
~~~~~~~~~~~~~~~~

Per chip drivers and board files publish chip specific subcommands
in the ``pmbus <vendor> ...`` namespace by calling
``pmbus_register_vendor_handler()`` at init time. The framework
dispatches ``pmbus mps last``, ``pmbus mps clear last``, and
``pmbus mps clear force`` to the MPS handler when the active
device matches the ``mps`` vendor. Additional vendor handlers for
``lltc``, ``renesas``, etc. land alongside the per chip drivers
that need them.

Relationship to ``vdd_override`` / ``vdd_read``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The NXP Layerscape ``vdd_override <mV>`` and ``vdd_read`` commands
remain available in their original form for compatibility with
existing AVS production scripts. The new ``pmbus vout`` and
``pmbus vout <uV>`` subcommands cover the read and single shot
write paths against the same chips, but do not implement
``vdd_override``'s full sequence (board drop compensation, fuse
target derivation, multi step convergence loop, atomic
``PAGE_PLUS_WRITE`` block transaction, ``WRITE_PROTECT`` dance).
For interactive bring up ``pmbus vout`` is sufficient; for
production AVS, ``vdd_override`` stays canonical.

Lifecycle: from board boot to Linux handoff
-------------------------------------------

The PMBus framework spans the entire U-Boot lifecycle. This section
walks the boot timeline phase by phase, showing when each piece
comes online and how the regulator uclass and the ``pmbus`` CLI
converge on the same chip.

Timeline overview
~~~~~~~~~~~~~~~~~

::

    Phase 0  chip power on              chip ramps to NVM default VOUT
    Phase 1  boot ROM / SPL / TF-A      PMBus typically untouched
    Phase 2  U-Boot relocation, DM init regulators bound, not probed
    Phase 3  first regulator probe      chip driver runs, framework lights up
    Phase 4  board hooks / boot scripts snapshot, AVS trim, gating
    Phase 5  Linux handoff              DT passed, chip state preserved
    Phase 6  Linux runtime              kernel pmbus driver takes over

Phase 0: chip power on
~~~~~~~~~~~~~~~~~~~~~~

When the regulator chip receives its input voltage, it ramps its
output to the VOUT default programmed into its NVM at factory
provisioning. PMBus is silent: no software runs anywhere on the
SoC yet.

Phase 1: pre U-Boot stages
~~~~~~~~~~~~~~~~~~~~~~~~~~

Boot ROMs, secondary boot loaders (SPL, ARM TF-A BL2 / BL31)
typically do not touch PMBus. They focus on PLLs, DDR PHY init,
and bringing up enough hardware to load the next stage. Some
platforms have a pre U-Boot AVS path in board specific TF-A
code that writes ``VOUT_COMMAND`` from a fuse derived target;
that path is independent of the U-Boot framework described here.

Phase 2: U-Boot relocation and DM init
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After relocation, U-Boot binds device tree nodes to drivers but
does not probe them. UCLASS_REGULATOR devices for PMBus chips
are bound (driver and DT match resolved) but the ``.probe``
callback has not run yet.

Framework state at this point:

* chip match registry: empty
* vendor handler registry: empty
* active device: none
* regulator uclass: devices bound, none probed

Phase 3: lazy regulator probe
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first caller into the regulator uclass for a given chip
triggers the chip driver's ``.probe``. Typical first callers:

* a board ``EVENT_SPY`` at ``EVT_LAST_STAGE_INIT`` (boot snapshot)
* a U-Boot script: ``regulator dev <name>; regulator value``
* the ``pmbus dev <name>`` CLI command (resolves to the regulator)
* another DT consumer with a ``regulator-supplies`` reference

The probe chain looks like this::

    <chip>_probe(dev)
        pmbus_regulator_probe_common(dev, &<chip>_info, page)
            dev_read_addr(dev)               -> reg = <addr>
            i2c_get_chip(dev->parent, addr)  -> I2C chip handle
            priv->i2c_dev = handle
            priv->info    = &<chip>_info
            priv->page    = page
            (page > 0)  write PMBUS_PAGE
        <chip>_identify_vout(priv->i2c_dev)        [optional]
            read VOUT_MODE; refine info->format[PSC_VOLTAGE_OUT]
        pmbus_regulator_apply_voltage_scale(dev, fb_div)  [optional]
            write PMBUS_VOUT_SCALE_LOOP if DT property set
        pmbus_register_chip(&<chip>_match)              [idempotent]
        pmbus_register_vendor_handler(&<chip>_op)       [idempotent]

Once probed, three independent surfaces are functional against
the same chip:

* the regulator uclass API (``regulator_get_value``,
  ``regulator_set_value``, ``regulator_get_enable``,
  ``regulator_set_enable``)
* the ``pmbus`` CLI (chip is reachable by name through
  ``pmbus_resolve_by_name()``, by raw ``<bus>:<addr>`` through
  ``pmbus_set_active()``)
* the chip's vendor extension subcommands (``pmbus <vendor> ...``)

Phase 4: board hooks and boot scripts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Boards hook the boot flow at well known points to drive board
specific PMBus behaviour. The framework prescribes none of these;
they are conventions:

* boot time rail snapshot. An ``EVENT_SPY`` at
  ``EVT_LAST_STAGE_INIT`` reads telemetry through the regulator
  uclass and prints a one shot summary to the console. Useful
  for operator visibility on serial during bring up.

* pre kernel rail trim (AVS). A board hook in
  ``board_late_init`` or a custom event spy reads a fuse derived
  target voltage and calls ``regulator_set_value_force()`` to
  trim the SoC core rail before kernel handoff.

* Linux handoff gate. A bootcmd reads the rail voltage
  through the regulator command and refuses to boot Linux if
  the rail is outside the expected range.

Phase 5: Linux handoff
~~~~~~~~~~~~~~~~~~~~~~

When U-Boot transfers control to Linux, it passes the device
tree (potentially patched). The DT compatible strings for PMBus
regulators must match those in the upstream kernel binding so
the kernel's ``drivers/hwmon/pmbus/<chip>.c`` picks them up.
Property names are shared with the kernel binding
(``regulator-name``, ``regulator-min-microvolt``,
``mps,vout-fb-divider-ratio-permille``, etc.); see "DT alignment
with Linux" below.

The chip itself is left in the state U-Boot wrote it to. If
U-Boot trimmed VOUT, the chip stays at the trimmed voltage
through handoff. ``CLEAR_FAULTS`` state is preserved unless an
operator explicitly issued one.

Phase 6: Linux runtime
~~~~~~~~~~~~~~~~~~~~~~

Linux's ``drivers/hwmon/pmbus/pmbus_core.c`` probes the chip,
exposes telemetry under ``/sys/class/hwmon``, and takes over
runtime voltage management through its regulator subsystem.
The hwmon framework polls periodically; U-Boot does not.

Operation paths through the regulator uclass
--------------------------------------------

After the first probe completes, calls into the regulator uclass
for a PMBus chip flow through the shared helper.

Read VOUT::

    regulator_get_value(dev)
       -> dm_regulator_ops->get_value
       -> pmbus_regulator_get_value(dev)
          pmbus_regulator_select_page(priv)
          pmbus_read_byte(priv->i2c_dev, VOUT_MODE, &mode)
          pmbus_read_word(priv->i2c_dev, READ_VOUT, &raw)
          pmbus_reg2data(priv->info, PSC_VOLTAGE_OUT, raw, mode)
              -> reg2data_linear16  (mode = 0)
              -> reg2data_direct    (chip configured for DIRECT)
          return engineering value (microvolts)

Write VOUT::

    regulator_set_value(dev, uV)
       -> dm_regulator_ops->set_value
       -> pmbus_regulator_set_value(dev, uV)
          pmbus_regulator_select_page(priv)
          pmbus_read_byte(VOUT_MODE)
          check (mode == LINEAR)         [LINEAR16 only today]
          raw = pmbus_data2reg_linear16(uV, mode)
          dm_i2c_write(VOUT_COMMAND, raw)

Read / write enable bit::

    regulator_get_enable(dev)
       -> pmbus_regulator_get_enable(dev)
          pmbus_read_byte(OPERATION) & PB_OPERATION_ON

    regulator_set_enable(dev, on)
       -> pmbus_regulator_set_enable(dev, on)
          read OPERATION, set or clear PB_OPERATION_ON, write back

Bus traffic per call:

* ``get_value``  : 1 byte read (VOUT_MODE) + 1 word read (READ_VOUT)
  + 1 byte write (PAGE) when ``page > 0``
* ``set_value``  : 1 byte read (VOUT_MODE) + 1 word write (VOUT_COMMAND)
  + 1 byte write (PAGE) when ``page > 0``
* ``get_enable`` : 1 byte read (OPERATION)
* ``set_enable`` : 1 byte read (OPERATION) + 1 byte write (OPERATION)

Common board hook patterns
~~~~~~~~~~~~~~~~~~~~~~~~~~

Boot time rail snapshot::

    static int my_board_pmbus_snapshot(void)
    {
        struct udevice *reg;

        if (regulator_get_by_platname("MY_RAIL", &reg))
            return 0;
        printf("MY_RAIL: VOUT = %d uV, enabled = %d\n",
               regulator_get_value(reg),
               regulator_get_enable(reg));
        return 0;
    }
    EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, my_board_pmbus_snapshot);

The first call to ``regulator_get_value()`` triggers the chip
driver's ``.probe``, which seeds the chip match and vendor
extension registries. Subsequent ``pmbus`` CLI commands work
without further setup.

Pre kernel rail trim (AVS)::

    int board_late_init(void)
    {
        struct udevice *reg;
        int target_uV = compute_avs_target();

        if (regulator_get_by_platname("VDD_CORE", &reg))
            return 0;
        return regulator_set_value_force(reg, target_uV);
    }

Use ``regulator_set_value_force()`` when the target may sit
outside the DT declared ``regulator-min-microvolt`` /
``regulator-max-microvolt`` range; force bypasses the bounds
check.

Adding a new PMBus chip from scratch
------------------------------------

Use this path when the chip has no Linux driver yet, or when you want
to validate the U-Boot port against the datasheet alone.

1. Confirm PMBus 1.x compliance level. Locate in the chip
   datasheet:

     which PMBus standard command codes the chip implements
     (``READ_VIN``, ``READ_VOUT``, ``STATUS_WORD``, ``MFR_ID`` ...),
     which numeric format(s) it uses for VOUT (LINEAR16 with the
     exponent in ``VOUT_MODE``, DIRECT with chip specific m/b/R, or
     VID with one of the documented VRM tables),
     which numeric format it uses for VIN, IIN, IOUT, TEMPERATURE
     (most commonly LINEAR11; some MPS / MPS derivative chips use
     DIRECT instead),
     how many output rails it exposes (single page parts vs.
     multi rail PMBus pages).

2. Declare a ``struct pmbus_driver_info``. Wire each sensor
   class to one ``enum pmbus_data_format``, plus the m/b/R triple if
   the format is DIRECT::

      static struct pmbus_driver_info chipname_info = {
          .pages = 1,
          .format[PSC_VOLTAGE_IN]  = pmbus_fmt_direct,
          .format[PSC_VOLTAGE_OUT] = pmbus_fmt_linear,
          .format[PSC_CURRENT_OUT] = pmbus_fmt_direct,
          .format[PSC_TEMPERATURE] = pmbus_fmt_direct,
          .m[PSC_VOLTAGE_IN]  = 4,  .R[PSC_VOLTAGE_IN]  = 1,
          .m[PSC_CURRENT_OUT] = 16, .R[PSC_CURRENT_OUT] = 0,
          .m[PSC_TEMPERATURE] = 1,  .R[PSC_TEMPERATURE] = 0,
      };

3. Bind to a DT compatible. Use the lowercase ``vendor,chip``
   tuple Linux uses (see "DT alignment with Linux" below). Add the
   driver under ``drivers/power/regulator/`` matching the existing
   skeleton (``fan53555.c``, ``pca9450.c``).

4. Rely on the DT binding from the Linux kernel which is imported into
   U-Boot under ``dts/upstream/Bindings/`` (for PMBus chips,
   ``dts/upstream/Bindings/hwmon/pmbus/``).

5. Smoke test. With the chip wired up in DT::

      => regulator dev <name>
      => regulator value
      => regulator info

   Numbers should match the bench measurement to within the chip's
   advertised LSB.

Porting an existing Linux PMBus driver to U-Boot
------------------------------------------------

When the chip already has a ``linux/drivers/hwmon/pmbus/<chip>.c``,
that driver is the authoritative reference for format, coefficients,
and quirks. Take what carries; leave what does not.

What carries verbatim
~~~~~~~~~~~~~~~~~~~~~

* Numeric formats (``format[PSC_*]``).
* DIRECT coefficients (``m[]``, ``b[]``, ``R[]``).
* Per page count and per page functionality bits (``pages``,
  ``func[]``).
* VOUT_MODE driven per chip identify hook (e.g. MPQ8785's
  switch between LINEAR16 and VID coerced DIRECT m=64 R=1).
* Vendor register addresses for chip specific quirks (fault
  history, scale-loop, page mapping).

What does not carry
~~~~~~~~~~~~~~~~~~~~~~~

* ``hwmon_device_register()`` and the attribute groups it consumes.
* ``struct pmbus_data`` / ``update_lock`` / ``last_updated``
  U-Boot has no caching layer.
* ALERT# IRQ wiring; U-Boot is single threaded boot code.
* Fan control hooks (``read_fan_*``, ``set_pwm_*``).
* Virtual register handling (``PMBUS_VIRT_READ_VIN_*`` etc.); those
  are entirely a hwmon publication aid.
* ``module_i2c_driver(...)`` and ``MODULE_*`` macros; U-Boot uses
  ``U_BOOT_DRIVER(...)``.

Worked example: porting MPQ8785
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Linux's ``drivers/hwmon/pmbus/mpq8785.c`` is 193 LOC; the U-Boot
equivalent is ~150 LOC.

The ``mpq8785_info`` struct transcribes verbatim::

    .pages = 1,
    .format[PSC_VOLTAGE_IN]  = direct,  .m[PSC_VOLTAGE_IN]  = 4,  .R[PSC_VOLTAGE_IN]  = 1,
    .format[PSC_CURRENT_OUT] = direct,  .m[PSC_CURRENT_OUT] = 16, .R[PSC_CURRENT_OUT] = 0,
    .format[PSC_TEMPERATURE] = direct,  .m[PSC_TEMPERATURE] = 1,  .R[PSC_TEMPERATURE] = 0,

The VOUT format is decided at probe time from VOUT_MODE bits[7:5] :
mode 0 means LINEAR16, mode 1 or 2 means DIRECT m=64 R=1 (the chip's
"VID" mode is coerced to DIRECT by the driver). Translate Linux's
``mpq8785_identify()`` 1:1.

The per chip quirks that carry over:

* MPS NVM string byte order: chip stores ``S P M`` for the human
  string ``MPS``. ``pmbus_read_string()`` accepts a ``reverse_bytes``
  flag for this case.
* ``mps,vout-fb-divider-ratio-permille`` DT property maps to
  ``VOUT_SCALE_LOOP`` write at probe time.

The quirks that do not carry over:

* The ``PMBUS_VIRT_*`` virtual sensor wiring. Drop entirely.
* The ``hwmon_chip_info`` attribute group registration.
* The ``MODULE_AUTHOR`` / ``MODULE_LICENSE`` declarations.

Using the generic ``compatible = "pmbus"`` driver
-------------------------------------------------

When a board carries a PMBus chip without a per chip U-Boot driver,
the catch all ``drivers/power/regulator/pmbus_generic.c`` (Layer 3b)
binds against ``compatible = "pmbus"``. It auto detects format via
``VOUT_MODE`` and ``PMBUS_QUERY`` (where the chip supports it) and
provides telemetry + voltage set/get against the standard PMBus 1.x
subset.

Decision tree:

1. Try the generic driver first. Add the regulator node to the
   board DT with ``compatible = "pmbus"`` plus the standard
   regulator properties. Boot, run ``regulator value``, compare
   against bench measurement.
2. Switch to a per chip driver only when the generic one is
   wrong: telemetry shows wrong values (chip uses DIRECT with
   non default coefficients), an alert can't be decoded (chip has
   vendor specific status bits), AVS is needed (the boot path has
   to actively trim VOUT before kernel handoff), or the chip has an
   ADDR-pin auto promotion / VID coercion / vendor register quirk.

DT alignment with Linux
-----------------------

The same ``.dts`` file should work under both U-Boot (BL33) and Linux
post handoff. To make that possible:

* Reuse the upstream Linux compatible for every PMBus chip. Look
  in ``linux/Documentation/devicetree/bindings/hwmon/pmbus/`` and
  ``linux/Documentation/devicetree/bindings/regulator/``. The
  ``<vendor>,<chip>`` tuple from the kernel binding goes into U-Boot's
  ``of_match_table`` unchanged.
* Reuse Linux property names verbatim: ``regulator-name``,
  ``regulator-min-microvolt``, ``regulator-max-microvolt``,
  ``regulator-boot-on``, ``regulator-always-on``,
  ``mps,vout-fb-divider-ratio-permille``, etc.
* The DT binding is the kernel's, imported under
  ``dts/upstream/Bindings/`` (PMBus chips live in
  ``dts/upstream/Bindings/hwmon/pmbus/``).

Multi rail/multi page chips (e.g. ISL68137 with seven outputs)
declare each rail as a child regulator node with ``reg = <page>``;
each child binds as a UCLASS_REGULATOR with that PMBus PAGE setting
applied at every read/write.

Common pitfalls
---------------

These have all bitten contributors during nbxv3 bring up; record them
here so the next port doesn't repeat them.

* VOUT_MODE/DIRECT format confusion. Most generic PMBus call
  sites assume LINEAR16. Several MPS chips report VOUT in DIRECT
  format with chip specific m/b/R after a single VOUT_MODE read,
  the same chip read at the same address produces different
  numbers depending on the format the driver applies. Always read
  ``VOUT_MODE`` at probe time and switch the decoder accordingly.
  Linux's per chip ``identify()`` callbacks document the exact
  rules; copy them rather than guessing.
* SMBus block read protocol. Some I2C controllers strict check
  block read transactions: the master must read the length byte
  first, then reissue the read for the payload. Over reading a
  fixed length and ignoring the length byte works on lenient
  controllers but errors on strict ones. ``pmbus_read_string()``
  does the two stage read; use it.
* I2C bus number stability. ``uclass_get_device_by_seq()``
  uses the DT alias index (``i2c0`` -> ``UCLASS_I2C`` seq 0) when
  aliases are declared, otherwise falls back to probe order which
  varies with which controllers are enabled in the defconfig.
  Always declare DT aliases for I2C buses you reference by index.
* ADDR-pin auto addressessing. Some chips (notably MPS parts) decode
  their PMBus 7-bit address from an external resistor divider on
  ADDR_VBOOT. The "default" address in the datasheet is the
  factory fused slot; a board with a different divider or a die
  with a different revision can land in another window. If the
  driver hardcodes the default and the board side scan finds the
  chip in another window, auto promote the working address rather
  than failing the probe.
* MFR string byte order. Most PMBus chips return ``MFR_ID``
  characters in human order. Some MPS personalities reverse them.
  Pass ``reverse_bytes=true`` to ``pmbus_read_string()`` for those;
  spec compliant chips pass false.

References
----------

* Linux PMBus core: ``linux/drivers/hwmon/pmbus/pmbus_core.c``,
  decoder reference; ignore the hwmon publication and caching layers.
* Linux PMBus header: ``linux/drivers/hwmon/pmbus/pmbus.h``; API
  surface reference; many constants and the ``struct
  pmbus_driver_info`` shape are mirrored verbatim into U-Boot's
  ``include/pmbus.h``.
* Linux DT bindings:
  ``linux/Documentation/devicetree/bindings/hwmon/pmbus/``.
