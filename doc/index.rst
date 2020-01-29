.. SPDX-License-Identifier: GPL-2.0+

.. _u-boot_doc:

The U-Boot Documentation
========================

This is the top level of the U-Boot's documentation tree.  U-Boot
documentation, like the U-Boot itself, is very much a work in progress;
that is especially true as we work to integrate our many scattered
documents into a coherent whole.  Please note that improvements to the
documentation are welcome; join the U-Boot list at http://lists.denx.de
if you want to help out.

.. toctree::
   :maxdepth: 2

User-oriented documentation
---------------------------

The following manuals are written for *users* of the U-Boot - those who are
trying to get it to work optimally on a given system.

.. toctree::
   :maxdepth: 2

   build/index

Unified Extensible Firmware (UEFI)
----------------------------------

U-Boot provides an implementation of the UEFI API allowing to run UEFI
compliant software like Linux, GRUB, and iPXE. Furthermore U-Boot itself
can be run an UEFI payload.

.. toctree::
   :maxdepth: 2

   uefi/index

Driver-Model documentation
--------------------------

The following holds information on the U-Boot device driver framework:
driver-model, including the design details of itself and several driver
subsystems.

.. toctree::
   :maxdepth: 2

   driver-model/index

U-Boot API documentation
------------------------

These books get into the details of how specific U-Boot subsystems work
from the point of view of a U-Boot developer.  Much of the information here
is taken directly from the U-Boot source, with supplemental material added
as needed (or at least as we managed to add it - probably *not* all that is
needed).

.. toctree::
   :maxdepth: 2

   api/index

Architecture-specific doc
-------------------------

These books provide programming details about architecture-specific
implementation.

.. toctree::
   :maxdepth: 2

   arch/index

Board-specific doc
------------------

These books provide details about board-specific information. They are
organized in a vendor subdirectory.

.. toctree::
   :maxdepth: 2

   board/index

Indices and tables
==================

* :ref:`genindex`
