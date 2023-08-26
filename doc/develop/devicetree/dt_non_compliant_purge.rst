.. SPDX-License-Identifier: GPL-2.0+

Removal of non-compliant nodes and properties
=============================================

The devicetree used in U-Boot might contain nodes and properties which
are specific only to U-Boot, and are not necessarily being used to
describe hardware but to pass information to U-Boot. An example of
such a property would be the public key being passed to U-Boot for
verification.

This devicetree can then be passed to the OS. Since certain nodes and
properties are not really describing hardware, and more importantly,
these are only relevant to U-Boot, bindings for these cannot be
upstreamed into the devicetree repository. There have been instances
of attempts being made to upstream such bindings, and these deemed not
fit for upstreaming. Not having a binding for these nodes and
properties means that the devicetree fails the schema compliance tests
[1]. This also means that the platform cannot get certifications like
SystemReady [2] which, among other things require a devicetree which
passes the schema compliance tests.

For such nodes and properties, it has been suggested by the devicetree
maintainers that the right thing to do is to remove them from the
devicetree before it gets passed on to the OS [3].

Removing nodes/properties
-------------------------

In U-Boot, this is been done through adding information on such nodes
and properties in a list. The entire node can be deleted, or a
specific property under a node can be deleted. The list of such nodes
and properties is generated at compile time, and the function to purge
these can be invoked through a EVT_FT_FIXUP event notify call.

For deleting a node, this can be done by declaring a macro::

	DT_NON_COMPLIANT_PURGE(fwu_mdata) = {
		.node_path      = "/fwu-mdata",
	};

Similarly, for deleting a property under a node, that can be done by
specifying the property name::

	DT_NON_COMPLIANT_PURGE(capsule_key) = {
		.node_path      = "/signature",
		.prop           = "capsule-key",
	};

In the first example, the entire node with path /fwu-mdata will be
removed. In the second example, the property capsule-key
under /signature node will be removed.

Similarly, a list of nodes and properties can be specified using the
following macro::

	DT_NON_COMPLIANT_PURGE_LIST(foo) = {
		{ .node_path = "/some_node", .prop = "some_bar" },
		{ .node_path = "/some_node" },
	};

[1] - https://github.com/devicetree-org/dt-schema
[2] - https://www.arm.com/architecture/system-architectures/systemready-certification-program
[3] - https://lore.kernel.org/u-boot/CAL_JsqJN4FeHomL7z3yj0WJ9bpx1oSE7zf26L_GV2oS6cg-5qg@mail.gmail.com/
