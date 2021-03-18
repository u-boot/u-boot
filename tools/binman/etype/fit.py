# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for producing a FIT
#

from collections import defaultdict, OrderedDict
import libfdt

from binman.entry import Entry, EntryArg
from dtoc import fdt_util
from dtoc.fdt import Fdt
from patman import tools

class Entry_fit(Entry):
    """Entry containing a FIT

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

    U-Boot supports creating fdt and config nodes automatically. To do this,
    pass an of-list property (e.g. -a of-list=file1 file2). This tells binman
    that you want to generates nodes for two files: file1.dtb and file2.dtb
    The fit,fdt-list property (see above) indicates that of-list should be used.
    If the property is missing you will get an error.

    Then add a 'generator node', a node with a name starting with '@'::

        images {
            @fdt-SEQ {
                description = "fdt-NAME";
                type = "flat_dt";
                compression = "none";
            };
        };

    This tells binman to create nodes fdt-1 and fdt-2 for each of your two
    files. All the properties you specify will be included in the node. This
    node acts like a template to generate the nodes. The generator node itself
    does not appear in the output - it is replaced with what binman generates.

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

    This tells binman to create nodes config-1 and config-2, i.e. a config for
    each of your two files.

    Available substitutions for '@' nodes are:

    SEQ:
        Sequence number of the generated fdt (1, 2, ...)
    NAME
        Name of the dtb as provided (i.e. without adding '.dtb')

    Note that if no devicetree files are provided (with '-a of-list' as above)
    then no nodes will be generated.

    The 'default' property, if present, will be automatically set to the name
    if of configuration whose devicetree matches the 'default-dt' entry
    argument, e.g. with '-a default-dt=sun50i-a64-pine64-lts'.

    Available substitutions for '@' property values are

    DEFAULT-SEQ:
        Sequence number of the default fdt,as provided by the 'default-dt' entry
        argument

    Properties (in the 'fit' node itself):
        fit,external-offset: Indicates that the contents of the FIT are external
            and provides the external offset. This is passsed to mkimage via
            the -E and -p flags.

    """
    def __init__(self, section, etype, node):
        """
        Members:
            _fit: FIT file being built
            _fit_sections: dict:
                key: relative path to entry Node (from the base of the FIT)
                value: Entry_section object comprising the contents of this
                    node
        """
        super().__init__(section, etype, node)
        self._fit = None
        self._fit_sections = {}
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

    def ReadNode(self):
        self._ReadSubnodes()
        super().ReadNode()

    def _ReadSubnodes(self):
        def _AddNode(base_node, depth, node):
            """Add a node to the FIT

            Args:
                base_node: Base Node of the FIT (with 'description' property)
                depth: Current node depth (0 is the base node)
                node: Current node to process

            There are two cases to deal with:
                - hash and signature nodes which become part of the FIT
                - binman entries which are used to define the 'data' for each
                  image
            """
            for pname, prop in node.props.items():
                if not pname.startswith('fit,'):
                    if pname == 'default':
                        val = prop.value
                        # Handle the 'default' property
                        if val.startswith('@'):
                            if not self._fdts:
                                continue
                            if not self._fit_default_dt:
                                self.Raise("Generated 'default' node requires default-dt entry argument")
                            if self._fit_default_dt not in self._fdts:
                                self.Raise("default-dt entry argument '%s' not found in fdt list: %s" %
                                           (self._fit_default_dt,
                                            ', '.join(self._fdts)))
                            seq = self._fdts.index(self._fit_default_dt)
                            val = val[1:].replace('DEFAULT-SEQ', str(seq + 1))
                            fsw.property_string(pname, val)
                            continue
                    fsw.property(pname, prop.bytes)

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
                self._fit_sections[rel_path] = entry

            for subnode in node.subnodes:
                if has_images and not (subnode.name.startswith('hash') or
                                       subnode.name.startswith('signature')):
                    # This subnode is a content node not meant to appear in
                    # the FIT (e.g. "/images/kernel/u-boot"), so don't call
                    # fsw.add_node() or _AddNode() for it.
                    pass
                elif subnode.name.startswith('@'):
                    if self._fdts:
                        # Generate notes for each FDT
                        for seq, fdt_fname in enumerate(self._fdts):
                            node_name = subnode.name[1:].replace('SEQ',
                                                                 str(seq + 1))
                            fname = tools.GetInputFilename(fdt_fname + '.dtb')
                            with fsw.add_node(node_name):
                                for pname, prop in subnode.props.items():
                                    val = prop.bytes.replace(
                                        b'NAME', tools.ToBytes(fdt_fname))
                                    val = val.replace(
                                        b'SEQ', tools.ToBytes(str(seq + 1)))
                                    fsw.property(pname, val)

                                # Add data for 'fdt' nodes (but not 'config')
                                if depth == 1 and in_images:
                                    fsw.property('data',
                                                 tools.ReadFile(fname))
                    else:
                        if self._fdts is None:
                            if self._fit_list_prop:
                                self.Raise("Generator node requires '%s' entry argument" %
                                           self._fit_list_prop.value)
                            else:
                                self.Raise("Generator node requires 'fit,fdt-list' property")
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

    def ObtainContents(self):
        """Obtain the contents of the FIT

        This adds the 'data' properties to the input ITB (Image-tree Binary)
        then runs mkimage to process it.
        """
        # self._BuildInput() either returns bytes or raises an exception.
        data = self._BuildInput(self._fdt)
        uniq = self.GetUniqueName()
        input_fname = tools.GetOutputFilename('%s.itb' % uniq)
        output_fname = tools.GetOutputFilename('%s.fit' % uniq)
        tools.WriteFile(input_fname, data)
        tools.WriteFile(output_fname, data)

        args = []
        ext_offset = self._fit_props.get('fit,external-offset')
        if ext_offset is not None:
            args += ['-E', '-p', '%x' % fdt_util.fdt32_to_cpu(ext_offset.value)]
        tools.Run('mkimage', '-t', '-F', output_fname, *args)

        self.SetContents(tools.ReadFile(output_fname))
        return True

    def _BuildInput(self, fdt):
        """Finish the FIT by adding the 'data' properties to it

        Arguments:
            fdt: FIT to update

        Returns:
            New fdt contents (bytes)
        """
        for path, section in self._fit_sections.items():
            node = fdt.GetNode(path)
            # Entry_section.ObtainContents() either returns True or
            # raises an exception.
            section.ObtainContents()
            section.Pack(0)
            data = section.GetData()
            node.AddData('data', data)

        fdt.Sync(auto_resize=True)
        data = fdt.GetContents()
        return data

    def CheckMissing(self, missing_list):
        """Check if any entries in this FIT have missing external blobs

        If there are missing blobs, the entries are added to the list

        Args:
            missing_list: List of Entry objects to be added to
        """
        for path, section in self._fit_sections.items():
            section.CheckMissing(missing_list)

    def SetAllowMissing(self, allow_missing):
        for section in self._fit_sections.values():
            section.SetAllowMissing(allow_missing)
