#!/usr/bin/python
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

"""Device tree to platform data class

This supports converting device tree data to C structures definitions and
static data.

See doc/driver-model/of-plat.rst for more informaiton
"""

import collections
import copy
from enum import IntEnum
import os
import re
import sys

from dtoc import fdt
from dtoc import fdt_util
from dtoc import src_scan
from dtoc.src_scan import conv_name_to_c

# When we see these properties we ignore them - i.e. do not create a structure
# member
PROP_IGNORE_LIST = [
    '#address-cells',
    '#gpio-cells',
    '#size-cells',
    'compatible',
    'linux,phandle',
    "status",
    'phandle',
    'u-boot,dm-pre-reloc',
    'u-boot,dm-tpl',
    'u-boot,dm-spl',
]

# C type declarations for the types we support
TYPE_NAMES = {
    fdt.Type.INT: 'fdt32_t',
    fdt.Type.BYTE: 'unsigned char',
    fdt.Type.STRING: 'const char *',
    fdt.Type.BOOL: 'bool',
    fdt.Type.INT64: 'fdt64_t',
}

STRUCT_PREFIX = 'dtd_'
VAL_PREFIX = 'dtv_'

class Ftype(IntEnum):
    SOURCE, HEADER = range(2)


# This holds information about each type of output file dtoc can create
# type: Type of file (Ftype)
# fname: Filename excluding directory, e.g. 'dt-plat.c'
# hdr_comment: Comment explaining the purpose of the file
OutputFile = collections.namedtuple('OutputFile',
                                    ['ftype', 'fname', 'method', 'hdr_comment'])

# This holds information about a property which includes phandles.
#
# max_args: integer: Maximum number or arguments that any phandle uses (int).
# args: Number of args for each phandle in the property. The total number of
#     phandles is len(args). This is a list of integers.
PhandleInfo = collections.namedtuple('PhandleInfo', ['max_args', 'args'])

# Holds a single phandle link, allowing a C struct value to be assigned to point
# to a device
#
# var_node: C variable to assign (e.g. 'dtv_mmc.clocks[0].node')
# dev_name: Name of device to assign to (e.g. 'clock')
PhandleLink = collections.namedtuple('PhandleLink', ['var_node', 'dev_name'])


def tab_to(num_tabs, line):
    """Append tabs to a line of text to reach a tab stop.

    Args:
        num_tabs (int): Tab stop to obtain (0 = column 0, 1 = column 8, etc.)
        line (str): Line of text to append to

    Returns:
        str: line with the correct number of tabs appeneded. If the line already
        extends past that tab stop then a single space is appended.
    """
    if len(line) >= num_tabs * 8:
        return line + ' '
    return line + '\t' * (num_tabs - len(line) // 8)

def get_value(ftype, value):
    """Get a value as a C expression

    For integers this returns a byte-swapped (little-endian) hex string
    For bytes this returns a hex string, e.g. 0x12
    For strings this returns a literal string enclosed in quotes
    For booleans this return 'true'

    Args:
        ftype (fdt.Type): Data type (fdt_util)
        value (bytes): Data value, as a string of bytes

    Returns:
        str: String representation of the value
    """
    if ftype == fdt.Type.INT:
        val = '%#x' % fdt_util.fdt32_to_cpu(value)
    elif ftype == fdt.Type.BYTE:
        char = value[0]
        val = '%#x' % (ord(char) if isinstance(char, str) else char)
    elif ftype == fdt.Type.STRING:
        # Handle evil ACPI backslashes by adding another backslash before them.
        # So "\\_SB.GPO0" in the device tree effectively stays like that in C
        val = '"%s"' % value.replace('\\', '\\\\')
    elif ftype == fdt.Type.BOOL:
        val = 'true'
    else:  # ftype == fdt.Type.INT64:
        val = '%#x' % value
    return val


class DtbPlatdata():
    """Provide a means to convert device tree binary data to platform data

    The output of this process is C structures which can be used in space-
    constrained encvironments where the ~3KB code overhead of device tree
    code is not affordable.

    Properties:
        _scan: Scan object, for scanning and reporting on useful information
            from the U-Boot source code
        _fdt: Fdt object, referencing the device tree
        _dtb_fname: Filename of the input device tree binary file
        _valid_nodes: A list of Node object with compatible strings. The list
            is ordered by conv_name_to_c(node.name)
        _include_disabled: true to include nodes marked status = "disabled"
        _outfile: The current output file (sys.stdout or a real file)
        _lines: Stashed list of output lines for outputting in the future
        _dirname: Directory to hold output files, or None for none (all files
            go to stdout)
        _struct_data (dict): OrderedDict of dtplat structures to output
            key (str): Node name, as a C identifier
                    value: dict containing structure fields:
                        key (str): Field name
                        value: Prop object with field information
        _basedir (str): Base directory of source tree
    """
    def __init__(self, scan, dtb_fname, include_disabled):
        self._scan = scan
        self._fdt = None
        self._dtb_fname = dtb_fname
        self._valid_nodes = None
        self._include_disabled = include_disabled
        self._outfile = None
        self._lines = []
        self._dirnames = [None] * len(Ftype)
        self._struct_data = collections.OrderedDict()
        self._basedir = None

    def setup_output_dirs(self, output_dirs):
        """Set up the output directories

        This should be done before setup_output() is called

        Args:
            output_dirs (tuple of str):
                Directory to use for C output files.
                    Use None to write files relative current directory
                Directory to use for H output files.
                    Defaults to the C output dir
        """
        def process_dir(ftype, dirname):
            if dirname:
                os.makedirs(dirname, exist_ok=True)
                self._dirnames[ftype] = dirname

        if output_dirs:
            c_dirname = output_dirs[0]
            h_dirname = output_dirs[1] if len(output_dirs) > 1 else c_dirname
            process_dir(Ftype.SOURCE, c_dirname)
            process_dir(Ftype.HEADER, h_dirname)

    def setup_output(self, ftype, fname):
        """Set up the output destination

        Once this is done, future calls to self.out() will output to this
        file. The file used is as follows:

        self._dirnames[ftype] is None: output to fname, or stdout if None
        self._dirnames[ftype] is not None: output to fname in that directory

        Calling this function multiple times will close the old file and open
        the new one. If they are the same file, nothing happens and output will
        continue to the same file.

        Args:
            ftype (str): Type of file to create ('c' or 'h')
            fname (str): Filename to send output to. If there is a directory in
                self._dirnames for this file type, it will be put in that
                directory
        """
        dirname = self._dirnames[ftype]
        if dirname:
            pathname = os.path.join(dirname, fname)
            if self._outfile:
                self._outfile.close()
            self._outfile = open(pathname, 'w')
        elif fname:
            if not self._outfile:
                self._outfile = open(fname, 'w')
        else:
            self._outfile = sys.stdout

    def finish_output(self):
        """Finish outputing to a file

        This closes the output file, if one is in use
        """
        if self._outfile != sys.stdout:
            self._outfile.close()

    def out(self, line):
        """Output a string to the output file

        Args:
            line (str): String to output
        """
        self._outfile.write(line)

    def buf(self, line):
        """Buffer up a string to send later

        Args:
            line (str): String to add to our 'buffer' list
        """
        self._lines.append(line)

    def get_buf(self):
        """Get the contents of the output buffer, and clear it

        Returns:
            list(str): The output buffer, which is then cleared for future use
        """
        lines = self._lines
        self._lines = []
        return lines

    def out_header(self, outfile):
        """Output a message indicating that this is an auto-generated file

        Args:
            outfile: OutputFile describing the file being generated
        """
        self.out('''/*
 * DO NOT MODIFY
 *
 * %s.
 * This was generated by dtoc from a .dtb (device tree binary) file.
 */

''' % outfile.hdr_comment)

    def get_phandle_argc(self, prop, node_name):
        """Check if a node contains phandles

        We have no reliable way of detecting whether a node uses a phandle
        or not. As an interim measure, use a list of known property names.

        Args:
            prop (fdt.Prop): Prop object to check
            node_name (str): Node name, only used for raising an error
        Returns:
            int or None: Number of argument cells is this is a phandle,
                else None
        Raises:
            ValueError: if the phandle cannot be parsed or the required property
                is not present
        """
        if prop.name in ['clocks', 'cd-gpios']:
            if not isinstance(prop.value, list):
                prop.value = [prop.value]
            val = prop.value
            i = 0

            max_args = 0
            args = []
            while i < len(val):
                phandle = fdt_util.fdt32_to_cpu(val[i])
                # If we get to the end of the list, stop. This can happen
                # since some nodes have more phandles in the list than others,
                # but we allocate enough space for the largest list. So those
                # nodes with shorter lists end up with zeroes at the end.
                if not phandle:
                    break
                target = self._fdt.phandle_to_node.get(phandle)
                if not target:
                    raise ValueError("Cannot parse '%s' in node '%s'" %
                                     (prop.name, node_name))
                cells = None
                for prop_name in ['#clock-cells', '#gpio-cells']:
                    cells = target.props.get(prop_name)
                    if cells:
                        break
                if not cells:
                    raise ValueError("Node '%s' has no cells property" %
                                     (target.name))
                num_args = fdt_util.fdt32_to_cpu(cells.value)
                max_args = max(max_args, num_args)
                args.append(num_args)
                i += 1 + num_args
            return PhandleInfo(max_args, args)
        return None

    def scan_dtb(self):
        """Scan the device tree to obtain a tree of nodes and properties

        Once this is done, self._fdt.GetRoot() can be called to obtain the
        device tree root node, and progress from there.
        """
        self._fdt = fdt.FdtScan(self._dtb_fname)

    def scan_node(self, root, valid_nodes):
        """Scan a node and subnodes to build a tree of node and phandle info

        This adds each node to self._valid_nodes.

        Args:
            root (Node): Root node for scan
            valid_nodes (list of Node): List of Node objects to add to
        """
        for node in root.subnodes:
            if 'compatible' in node.props:
                status = node.props.get('status')
                if (not self._include_disabled and not status or
                        status.value != 'disabled'):
                    valid_nodes.append(node)

            # recurse to handle any subnodes
            self.scan_node(node, valid_nodes)

    def scan_tree(self):
        """Scan the device tree for useful information

        This fills in the following properties:
            _valid_nodes: A list of nodes we wish to consider include in the
                platform data
        """
        valid_nodes = []
        self.scan_node(self._fdt.GetRoot(), valid_nodes)
        self._valid_nodes = sorted(valid_nodes,
                                   key=lambda x: conv_name_to_c(x.name))

    def prepare_nodes(self):
        """Add extra properties to the nodes we are using

        The following properties are added for use by dtoc:
            idx: Index number of this node (0=first, etc.)
            struct_name: Name of the struct dtd used by this node
            var_name: C name for this node
            child_devs: List of child devices for this node, each a None
            child_refs: Dict of references for each child:
                key: Position in child list (-1=head, 0=first, 1=second, ...
                                             n-1=last, n=head)
            seq: Sequence number of the device (unique within its uclass), or
                -1 not not known yet
            dev_ref: Reference to this device, e.g. 'DM_DEVICE_REF(serial)'
            driver: Driver record for this node, or None if not known
            uclass: Uclass record for this node, or None if not known
            uclass_seq: Position of this device within the uclass list (0=first,
                n-1=last)
            parent_seq: Position of this device within it siblings (0=first,
                n-1=last)
            parent_driver: Driver record of the node's parent, or None if none.
                We don't use node.parent.driver since node.parent may not be in
                the list of valid nodes
        """
        for idx, node in enumerate(self._valid_nodes):
            node.idx = idx
            node.struct_name, _ = self._scan.get_normalized_compat_name(node)
            node.var_name = conv_name_to_c(node.name)
            node.child_devs = []
            node.child_refs = {}
            node.seq = -1
            node.dev_ref = None
            node.driver = None
            node.uclass = None
            node.uclass_seq = None
            node.parent_seq = None
            node.parent_driver = None

    @staticmethod
    def get_num_cells(node):
        """Get the number of cells in addresses and sizes for this node

        Args:
            node (fdt.None): Node to check

        Returns:
            Tuple:
                Number of address cells for this node
                Number of size cells for this node
        """
        parent = node.parent
        num_addr, num_size = 2, 2
        if parent:
            addr_prop = parent.props.get('#address-cells')
            size_prop = parent.props.get('#size-cells')
            if addr_prop:
                num_addr = fdt_util.fdt32_to_cpu(addr_prop.value)
            if size_prop:
                num_size = fdt_util.fdt32_to_cpu(size_prop.value)
        return num_addr, num_size

    def scan_reg_sizes(self):
        """Scan for 64-bit 'reg' properties and update the values

        This finds 'reg' properties with 64-bit data and converts the value to
        an array of 64-values. This allows it to be output in a way that the
        C code can read.
        """
        for node in self._valid_nodes:
            reg = node.props.get('reg')
            if not reg:
                continue
            num_addr, num_size = self.get_num_cells(node)
            total = num_addr + num_size

            if reg.type != fdt.Type.INT:
                raise ValueError("Node '%s' reg property is not an int" %
                                 node.name)
            if len(reg.value) % total:
                raise ValueError(
                    "Node '%s' reg property has %d cells "
                    'which is not a multiple of na + ns = %d + %d)' %
                    (node.name, len(reg.value), num_addr, num_size))
            reg.num_addr = num_addr
            reg.num_size = num_size
            if num_addr != 1 or num_size != 1:
                reg.type = fdt.Type.INT64
                i = 0
                new_value = []
                val = reg.value
                if not isinstance(val, list):
                    val = [val]
                while i < len(val):
                    addr = fdt_util.fdt_cells_to_cpu(val[i:], reg.num_addr)
                    i += num_addr
                    size = fdt_util.fdt_cells_to_cpu(val[i:], reg.num_size)
                    i += num_size
                    new_value += [addr, size]
                reg.value = new_value

    def scan_structs(self):
        """Scan the device tree building up the C structures we will use.

        Build a dict keyed by C struct name containing a dict of Prop
        object for each struct field (keyed by property name). Where the
        same struct appears multiple times, try to use the 'widest'
        property, i.e. the one with a type which can express all others.

        Once the widest property is determined, all other properties are
        updated to match that width.

        The results are written to self._struct_data
        """
        structs = self._struct_data
        for node in self._valid_nodes:
            fields = {}

            # Get a list of all the valid properties in this node.
            for name, prop in node.props.items():
                if name not in PROP_IGNORE_LIST and name[0] != '#':
                    fields[name] = copy.deepcopy(prop)

            # If we've seen this struct_name before, update the existing struct
            if node.struct_name in structs:
                struct = structs[node.struct_name]
                for name, prop in fields.items():
                    oldprop = struct.get(name)
                    if oldprop:
                        oldprop.Widen(prop)
                    else:
                        struct[name] = prop

            # Otherwise store this as a new struct.
            else:
                structs[node.struct_name] = fields

        for node in self._valid_nodes:
            struct = structs[node.struct_name]
            for name, prop in node.props.items():
                if name not in PROP_IGNORE_LIST and name[0] != '#':
                    prop.Widen(struct[name])

    def scan_phandles(self):
        """Figure out what phandles each node uses

        We need to be careful when outputing nodes that use phandles since
        they must come after the declaration of the phandles in the C file.
        Otherwise we get a compiler error since the phandle struct is not yet
        declared.

        This function adds to each node a list of phandle nodes that the node
        depends on. This allows us to output things in the right order.
        """
        for node in self._valid_nodes:
            node.phandles = set()
            for pname, prop in node.props.items():
                if pname in PROP_IGNORE_LIST or pname[0] == '#':
                    continue
                info = self.get_phandle_argc(prop, node.name)
                if info:
                    # Process the list as pairs of (phandle, id)
                    pos = 0
                    for args in info.args:
                        phandle_cell = prop.value[pos]
                        phandle = fdt_util.fdt32_to_cpu(phandle_cell)
                        target_node = self._fdt.phandle_to_node[phandle]
                        node.phandles.add(target_node)
                        pos += 1 + args


    def generate_structs(self):
        """Generate struct defintions for the platform data

        This writes out the body of a header file consisting of structure
        definitions for node in self._valid_nodes. See the documentation in
        doc/driver-model/of-plat.rst for more information.
        """
        structs = self._struct_data
        self.out('#include <stdbool.h>\n')
        self.out('#include <linux/libfdt.h>\n')

        # Output the struct definition
        for name in sorted(structs):
            self.out('struct %s%s {\n' % (STRUCT_PREFIX, name))
            for pname in sorted(structs[name]):
                prop = structs[name][pname]
                info = self.get_phandle_argc(prop, structs[name])
                if info:
                    # For phandles, include a reference to the target
                    struct_name = 'struct phandle_%d_arg' % info.max_args
                    self.out('\t%s%s[%d]' % (tab_to(2, struct_name),
                                             conv_name_to_c(prop.name),
                                             len(info.args)))
                else:
                    ptype = TYPE_NAMES[prop.type]
                    self.out('\t%s%s' % (tab_to(2, ptype),
                                         conv_name_to_c(prop.name)))
                    if isinstance(prop.value, list):
                        self.out('[%d]' % len(prop.value))
                self.out(';\n')
            self.out('};\n')

    def _output_list(self, node, prop):
        """Output the C code for a devicetree property that holds a list

        Args:
            node (fdt.Node): Node to output
            prop (fdt.Prop): Prop to output
        """
        self.buf('{')
        vals = []
        # For phandles, output a reference to the platform data
        # of the target node.
        info = self.get_phandle_argc(prop, node.name)
        if info:
            # Process the list as pairs of (phandle, id)
            pos = 0
            for args in info.args:
                phandle_cell = prop.value[pos]
                phandle = fdt_util.fdt32_to_cpu(phandle_cell)
                target_node = self._fdt.phandle_to_node[phandle]
                arg_values = []
                for i in range(args):
                    arg_values.append(
                        str(fdt_util.fdt32_to_cpu(prop.value[pos + 1 + i])))
                pos += 1 + args
                vals.append('\t{%d, {%s}}' % (target_node.idx,
                                              ', '.join(arg_values)))
            for val in vals:
                self.buf('\n\t\t%s,' % val)
        else:
            for val in prop.value:
                vals.append(get_value(prop.type, val))

            # Put 8 values per line to avoid very long lines.
            for i in range(0, len(vals), 8):
                if i:
                    self.buf(',\n\t\t')
                self.buf(', '.join(vals[i:i + 8]))
        self.buf('}')

    def _declare_device(self, node):
        """Add a device declaration to the output

        This declares a U_BOOT_DRVINFO() for the device being processed

        Args:
            node: Node to process
        """
        self.buf('U_BOOT_DRVINFO(%s) = {\n' % node.var_name)
        self.buf('\t.name\t\t= "%s",\n' % node.struct_name)
        self.buf('\t.plat\t= &%s%s,\n' % (VAL_PREFIX, node.var_name))
        self.buf('\t.plat_size\t= sizeof(%s%s),\n' %
                 (VAL_PREFIX, node.var_name))
        idx = -1
        if node.parent and node.parent in self._valid_nodes:
            idx = node.parent.idx
        self.buf('\t.parent_idx\t= %d,\n' % idx)
        self.buf('};\n')
        self.buf('\n')

    def _output_prop(self, node, prop):
        """Output a line containing the value of a struct member

        Args:
            node (Node): Node being output
            prop (Prop): Prop object to output
        """
        if prop.name in PROP_IGNORE_LIST or prop.name[0] == '#':
            return
        member_name = conv_name_to_c(prop.name)
        self.buf('\t%s= ' % tab_to(3, '.' + member_name))

        # Special handling for lists
        if isinstance(prop.value, list):
            self._output_list(node, prop)
        else:
            self.buf(get_value(prop.type, prop.value))
        self.buf(',\n')

    def _output_values(self, node):
        """Output the definition of a device's struct values

        Args:
            node (Node): Node to output
        """
        self.buf('static struct %s%s %s%s = {\n' %
                 (STRUCT_PREFIX, node.struct_name, VAL_PREFIX, node.var_name))
        for pname in sorted(node.props):
            self._output_prop(node, node.props[pname])
        self.buf('};\n')

    def read_aliases(self):
        """Read the aliases and attach the information to self._alias

        Raises:
            ValueError: The alias path is not found
        """
        alias_node = self._fdt.GetNode('/aliases')
        if not alias_node:
            return
        re_num = re.compile('(^[a-z0-9-]+[a-z]+)([0-9]+)$')
        for prop in alias_node.props.values():
            m_alias = re_num.match(prop.name)
            if not m_alias:
                raise ValueError("Cannot decode alias '%s'" % prop.name)
            name, num = m_alias.groups()
            node = self._fdt.GetNode(prop.value)
            result = self._scan.add_uclass_alias(name, num, node)
            if result is None:
                raise ValueError("Alias '%s' path '%s' not found" %
                                 (prop.name, prop.value))
            elif result is False:
                print("Could not find uclass for alias '%s'" % prop.name)

    def process_nodes(self, need_drivers):
        nodes_to_output = list(self._valid_nodes)

        # Figure out which drivers we actually use
        self._scan.mark_used(nodes_to_output)

        for node in nodes_to_output:
            node.dev_ref = 'DM_DEVICE_REF(%s)' % node.var_name
            driver = self._scan.get_driver(node.struct_name)
            if not driver:
                if not need_drivers:
                    continue
                raise ValueError("Cannot parse/find driver for '%s'" %
                                 node.struct_name)
            node.driver = driver
            parent_driver = None
            if node.parent in self._valid_nodes:
                parent_driver = self._scan.get_driver(node.parent.struct_name)
                if not parent_driver:
                    if not need_drivers:
                        continue
                    raise ValueError(
                        "Cannot parse/find parent driver '%s' for '%s'" %
                        (node.parent.struct_name, node.struct_name))
                node.parent_seq = len(node.parent.child_devs)
                node.parent.child_devs.append(node)
                node.parent.child_refs[node.parent_seq] = \
                    '&%s->sibling_node' % node.dev_ref
                node.parent_driver = parent_driver

        for node in nodes_to_output:
            ref = '&%s->child_head' % node.dev_ref
            node.child_refs[-1] = ref
            node.child_refs[len(node.child_devs)] = ref

    def output_node(self, node):
        """Output the C code for a node

        Args:
            node (fdt.Node): node to output
        """
        self.buf('/* Node %s index %d */\n' % (node.path, node.idx))

        self._output_values(node)
        self._declare_device(node)

        self.out(''.join(self.get_buf()))

    def generate_plat(self):
        """Generate device defintions for the platform data

        This writes out C platform data initialisation data and
        U_BOOT_DRVINFO() declarations for each valid node. Where a node has
        multiple compatible strings, a #define is used to make them equivalent.

        See the documentation in doc/driver-model/of-plat.rst for more
        information.
        """
        self.out('/* Allow use of U_BOOT_DRVINFO() in this file */\n')
        self.out('#define DT_PLAT_C\n')
        self.out('\n')
        self.out('#include <common.h>\n')
        self.out('#include <dm.h>\n')
        self.out('#include <dt-structs.h>\n')
        self.out('\n')

        for node in self._valid_nodes:
            self.output_node(node)

        self.out(''.join(self.get_buf()))


# Types of output file we understand
# key: Command used to generate this file
# value: OutputFile for this command
OUTPUT_FILES = {
    'struct':
        OutputFile(Ftype.HEADER, 'dt-structs-gen.h',
                   DtbPlatdata.generate_structs,
                   'Defines the structs used to hold devicetree data'),
    'platdata':
        OutputFile(Ftype.SOURCE, 'dt-plat.c', DtbPlatdata.generate_plat,
                   'Declares the U_BOOT_DRIVER() records and platform data'),
    }


def run_steps(args, dtb_file, include_disabled, output, output_dirs, phase,
              warning_disabled=False, drivers_additional=None, basedir=None,
              scan=None):
    """Run all the steps of the dtoc tool

    Args:
        args (list): List of non-option arguments provided to the problem
        dtb_file (str): Filename of dtb file to process
        include_disabled (bool): True to include disabled nodes
        output (str): Name of output file (None for stdout)
        output_dirs (tuple of str):
            Directory to put C output files
            Directory to put H output files
        phase: The phase of U-Boot that we are generating data for, e.g. 'spl'
             or 'tpl'. None if not known
        warning_disabled (bool): True to avoid showing warnings about missing
            drivers
        drivers_additional (list): List of additional drivers to use during
            scanning
        basedir (str): Base directory of U-Boot source code. Defaults to the
            grandparent of this file's directory
        scan (src_src.Scanner): Scanner from a previous run. This can help speed
            up tests. Use None for normal operation

    Returns:
        DtbPlatdata object

    Raises:
        ValueError: if args has no command, or an unknown command
    """
    if not args:
        raise ValueError('Please specify a command: struct, platdata, all')
    if output and output_dirs and any(output_dirs):
        raise ValueError('Must specify either output or output_dirs, not both')

    if not scan:
        scan = src_scan.Scanner(basedir, warning_disabled, drivers_additional,
                                phase)
        scan.scan_drivers()
        do_process = True
    else:
        do_process = False
    plat = DtbPlatdata(scan, dtb_file, include_disabled)
    plat.scan_dtb()
    plat.scan_tree()
    plat.prepare_nodes()
    plat.scan_reg_sizes()
    plat.setup_output_dirs(output_dirs)
    plat.scan_structs()
    plat.scan_phandles()
    if do_process:
        plat.process_nodes(False)
    plat.read_aliases()

    cmds = args[0].split(',')
    if 'all' in cmds:
        cmds = sorted(OUTPUT_FILES.keys())
    for cmd in cmds:
        outfile = OUTPUT_FILES.get(cmd)
        if not outfile:
            raise ValueError("Unknown command '%s': (use: %s)" %
                             (cmd, ', '.join(sorted(OUTPUT_FILES.keys()))))
        plat.setup_output(outfile.ftype,
                          outfile.fname if output_dirs else output)
        plat.out_header(outfile)
        outfile.method(plat)
    plat.finish_output()
    return plat
