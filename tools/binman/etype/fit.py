# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for producing a FIT
#

from collections import defaultdict, OrderedDict
import libfdt

from binman.entry import Entry, EntryArg
from binman.etype.section import Entry_section
from dtoc import fdt_util
from dtoc.fdt import Fdt
from patman import tools

# Supported operations, with the fit,operation property
OP_GEN_FDT_NODES = range(1)
OPERATIONS = {
    'gen-fdt-nodes': OP_GEN_FDT_NODES,
    }

class Entry_fit(Entry_section):

    """Flat Image Tree (FIT)

    This calls mkimage to create a FIT (U-Boot Flat Image Tree) based on the
    input provided.

    Nodes for the FIT should be written out in the binman configuration just as
    they would be in a file passed to mkimage.

    For example, this creates an image containing a FIT with U-Boot SPL::

        binman {
            fit {
                description = "Test FIT";
                fit,fdt-list = "of-list";

                images {
                    kernel@1 {
                        description = "SPL";
                        os = "u-boot";
                        type = "rkspi";
                        arch = "arm";
                        compression = "none";
                        load = <0>;
                        entry = <0>;

                        u-boot-spl {
                        };
                    };
                };
            };
        };

    More complex setups can be created, with generated nodes, as described
    below.

    Properties (in the 'fit' node itself)
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Special properties have a `fit,` prefix, indicating that they should be
    processed but not included in the final FIT.

    The top-level 'fit' node supports the following special properties:

        fit,external-offset
            Indicates that the contents of the FIT are external and provides the
            external offset. This is passed to mkimage via the -E and -p flags.

        fit,fdt-list
            Indicates the entry argument which provides the list of device tree
            files for the gen-fdt-nodes operation (as below). This is often
            `of-list` meaning that `-a of-list="dtb1 dtb2..."` should be passed
            to binman.

    Substitutions
    ~~~~~~~~~~~~~

    Node names and property values support a basic string-substitution feature.
    Available substitutions for '@' nodes (and property values) are:

    SEQ:
        Sequence number of the generated fdt (1, 2, ...)
    NAME
        Name of the dtb as provided (i.e. without adding '.dtb')

    The `default` property, if present, will be automatically set to the name
    if of configuration whose devicetree matches the `default-dt` entry
    argument, e.g. with `-a default-dt=sun50i-a64-pine64-lts`.

    Available substitutions for property values in these nodes are:

    DEFAULT-SEQ:
        Sequence number of the default fdt, as provided by the 'default-dt'
        entry argument

    Available operations
    ~~~~~~~~~~~~~~~~~~~~

    You can add an operation to an '@' node to indicate which operation is
    required::

        @fdt-SEQ {
            fit,operation = "gen-fdt-nodes";
            ...
        };

    Available operations are:

    gen-fdt-nodes
        Generate FDT nodes as above. This is the default if there is no
        `fit,operation` property.

    Generating nodes from an FDT list (gen-fdt-nodes)
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    U-Boot supports creating fdt and config nodes automatically. To do this,
    pass an `of-list` property (e.g. `-a of-list=file1 file2`). This tells
    binman that you want to generates nodes for two files: `file1.dtb` and
    `file2.dtb`. The `fit,fdt-list` property (see above) indicates that
    `of-list` should be used. If the property is missing you will get an error.

    Then add a 'generator node', a node with a name starting with '@'::

        images {
            @fdt-SEQ {
                description = "fdt-NAME";
                type = "flat_dt";
                compression = "none";
            };
        };

    This tells binman to create nodes `fdt-1` and `fdt-2` for each of your two
    files. All the properties you specify will be included in the node. This
    node acts like a template to generate the nodes. The generator node itself
    does not appear in the output - it is replaced with what binman generates.
    A 'data' property is created with the contents of the FDT file.

    You can create config nodes in a similar way::

        configurations {
            default = "@config-DEFAULT-SEQ";
            @config-SEQ {
                description = "NAME";
                firmware = "atf";
                loadables = "uboot";
                fdt = "fdt-SEQ";
            };
        };

    This tells binman to create nodes `config-1` and `config-2`, i.e. a config
    for each of your two files.

    Note that if no devicetree files are provided (with '-a of-list' as above)
    then no nodes will be generated.
    """
    def __init__(self, section, etype, node):
        """
        Members:
            _fit: FIT file being built
            _entries: dict from Entry_section:
                key: relative path to entry Node (from the base of the FIT)
                value: Entry_section object comprising the contents of this
                    node
        """
        super().__init__(section, etype, node)
        self._fit = None
        self._fit_props = {}

        for pname, prop in self._node.props.items():
            if pname.startswith('fit,'):
                self._fit_props[pname] = prop

        self._fdts = None
        self._fit_list_prop = self._fit_props.get('fit,fdt-list')
        if self._fit_list_prop:
            fdts, = self.GetEntryArgsOrProps(
                [EntryArg(self._fit_list_prop.value, str)])
            if fdts is not None:
                self._fdts = fdts.split()
        self._fit_default_dt = self.GetEntryArgsOrProps([EntryArg('default-dt',
                                                                  str)])[0]
        self.mkimage = None

    def ReadNode(self):
        self.ReadEntries()
        super().ReadNode()

    def _get_operation(self, subnode):
        """Get the operation referenced by a subnode

        Args:
            subnode (Node): Subnode (of the FIT) to check

        Returns:
            int: Operation to perform

        Raises:
            ValueError: Invalid operation name
        """
        oper_name = subnode.props.get('fit,operation')
        if not oper_name:
            return OP_GEN_FDT_NODES
        oper = OPERATIONS.get(oper_name.value)
        if not oper:
            self.Raise(f"Unknown operation '{oper_name.value}'")
        return oper

    def ReadEntries(self):
        def _process_prop(pname, prop):
            """Process special properties

            Handles properties with generated values. At present the only
            supported property is 'default', i.e. the default device tree in
            the configurations node.

            Args:
                pname (str): Name of property
                prop (Prop): Property to process
            """
            if pname == 'default':
                val = prop.value
                # Handle the 'default' property
                if val.startswith('@'):
                    if not self._fdts:
                        return
                    if not self._fit_default_dt:
                        self.Raise("Generated 'default' node requires default-dt entry argument")
                    if self._fit_default_dt not in self._fdts:
                        self.Raise("default-dt entry argument '%s' not found in fdt list: %s" %
                                   (self._fit_default_dt,
                                    ', '.join(self._fdts)))
                    seq = self._fdts.index(self._fit_default_dt)
                    val = val[1:].replace('DEFAULT-SEQ', str(seq + 1))
                    fsw.property_string(pname, val)
                    return
            fsw.property(pname, prop.bytes)

        def _scan_gen_fdt_nodes(subnode, depth, in_images):
            """Generate FDT nodes

            This creates one node for each member of self._fdts using the
            provided template. If a property value contains 'NAME' it is
            replaced with the filename of the FDT. If a property value contains
            SEQ it is replaced with the node sequence number, where 1 is the
            first.

            Args:
                subnode (None): Generator node to process
                depth: Current node depth (0 is the base 'fit' node)
                in_images: True if this is inside the 'images' node, so that
                    'data' properties should be generated
            """
            if self._fdts:
                # Generate nodes for each FDT
                for seq, fdt_fname in enumerate(self._fdts):
                    node_name = subnode.name[1:].replace('SEQ', str(seq + 1))
                    fname = tools.get_input_filename(fdt_fname + '.dtb')
                    with fsw.add_node(node_name):
                        for pname, prop in subnode.props.items():
                            val = prop.bytes.replace(
                                b'NAME', tools.to_bytes(fdt_fname))
                            val = val.replace(
                                b'SEQ', tools.to_bytes(str(seq + 1)))
                            fsw.property(pname, val)

                        # Add data for 'images' nodes (but not 'config')
                        if depth == 1 and in_images:
                            fsw.property('data', tools.read_file(fname))
            else:
                if self._fdts is None:
                    if self._fit_list_prop:
                        self.Raise("Generator node requires '%s' entry argument" %
                                   self._fit_list_prop.value)
                    else:
                        self.Raise("Generator node requires 'fit,fdt-list' property")

        def _scan_node(subnode, depth, in_images):
            """Generate nodes from a template

            This creates one node for each member of self._fdts using the
            provided template. If a property value contains 'NAME' it is
            replaced with the filename of the FDT. If a property value contains
            SEQ it is replaced with the node sequence number, where 1 is the
            first.

            Args:
                subnode (None): Generator node to process
                depth: Current node depth (0 is the base 'fit' node)
                in_images: True if this is inside the 'images' node, so that
                    'data' properties should be generated
            """
            oper = self._get_operation(subnode)
            if oper == OP_GEN_FDT_NODES:
                _scan_gen_fdt_nodes(subnode, depth, in_images)

        def _AddNode(base_node, depth, node):
            """Add a node to the FIT

            Args:
                base_node: Base Node of the FIT (with 'description' property)
                depth: Current node depth (0 is the base 'fit' node)
                node: Current node to process

            There are two cases to deal with:
                - hash and signature nodes which become part of the FIT
                - binman entries which are used to define the 'data' for each
                  image
            """
            for pname, prop in node.props.items():
                if not pname.startswith('fit,'):
                    _process_prop(pname, prop)

            rel_path = node.path[len(base_node.path):]
            in_images = rel_path.startswith('/images')
            has_images = depth == 2 and in_images
            if has_images:
                # This node is a FIT subimage node (e.g. "/images/kernel")
                # containing content nodes. We collect the subimage nodes and
                # section entries for them here to merge the content subnodes
                # together and put the merged contents in the subimage node's
                # 'data' property later.
                entry = Entry.Create(self.section, node, etype='section')
                entry.ReadNode()
                # The hash subnodes here are for mkimage, not binman.
                entry.SetUpdateHash(False)
                self._entries[rel_path] = entry

            for subnode in node.subnodes:
                if has_images and not (subnode.name.startswith('hash') or
                                       subnode.name.startswith('signature')):
                    # This subnode is a content node not meant to appear in
                    # the FIT (e.g. "/images/kernel/u-boot"), so don't call
                    # fsw.add_node() or _AddNode() for it.
                    pass
                elif self.GetImage().generate and subnode.name.startswith('@'):
                    _scan_node(subnode, depth, in_images)
                else:
                    with fsw.add_node(subnode.name):
                        _AddNode(base_node, depth + 1, subnode)

        # Build a new tree with all nodes and properties starting from the
        # entry node
        fsw = libfdt.FdtSw()
        fsw.finish_reservemap()
        with fsw.add_node(''):
            _AddNode(self._node, 0, self._node)
        fdt = fsw.as_fdt()

        # Pack this new FDT and scan it so we can add the data later
        fdt.pack()
        self._fdt = Fdt.FromData(fdt.as_bytearray())
        self._fdt.Scan()

    def BuildSectionData(self, required):
        """Build FIT entry contents

        This adds the 'data' properties to the input ITB (Image-tree Binary)
        then runs mkimage to process it.

        Args:
            required: True if the data must be present, False if it is OK to
                return None

        Returns:
            Contents of the section (bytes)
        """
        data = self._BuildInput(self._fdt)
        uniq = self.GetUniqueName()
        input_fname = tools.get_output_filename('%s.itb' % uniq)
        output_fname = tools.get_output_filename('%s.fit' % uniq)
        tools.write_file(input_fname, data)
        tools.write_file(output_fname, data)

        args = {}
        ext_offset = self._fit_props.get('fit,external-offset')
        if ext_offset is not None:
            args = {
                'external': True,
                'pad': fdt_util.fdt32_to_cpu(ext_offset.value)
                }
        if self.mkimage.run(reset_timestamp=True, output_fname=output_fname,
                            **args) is None:
            # Bintool is missing; just use empty data as the output
            self.record_missing_bintool(self.mkimage)
            return tools.get_bytes(0, 1024)

        return tools.read_file(output_fname)

    def _BuildInput(self, fdt):
        """Finish the FIT by adding the 'data' properties to it

        Arguments:
            fdt: FIT to update

        Returns:
            New fdt contents (bytes)
        """
        for path, section in self._entries.items():
            node = fdt.GetNode(path)
            data = section.GetData()
            node.AddData('data', data)

        fdt.Sync(auto_resize=True)
        data = fdt.GetContents()
        return data

    def SetImagePos(self, image_pos):
        """Set the position in the image

        This sets each subentry's offsets, sizes and positions-in-image
        according to where they ended up in the packed FIT file.

        Args:
            image_pos: Position of this entry in the image
        """
        super().SetImagePos(image_pos)

        # If mkimage is missing we'll have empty data,
        # which will cause a FDT_ERR_BADMAGIC error
        if self.mkimage in self.missing_bintools:
            return

        fdt = Fdt.FromData(self.GetData())
        fdt.Scan()

        for path, section in self._entries.items():
            node = fdt.GetNode(path)

            data_prop = node.props.get("data")
            data_pos = fdt_util.GetInt(node, "data-position")
            data_offset = fdt_util.GetInt(node, "data-offset")
            data_size = fdt_util.GetInt(node, "data-size")

            # Contents are inside the FIT
            if data_prop is not None:
                # GetOffset() returns offset of a fdt_property struct,
                # which has 3 fdt32_t members before the actual data.
                offset = data_prop.GetOffset() + 12
                size = len(data_prop.bytes)

            # External offset from the base of the FIT
            elif data_pos is not None:
                offset = data_pos
                size = data_size

            # External offset from the end of the FIT, not used in binman
            elif data_offset is not None: # pragma: no cover
                offset = fdt.GetFdtObj().totalsize() + data_offset
                size = data_size

            # This should never happen
            else: # pragma: no cover
                self.Raise("%s: missing data properties" % (path))

            section.SetOffsetSize(offset, size)
            section.SetImagePos(self.image_pos)

    def AddBintools(self, tools):
        super().AddBintools(tools)
        self.mkimage = self.AddBintool(tools, 'mkimage')
