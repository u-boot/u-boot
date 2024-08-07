.. SPDX-License-Identifier: GPL-2.0-or-later

Universal Payload
-----------------

`Universal Payload (UPL) <https://universalpayload.github.io/spec/index.html>`_
is an Industry Standard for firmware components. UPL
is designed to improve interoperability within the firmware industry, allowing
mixing and matching of projects with less friction and fewer project-specific
implementations. UPL is cross-platform, supporting ARM, x86 and RISC-V
initially.

UPL is defined in termns of two firmware components:

`Platform Init`
	Perhaps initial setup of the hardware and jumps to the payload.

`Payload`
	Selects the OS to boot

In practice UPL can be used to handle any number of handoffs through the
firmware startup process, with one program acting as platform init and another
acting as the payload.

UPL provides a standard for three main pieces:

- file format for the payload, which may comprise multiple images to load
- handoff format for the information the payload needs, such as serial port,
  memory layout, etc.
- machine state and register settings at the point of handoff

See also the :doc:`cmd/upl`.

UPL in U-Boot
~~~~~~~~~~~~~

U-Boot supports:

- writing a UPL handoff (devicetree) in SPL
- reading a UPL handoff in U-Boot proper
- creating a FIT

There are some new FIT features in UPL which are not yet supported in U-Boot.

.. sectionauthor:: Simon Glass <sjg@chromium.org>
.. July 2024
