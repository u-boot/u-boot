#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:	GPL-2.0+
#

import copy
from optparse import OptionError, OptionParser
import os
import sys

import fdt_util

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '../patman'))

# Bring in either the normal fdt library (which relies on libfdt) or the
# fallback one (which uses fdtget and is slower). Both provide the same
# interfface for this file to use.
try:
    from fdt import Fdt
    import fdt
    have_libfdt = True
except ImportError:
    have_libfdt = False
    from fdt_fallback import Fdt
    import fdt_fallback as fdt

import struct

# When we see these properties we ignore them - i.e. do not create a structure member
PROP_IGNORE_LIST = [
    '#address-cells',
    '#gpio-cells',
    '#size-cells',
    'compatible',
    'linux,phandle',
    "status",
    'phandle',
    'u-boot,dm-pre-reloc',
]

# C type declarations for the tyues we support
TYPE_NAMES = {
    fdt_util.TYPE_INT: 'fdt32_t',
    fdt_util.TYPE_BYTE: 'unsigned char',
    fdt_util.TYPE_STRING: 'const char *',
    fdt_util.TYPE_BOOL: 'bool',
};

STRUCT_PREFIX = 'dtd_'
VAL_PREFIX = 'dtv_'

def Conv_name_to_c(name):
    """Convert a device-tree name to a C identifier

    Args:
        name:   Name to convert
    Return:
        String containing the C version of this name
    """
    str = name.replace('@', '_at_')
    str = str.replace('-', '_')
    str = str.replace(',', '_')
    str = str.replace('/', '__')
    return str

def TabTo(num_tabs, str):
    if len(str) >= num_tabs * 8:
        return str + ' '
    return str + '\t' * (num_tabs - len(str) / 8)

class DtbPlatdata:
    """Provide a means to convert device tree binary data to platform data

    The output of this process is C structures which can be used in space-
    constrained encvironments where the ~3KB code overhead of device tree
    code is not affordable.

    Properties:
        fdt: Fdt object, referencing the device tree
        _dtb_fname: Filename of the input device tree binary file
        _valid_nodes: A list of Node object with compatible strings
        _options: Command-line options
        _phandle_node: A dict of nodes indexed by phandle number (1, 2...)
        _outfile: The current output file (sys.stdout or a real file)
        _lines: Stashed list of output lines for outputting in the future
        _phandle_node: A dict of Nodes indexed by phandle (an integer)
    """
    def __init__(self, dtb_fname, options):
        self._dtb_fname = dtb_fname
        self._valid_nodes = None
        self._options = options
        self._phandle_node = {}
        self._outfile = None
        self._lines = []

    def SetupOutput(self, fname):
        """Set up the output destination

        Once this is done, future calls to self.Out() will output to this
        file.

        Args:
            fname: Filename to send output to, or '-' for stdout
        """
        if fname == '-':
            self._outfile = sys.stdout
        else:
            self._outfile = open(fname, 'w')

    def Out(self, str):
        """Output a string to the output file

        Args:
            str: String to output
        """
        self._outfile.write(str)

    def Buf(self, str):
        """Buffer up a string to send later

        Args:
            str: String to add to our 'buffer' list
        """
        self._lines.append(str)

    def GetBuf(self):
        """Get the contents of the output buffer, and clear it

        Returns:
            The output buffer, which is then cleared for future use
        """
        lines = self._lines
        self._lines = []
        return lines

    def GetValue(self, type, value):
        """Get a value as a C expression

        For integers this returns a byte-swapped (little-endian) hex string
        For bytes this returns a hex string, e.g. 0x12
        For strings this returns a literal string enclosed in quotes
        For booleans this return 'true'

        Args:
            type: Data type (fdt_util)
            value: Data value, as a string of bytes
        """
        if type == fdt_util.TYPE_INT:
            return '%#x' % fdt_util.fdt32_to_cpu(value)
        elif type == fdt_util.TYPE_BYTE:
            return '%#x' % ord(value[0])
        elif type == fdt_util.TYPE_STRING:
            return '"%s"' % value
        elif type == fdt_util.TYPE_BOOL:
            return 'true'

    def GetCompatName(self, node):
        """Get a node's first compatible string as a C identifier

        Args:
            node: Node object to check
        Return:
            C identifier for the first compatible string
        """
        compat = node.props['compatible'].value
        if type(compat) == list:
            compat = compat[0]
        return Conv_name_to_c(compat)

    def ScanDtb(self):
        """Scan the device tree to obtain a tree of notes and properties

        Once this is done, self.fdt.GetRoot() can be called to obtain the
        device tree root node, and progress from there.
        """
        self.fdt = Fdt(self._dtb_fname)
        self.fdt.Scan()

    def ScanTree(self):
        """Scan the device tree for useful information

        This fills in the following properties:
            _phandle_node: A dict of Nodes indexed by phandle (an integer)
            _valid_nodes: A list of nodes we wish to consider include in the
                platform data
        """
        node_list = []
        self._phandle_node = {}
        for node in self.fdt.GetRoot().subnodes:
            if 'compatible' in node.props:
                status = node.props.get('status')
                if (not options.include_disabled and not status or
                    status.value != 'disabled'):
                    node_list.append(node)
                    phandle_prop = node.props.get('phandle')
                    if phandle_prop:
                        phandle = phandle_prop.GetPhandle()
                        self._phandle_node[phandle] = node

        self._valid_nodes = node_list

    def IsPhandle(self, prop):
        """Check if a node contains phandles

        We have no reliable way of detecting whether a node uses a phandle
        or not. As an interim measure, use a list of known property names.

        Args:
            prop: Prop object to check
        Return:
            True if the object value contains phandles, else False
        """
        if prop.name in ['clocks']:
            return True
        return False

    def ScanStructs(self):
        """Scan the device tree building up the C structures we will use.

        Build a dict keyed by C struct name containing a dict of Prop
        object for each struct field (keyed by property name). Where the
        same struct appears multiple times, try to use the 'widest'
        property, i.e. the one with a type which can express all others.

        Once the widest property is determined, all other properties are
        updated to match that width.
        """
        structs = {}
        for node in self._valid_nodes:
            node_name = self.GetCompatName(node)
            fields = {}

            # Get a list of all the valid properties in this node.
            for name, prop in node.props.iteritems():
                if name not in PROP_IGNORE_LIST and name[0] != '#':
                    fields[name] = copy.deepcopy(prop)

            # If we've seen this node_name before, update the existing struct.
            if node_name in structs:
                struct = structs[node_name]
                for name, prop in fields.iteritems():
                    oldprop = struct.get(name)
                    if oldprop:
                        oldprop.Widen(prop)
                    else:
                        struct[name] = prop

            # Otherwise store this as a new struct.
            else:
                structs[node_name] = fields

        upto = 0
        for node in self._valid_nodes:
            node_name = self.GetCompatName(node)
            struct = structs[node_name]
            for name, prop in node.props.iteritems():
                if name not in PROP_IGNORE_LIST and name[0] != '#':
                    prop.Widen(struct[name])
            upto += 1
        return structs

    def GenerateStructs(self, structs):
        """Generate struct defintions for the platform data

        This writes out the body of a header file consisting of structure
        definitions for node in self._valid_nodes. See the documentation in
        README.of-plat for more information.
        """
        self.Out('#include <stdbool.h>\n')
        self.Out('#include <libfdt.h>\n')

        # Output the struct definition
        for name in sorted(structs):
            self.Out('struct %s%s {\n' % (STRUCT_PREFIX, name));
            for pname in sorted(structs[name]):
                prop = structs[name][pname]
                if self.IsPhandle(prop):
                    # For phandles, include a reference to the target
                    self.Out('\t%s%s[%d]' % (TabTo(2, 'struct phandle_2_cell'),
                                             Conv_name_to_c(prop.name),
                                             len(prop.value) / 2))
                else:
                    ptype = TYPE_NAMES[prop.type]
                    self.Out('\t%s%s' % (TabTo(2, ptype),
                                         Conv_name_to_c(prop.name)))
                    if type(prop.value) == list:
                        self.Out('[%d]' % len(prop.value))
                self.Out(';\n')
            self.Out('};\n')

    def GenerateTables(self):
        """Generate device defintions for the platform data

        This writes out C platform data initialisation data and
        U_BOOT_DEVICE() declarations for each valid node. See the
        documentation in README.of-plat for more information.
        """
        self.Out('#include <common.h>\n')
        self.Out('#include <dm.h>\n')
        self.Out('#include <dt-structs.h>\n')
        self.Out('\n')
        node_txt_list = []
        for node in self._valid_nodes:
            struct_name = self.GetCompatName(node)
            var_name = Conv_name_to_c(node.name)
            self.Buf('static struct %s%s %s%s = {\n' %
                (STRUCT_PREFIX, struct_name, VAL_PREFIX, var_name))
            for pname, prop in node.props.iteritems():
                if pname in PROP_IGNORE_LIST or pname[0] == '#':
                    continue
                ptype = TYPE_NAMES[prop.type]
                member_name = Conv_name_to_c(prop.name)
                self.Buf('\t%s= ' % TabTo(3, '.' + member_name))

                # Special handling for lists
                if type(prop.value) == list:
                    self.Buf('{')
                    vals = []
                    # For phandles, output a reference to the platform data
                    # of the target node.
                    if self.IsPhandle(prop):
                        # Process the list as pairs of (phandle, id)
                        it = iter(prop.value)
                        for phandle_cell, id_cell in zip(it, it):
                            phandle = fdt_util.fdt32_to_cpu(phandle_cell)
                            id = fdt_util.fdt32_to_cpu(id_cell)
                            target_node = self._phandle_node[phandle]
                            name = Conv_name_to_c(target_node.name)
                            vals.append('{&%s%s, %d}' % (VAL_PREFIX, name, id))
                    else:
                        for val in prop.value:
                            vals.append(self.GetValue(prop.type, val))
                    self.Buf(', '.join(vals))
                    self.Buf('}')
                else:
                    self.Buf(self.GetValue(prop.type, prop.value))
                self.Buf(',\n')
            self.Buf('};\n')

            # Add a device declaration
            self.Buf('U_BOOT_DEVICE(%s) = {\n' % var_name)
            self.Buf('\t.name\t\t= "%s",\n' % struct_name)
            self.Buf('\t.platdata\t= &%s%s,\n' % (VAL_PREFIX, var_name))
            self.Buf('\t.platdata_size\t= sizeof(%s%s),\n' %
                     (VAL_PREFIX, var_name))
            self.Buf('};\n')
            self.Buf('\n')

            # Output phandle target nodes first, since they may be referenced
            # by others
            if 'phandle' in node.props:
                self.Out(''.join(self.GetBuf()))
            else:
                node_txt_list.append(self.GetBuf())

        # Output all the nodes which are not phandle targets themselves, but
        # may reference them. This avoids the need for forward declarations.
        for node_txt in node_txt_list:
            self.Out(''.join(node_txt))


if __name__ != "__main__":
    pass

parser = OptionParser()
parser.add_option('-d', '--dtb-file', action='store',
                  help='Specify the .dtb input file')
parser.add_option('--include-disabled', action='store_true',
                  help='Include disabled nodes')
parser.add_option('-o', '--output', action='store', default='-',
                  help='Select output filename')
(options, args) = parser.parse_args()

if not args:
    raise ValueError('Please specify a command: struct, platdata')

plat = DtbPlatdata(options.dtb_file, options)
plat.ScanDtb()
plat.ScanTree()
plat.SetupOutput(options.output)
structs = plat.ScanStructs()

for cmd in args[0].split(','):
    if cmd == 'struct':
        plat.GenerateStructs(structs)
    elif cmd == 'platdata':
        plat.GenerateTables()
    else:
        raise ValueError("Unknown command '%s': (use: struct, platdata)" % cmd)
