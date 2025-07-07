# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

"""Entry-type module for producing a FIT"""

import glob
import os

import libfdt
import os

from binman.entry import Entry, EntryArg
from binman.etype.section import Entry_section
from binman import elf
from dtoc import fdt_util
from dtoc.fdt import Fdt
from u_boot_pylib import tools

# Supported operations, with the fit,operation property
OP_GEN_FDT_NODES, OP_SPLIT_ELF = range(2)
OPERATIONS = {
    'gen-fdt-nodes': OP_GEN_FDT_NODES,
    'split-elf': OP_SPLIT_ELF,
    }

# pylint: disable=invalid-name
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

        fit,align
            Indicates what alignment to use for the FIT and its external data,
            and provides the alignment to use. This is passed to mkimage via
            the -B flag.

        fit,fdt-list
            Indicates the entry argument which provides the list of device tree
            files for the gen-fdt-nodes operation (as below). This is often
            `of-list` meaning that `-a of-list="dtb1 dtb2..."` should be passed
            to binman.

        fit,fdt-list-val
            As an alternative to fit,fdt-list the list of device tree files
            can be provided in this property as a string list, e.g.::

                fit,fdt-list-val = "dtb1", "dtb2";

        fit,fdt-list-dir
            As an alternative to fit,fdt-list the list of device tree files
            can be provided as a directory. Each .dtb file in the directory is
            processed, , e.g.::

                fit,fdt-list-dir = "arch/arm/dts";

            In this case the input directories are ignored and all devicetree
            files must be in that directory.

        fit,sign
            Enable signing FIT images via mkimage as described in
            verified-boot.rst. If the property is found, the private keys path
            is detected among binman include directories and passed to mkimage
            via  -k flag. All the keys required for signing FIT must be
            available at time of signing and must be located in single include
            directory.

        fit,encrypt
            Enable data encryption in FIT images via mkimage. If the property
            is found, the keys path is detected among binman include
            directories and passed to mkimage via  -k flag. All the keys
            required for encrypting the FIT must be available at the time of
            encrypting and must be located in a single include directory.

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

    DEFAULT-NAME:
        Name of the default fdt, as provided by the 'default-dt' entry argument

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

    split-elf
        Split an ELF file into a separate node for each segment.

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
                fit,compatible;    // optional
            };
        };

    This tells binman to create nodes `config-1` and `config-2`, i.e. a config
    for each of your two files.

    It is also possible to use NAME in the node names so that the FDT files name
    will be used instead of the sequence number. This can be useful to identify
    easily at runtime in U-Boot, the config to be used::

        configurations {
            default = "@config-DEFAULT-NAME";
            @config-NAME {
                description = "NAME";
                firmware = "atf";
                loadables = "uboot";
                fdt = "fdt-NAME";
                fit,compatible;    // optional
            };
        };

    Note that if no devicetree files are provided (with '-a of-list' as above)
    then no nodes will be generated.

    The 'fit,compatible' property (if present) is replaced with the compatible
    string from the root node of the devicetree, so that things work correctly
    with FIT's configuration-matching algortihm.

    Dealing with phases
    ~~~~~~~~~~~~~~~~~~~

    FIT can be used to load firmware. In this case it may be necessary to run
    the devicetree for each model through fdtgrep to remove unwanted properties.
    The 'fit,fdt-phase' property can be provided to indicate the phase for which
    the devicetree is intended.

    For example this indicates that the FDT should be processed for VPL::

        images {
            @fdt-SEQ {
                description = "fdt-NAME";
                type = "flat_dt";
                compression = "none";
                fit,fdt-phase = "vpl";
            };
        };

    Using this mechanism, it is possible to generate a FIT which can provide VPL
    images for multiple models, with TPL selecting the correct model to use. The
    same approach can of course be used for SPL images.

    Note that the `of-spl-remove-props` entryarg can be used to indicate
    additional properties to remove. It is often used to remove properties like
    `clock-names` and `pinctrl-names` which are not needed in SPL builds. This
    value is automatically passed to binman by the U-Boot build.

    See :ref:`fdtgrep_filter` for more information.

    Generating nodes from an ELF file (split-elf)
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    This uses the node as a template to generate multiple nodes. The following
    special properties are available:

    split-elf
        Split an ELF file into a separate node for each segment. This uses the
        node as a template to generate multiple nodes. The following special
        properties are available:

        fit,load
            Generates a `load = <...>` property with the load address of the
            segment

        fit,entry
            Generates a `entry = <...>` property with the entry address of the
            ELF. This is only produced for the first entry

        fit,data
            Generates a `data = <...>` property with the contents of the segment

        fit,firmware
            Generates a `firmware = <...>` property. Provides a list of possible
            nodes to be used as the `firmware` property value. The first valid
            node is picked as the firmware. Any remaining valid nodes is
            prepended to the `loadable` property generated by `fit,loadables`

        fit,loadables
            Generates a `loadable = <...>` property with a list of the generated
            nodes (including all nodes if this operation is used multiple times)


    Here is an example showing ATF, TEE and a device tree all combined::

        fit {
            description = "test-desc";
            #address-cells = <1>;
            fit,fdt-list = "of-list";

            images {
                u-boot {
                    description = "U-Boot (64-bit)";
                    type = "standalone";
                    os = "U-Boot";
                    arch = "arm64";
                    compression = "none";
                    load = <CONFIG_TEXT_BASE>;
                    u-boot-nodtb {
                    };
                };
                @fdt-SEQ {
                    description = "fdt-NAME.dtb";
                    type = "flat_dt";
                    compression = "none";
                };
                @atf-SEQ {
                    fit,operation = "split-elf";
                    description = "ARM Trusted Firmware";
                    type = "firmware";
                    arch = "arm64";
                    os = "arm-trusted-firmware";
                    compression = "none";
                    fit,load;
                    fit,entry;
                    fit,data;

                    atf-bl31 {
                    };
                    hash {
                        algo = "sha256";
                    };
                };

                @tee-SEQ {
                    fit,operation = "split-elf";
                    description = "TEE";
                    type = "tee";
                    arch = "arm64";
                    os = "tee";
                    compression = "none";
                    fit,load;
                    fit,entry;
                    fit,data;

                    tee-os {
                    };
                    hash {
                        algo = "sha256";
                    };
                };
            };

            configurations {
                default = "@config-DEFAULT-SEQ";
                @config-SEQ {
                    description = "conf-NAME.dtb";
                    fdt = "fdt-SEQ";
                    fit,firmware = "atf-1", "u-boot";
                    fit,loadables;
                };
            };
        };

    If ATF-BL31 is available, this generates a node for each segment in the
    ELF file, for example::

        images {
            atf-1 {
                data = <...contents of first segment...>;
                data-offset = <0x00000000>;
                entry = <0x00040000>;
                load = <0x00040000>;
                compression = "none";
                os = "arm-trusted-firmware";
                arch = "arm64";
                type = "firmware";
                description = "ARM Trusted Firmware";
                hash {
                    algo = "sha256";
                    value = <...hash of first segment...>;
                };
            };
            atf-2 {
                data = <...contents of second segment...>;
                load = <0xff3b0000>;
                compression = "none";
                os = "arm-trusted-firmware";
                arch = "arm64";
                type = "firmware";
                description = "ARM Trusted Firmware";
                hash {
                    algo = "sha256";
                    value = <...hash of second segment...>;
                };
            };
        };

    The same applies for OP-TEE if that is available.

    If each binary is not available, the relevant template node (@atf-SEQ or
    @tee-SEQ) is removed from the output.

    This also generates a `config-xxx` node for each device tree in `of-list`.
    Note that the U-Boot build system uses `-a of-list=$(CONFIG_OF_LIST)`
    so you can use `CONFIG_OF_LIST` to define that list. In this example it is
    set up for `firefly-rk3399` with a single device tree and the default set
    with `-a default-dt=$(CONFIG_DEFAULT_DEVICE_TREE)`, so the resulting output
    is::

        configurations {
            default = "config-1";
            config-1 {
                loadables = "u-boot", "atf-2", "atf-3", "tee-1", "tee-2";
                description = "rk3399-firefly.dtb";
                fdt = "fdt-1";
                firmware = "atf-1";
            };
        };

    U-Boot SPL can then load the firmware (ATF) and all the loadables (U-Boot
    proper, ATF and TEE), then proceed with the boot.
    """
    def __init__(self, section, etype, node):
        """
        Members:
            _fit (str): FIT file being built
            _fit_props (list of str): 'fit,...' properties found in the
                top-level node
            _fdts (list of str): Filenames of .dtb files to process
            _fdt_dir (str): Directory to scan to find .dtb files, or None
            _fit_list_prop (str): Name of the EntryArg containing a list of .dtb
                files
            _fit_default_dt (str): Name of the EntryArg containing the default
                .dtb file
            _entries (dict of entries): from Entry_section:
                key: relative path to entry Node (from the base of the FIT)
                value: Entry_section object comprising the contents of this
                    node
            _priv_entries (dict of entries): Internal copy of _entries which
                includes 'generator' entries which are used to create the FIT,
                but should not be processed as real entries. This is set up once
                we have the entries
            _loadables (list of str): List of generated split-elf nodes, each
                a node name
            _remove_props (list of str): Value of of-spl-remove-props EntryArg,
                the list of properties to remove with fdtgrep
            mkimage (Bintool): mkimage tool
            fdtgrep (Bintool): fdtgrep tool
        """
        super().__init__(section, etype, node)
        self._fit = None
        self._fit_props = {}
        self._fdts = None
        self._fdt_dir = None
        self._fit_list_prop = None
        self._fit_default_dt = None
        self._priv_entries = {}
        self._loadables = []
        self._remove_props = []
        props = self.GetEntryArgsOrProps(
            [EntryArg('of-spl-remove-props', str)], required=False)[0]
        if props:
            self._remove_props = props.split()
        self.mkimage = None
        self.fdtgrep = None
        self._fit_sign = None

    def ReadNode(self):
        super().ReadNode()
        for pname, prop in self._node.props.items():
            if pname.startswith('fit,'):
                self._fit_props[pname] = prop
        self._fit_list_prop = self._fit_props.get('fit,fdt-list')
        if self._fit_list_prop:
            fdts = self.GetEntryArgsOrProps(
                [EntryArg(self._fit_list_prop.value, str)])[0]
            if fdts is not None:
                self._fdts = fdts.split()
        else:
            self._fdt_dir = fdt_util.GetString(self._node, 'fit,fdt-list-dir')
            if self._fdt_dir:
                indir = tools.get_input_filename(self._fdt_dir)
                if indir:
                    tools.append_input_dirs(indir)
                fdts = glob.glob('*.dtb', root_dir=indir)
                self._fdts = [os.path.splitext(f)[0] for f in sorted(fdts)]
            else:
                self._fdts = fdt_util.GetStringList(self._node,
                                                    'fit,fdt-list-val')

        self._fit_default_dt = self.GetEntryArgsOrProps([EntryArg('default-dt',
                                                                  str)])[0]

    def _get_operation(self, node):
        """Get the operation referenced by a subnode

        Args:
            node (Node): Subnode (of the FIT) to check

        Returns:
            int: Operation to perform

        Raises:
            ValueError: Invalid operation name
        """
        oper_name = node.props.get('fit,operation')
        if not oper_name:
            return OP_GEN_FDT_NODES
        oper = OPERATIONS.get(oper_name.value)
        if oper is None:
            self._raise_subnode(node, f"Unknown operation '{oper_name.value}'")
        return oper

    def ReadEntries(self):
        def _add_entries(base_node, depth, node):
            """Add entries for any nodes that need them

            Args:
                base_node: Base Node of the FIT (with 'description' property)
                depth: Current node depth (0 is the base 'fit' node)
                node: Current node to process

            Here we only need to provide binman entries which are used to define
            the 'data' for each image. We create an entry_Section for each.
            """
            rel_path = node.path[len(base_node.path):]
            in_images = rel_path.startswith('/images')
            has_images = depth == 2 and in_images
            if has_images:
                # This node is a FIT subimage node (e.g. "/images/kernel")
                # containing content nodes. We collect the subimage nodes and
                # section entries for them here to merge the content subnodes
                # together and put the merged contents in the subimage node's
                # 'data' property later.
                entry = Entry.Create(self, node, etype='section')
                entry.ReadNode()
                # The hash subnodes here are for mkimage, not binman.
                entry.SetUpdateHash(False)
                image_name = rel_path[len('/images/'):]
                self._entries[image_name] = entry

            for subnode in node.subnodes:
                _add_entries(base_node, depth + 1, subnode)

        _add_entries(self._node, 0, self._node)

        # Keep a copy of all entries, including generator entries, since those
        # are removed from self._entries later.
        self._priv_entries = dict(self._entries)

    def _get_keys_dir(self, data):
        """Detect private and encryption keys path among binman include directories

        Args:
            data: FIT image in binary format

        Returns:
            str: Single path containing all keys found or None

        Raises:
            ValueError: Filename 'rsa2048.key' not found in input path
            ValueError: Multiple key paths found
            ValueError: 'dir/rsa2048' is a path not a filename
        """
        def _find_keys_dir(node):
            for subnode in node.subnodes:
                if (subnode.name.startswith('signature') or
                    subnode.name.startswith('cipher')):
                    hint = subnode.props['key-name-hint'].value
                    if '/' in hint:
                        self.Raise(f"'{hint}' is a path not a filename")
                    name = tools.get_input_filename(
                        f"{hint}.key" if subnode.name.startswith('signature')
                        else f"{hint}.bin")
                    path = os.path.dirname(name)
                    if path not in paths:
                        paths.append(path)
                else:
                    _find_keys_dir(subnode)
            return None

        fdt = Fdt.FromData(data)
        fdt.Scan()

        paths = []

        _find_keys_dir(fdt.GetRoot())

        if len(paths) > 1:
            self.Raise("multiple key paths found (%s)" % ",".join(paths))

        return paths[0] if len(paths) else None

    def BuildSectionData(self, required):
        """Build FIT entry contents

        This adds the 'data' properties to the input ITB (Image-tree Binary)
        then runs mkimage to process it.

        Args:
            required (bool): True if the data must be present, False if it is OK
                to return None

        Returns:
            bytes: Contents of the section
        """
        data = self._build_input()
        uniq = self.GetUniqueName()
        input_fname = tools.get_output_filename(f'{uniq}.itb')
        output_fname = tools.get_output_filename(f'{uniq}.fit')
        tools.write_file(input_fname, data)
        tools.write_file(output_fname, data)

        args = {}
        ext_offset = self._fit_props.get('fit,external-offset')
        if ext_offset is not None:
            args = {
                'external': True,
                'pad': fdt_util.fdt32_to_cpu(ext_offset.value)
                }
        align = self._fit_props.get('fit,align')
        if align is not None:
            args.update({'align': fdt_util.fdt32_to_cpu(align.value)})
        if (self._fit_props.get('fit,sign') is not None or
            self._fit_props.get('fit,encrypt') is not None):
            args.update({'keys_dir': self._get_keys_dir(data)})
        if self.mkimage.run(reset_timestamp=True, output_fname=output_fname,
                            **args) is None:
            if not self.GetAllowMissing():
                self.Raise("Missing tool: 'mkimage'")
            # Bintool is missing; just use empty data as the output
            self.record_missing_bintool(self.mkimage)
            return tools.get_bytes(0, 1024)

        return tools.read_file(output_fname)

    def _raise_subnode(self, node, msg):
        """Raise an error with a paticular FIT subnode

        Args:
            node (Node): FIT subnode containing the error
            msg (str): Message to report

        Raises:
            ValueError, as requested
        """
        rel_path = node.path[len(self._node.path) + 1:]
        self.Raise(f"subnode '{rel_path}': {msg}")

    def _run_fdtgrep(self, infile, phase, outfile):
        """Run fdtgrep to create the dtb for a phase

        Args:
            infile (str): Input filename containing the full FDT contents (with
                all nodes and properties)
            phase (str): Phase to generate for ('tpl', 'vpl', 'spl')
            outfile (str): Output filename to write the grepped FDT contents to
                (with only neceesary nodes and properties)

        Returns:
            str or bytes: Resulting stdout from fdtgrep
        """
        return self.fdtgrep.create_for_phase(infile, phase, outfile,
                                             self._remove_props)

    def _build_input(self):
        """Finish the FIT by adding the 'data' properties to it

        Returns:
            bytes: New fdt contents
        """
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
                    default_dt = self._fit_default_dt
                    if not default_dt:
                        self.Raise("Generated 'default' node requires default-dt entry argument")
                    if default_dt not in self._fdts:
                        if self._fdt_dir:
                            default_dt = os.path.basename(default_dt)
                        if default_dt not in self._fdts:
                            self.Raise(
                                f"default-dt entry argument '{self._fit_default_dt}' "
                                f"not found in fdt list: {', '.join(self._fdts)}")
                    seq = self._fdts.index(default_dt)
                    val = val[1:].replace('DEFAULT-SEQ', str(seq + 1))
                    val = val.replace('DEFAULT-NAME', self._fit_default_dt)
                    fsw.property_string(pname, val)
                    return
            elif pname.startswith('fit,'):
                # Ignore these, which are commands for binman to process
                return
            elif pname in ['offset', 'size', 'image-pos']:
                # Don't add binman's calculated properties
                return
            fsw.property(pname, prop.bytes)

        def _process_firmware_prop(node):
            """Process optional fit,firmware property

            Picks the first valid entry for use as the firmware, remaining valid
            entries is prepended to loadables

            Args:
                node (Node): Generator node to process

            Returns:
                firmware (str): Firmware or None
                result (list): List of remaining loadables
            """
            val = fdt_util.GetStringList(node, 'fit,firmware')
            if val is None:
                return None, loadables
            valid_entries = list(loadables)
            for name, entry in self.GetEntries().items():
                missing = []
                entry.CheckMissing(missing)
                entry.CheckOptional(missing)
                if not missing:
                    valid_entries.append(name)
            firmware = None
            result = []
            for name in val:
                if name in valid_entries:
                    if not firmware:
                        firmware = name
                    elif name not in result:
                        result.append(name)
            for name in loadables:
                if name != firmware and name not in result:
                    result.append(name)
            return firmware, result

        def _gen_fdt_nodes(node, depth, in_images):
            """Generate FDT nodes

            This creates one node for each member of self._fdts using the
            provided template. If a property value contains 'NAME' it is
            replaced with the filename of the FDT. If a property value contains
            SEQ it is replaced with the node sequence number, where 1 is the
            first.

            Args:
                node (Node): Generator node to process
                depth: Current node depth (0 is the base 'fit' node)
                in_images: True if this is inside the 'images' node, so that
                    'data' properties should be generated
            """
            if self._fdts:
                firmware, fit_loadables = _process_firmware_prop(node)
                # Generate nodes for each FDT
                for seq, fdt_fname in enumerate(self._fdts):
                    node_name = node.name[1:].replace('SEQ', str(seq + 1))
                    node_name = node_name.replace('NAME', fdt_fname)
                    if self._fdt_dir:
                        fname = os.path.join(self._fdt_dir, fdt_fname + '.dtb')
                    else:
                        fname = tools.get_input_filename(fdt_fname + '.dtb')
                    fdt_phase = None
                    with fsw.add_node(node_name):
                        for pname, prop in node.props.items():
                            if pname == 'fit,firmware':
                                if firmware:
                                    fsw.property_string('firmware', firmware)
                            elif pname == 'fit,loadables':
                                val = '\0'.join(fit_loadables) + '\0'
                                fsw.property('loadables', val.encode('utf-8'))
                            elif pname == 'fit,operation':
                                pass
                            elif pname == 'fit,compatible':
                                fdt_phase = fdt_util.GetString(node, pname)
                                data = tools.read_file(fname)
                                fdt = Fdt.FromData(data)
                                fdt.Scan()
                                prop = fdt.GetRoot().props['compatible']
                                fsw.property('compatible', prop.bytes)
                            elif pname == 'fit,fdt-phase':
                                fdt_phase = fdt_util.GetString(node, pname)
                            elif pname.startswith('fit,'):
                                self._raise_subnode(
                                    node, f"Unknown directive '{pname}'")
                            else:
                                val = prop.bytes.replace(
                                    b'NAME', tools.to_bytes(fdt_fname))
                                val = val.replace(
                                    b'SEQ', tools.to_bytes(str(seq + 1)))
                                fsw.property(pname, val)

                        # Add data for 'images' nodes (but not 'config')
                        if depth == 1 and in_images:
                            if fdt_phase:
                                leaf = os.path.basename(fdt_fname)
                                phase_fname = tools.get_output_filename(
                                    f'{leaf}-{fdt_phase}.dtb')
                                self._run_fdtgrep(fname, fdt_phase, phase_fname)
                                data = tools.read_file(phase_fname)
                            else:
                                data = tools.read_file(fname)
                            fsw.property('data', data)

                        for subnode in node.subnodes:
                            with fsw.add_node(subnode.name):
                                _add_node(node, depth + 1, subnode)
            else:
                if self._fdts is None:
                    if self._fit_list_prop:
                        self.Raise('Generator node requires '
                            f"'{self._fit_list_prop.value}' entry argument")
                    else:
                        self.Raise("Generator node requires 'fit,fdt-list' property")

        def _gen_split_elf(node, depth, segments, entry_addr):
            """Add nodes for the ELF file, one per group of contiguous segments

            Args:
                node (Node): Node to replace (in the FIT being built)
                depth: Current node depth (0 is the base 'fit' node)
                segments (list): list of segments, each:
                    int: Segment number (0 = first)
                    int: Start address of segment in memory
                    bytes: Contents of segment
                entry_addr (int): entry address of ELF file
            """
            for (seq, start, data) in segments:
                node_name = node.name[1:].replace('SEQ', str(seq + 1))
                with fsw.add_node(node_name):
                    loadables.append(node_name)
                    for pname, prop in node.props.items():
                        if not pname.startswith('fit,'):
                            fsw.property(pname, prop.bytes)
                        elif pname == 'fit,load':
                            fsw.property_u32('load', start)
                        elif pname == 'fit,entry':
                            if seq == 0:
                                fsw.property_u32('entry', entry_addr)
                        elif pname == 'fit,data':
                            fsw.property('data', bytes(data))
                        elif pname != 'fit,operation':
                            self._raise_subnode(
                                node, f"Unknown directive '{pname}'")

                    for subnode in node.subnodes:
                        with fsw.add_node(subnode.name):
                            _add_node(node, depth + 1, subnode)

        def _gen_node(node, depth, in_images, entry):
            """Generate nodes from a template

            This creates one or more nodes depending on the fit,operation being
            used.

            For OP_GEN_FDT_NODES it creates one node for each member of
            self._fdts using the provided template. If a property value contains
            'NAME' it is replaced with the filename of the FDT. If a property
            value contains SEQ it is replaced with the node sequence number,
            where 1 is the first.

            For OP_SPLIT_ELF it emits one node for each section in the ELF file.
            If the file is missing, nothing is generated.

            Args:
                node (Node): Generator node to process
                depth (int): Current node depth (0 is the base 'fit' node)
                in_images (bool): True if this is inside the 'images' node, so
                    that 'data' properties should be generated
                entry (entry_Section): Entry for the section containing the
                    contents of this node
            """
            oper = self._get_operation(node)
            if oper == OP_GEN_FDT_NODES:
                _gen_fdt_nodes(node, depth, in_images)
            elif oper == OP_SPLIT_ELF:
                # Entry_section.ObtainContents() either returns True or
                # raises an exception.
                missing_opt_list = []
                entry.ObtainContents()
                entry.Pack(0)
                entry.CheckMissing(missing_opt_list)
                entry.CheckOptional(missing_opt_list)

                # If any pieces are missing, skip this. The missing entries will
                # show an error
                if not missing_opt_list:
                    segs = entry.read_elf_segments()
                    if segs:
                        segments, entry_addr = segs
                    else:
                        elf_data = entry.GetData()
                        try:
                            segments, entry_addr = (
                                    elf.read_loadable_segments(elf_data))
                        except ValueError as exc:
                            self._raise_subnode(
                                node, f'Failed to read ELF file: {str(exc)}')

                    _gen_split_elf(node, depth, segments, entry_addr)

        def _add_node(base_node, depth, node):
            """Add nodes to the output FIT

            Args:
                base_node (Node): Base Node of the FIT (with 'description'
                    property)
                depth (int): Current node depth (0 is the base 'fit' node)
                node (Node): Current node to process

            There are two cases to deal with:
                - hash and signature nodes which become part of the FIT
                - binman entries which are used to define the 'data' for each
                  image, so don't appear in the FIT
            """
            # Copy over all the relevant properties
            for pname, prop in node.props.items():
                _process_prop(pname, prop)

            rel_path = node.path[len(base_node.path):]
            in_images = rel_path.startswith('/images')

            has_images = depth == 2 and in_images
            if has_images:
                image_name = rel_path[len('/images/'):]
                entry = self._priv_entries[image_name]
                data = entry.GetData()
                fsw.property('data', bytes(data))

            for subnode in node.subnodes:
                if has_images and not self.IsSpecialSubnode(subnode):
                    # This subnode is a content node not meant to appear in
                    # the FIT (e.g. "/images/kernel/u-boot"), so don't call
                    # fsw.add_node() or _add_node() for it.
                    pass
                elif self.GetImage().generate and subnode.name.startswith('@'):
                    entry = self._priv_entries.get(subnode.name)
                    _gen_node(subnode, depth, in_images, entry)
                    # This is a generator (template) entry, so remove it from
                    # the list of entries used by PackEntries(), etc. Otherwise
                    # it will appear in the binman output
                    to_remove.append(subnode.name)
                else:
                    with fsw.add_node(subnode.name):
                        _add_node(base_node, depth + 1, subnode)

        # Build a new tree with all nodes and properties starting from the
        # entry node
        fsw = libfdt.FdtSw()
        fsw.INC_SIZE = 65536
        fsw.finish_reservemap()
        to_remove = []
        loadables = []
        with fsw.add_node(''):
            _add_node(self._node, 0, self._node)
        self._loadables = loadables
        fdt = fsw.as_fdt()

        # Remove generator entries from the main list
        for path in to_remove:
            if path in self._entries:
                del self._entries[path]

        # Pack this new FDT and scan it so we can add the data later
        fdt.pack()
        data = fdt.as_bytearray()
        return data

    def SetImagePos(self, image_pos):
        """Set the position in the image

        This sets each subentry's offsets, sizes and positions-in-image
        according to where they ended up in the packed FIT file.

        Args:
            image_pos (int): Position of this entry in the image
        """
        if self.build_done:
            return

        # Skip the section processing, since we do that below. Just call the
        # entry method
        Entry.SetImagePos(self, image_pos)

        # If mkimage is missing we'll have empty data,
        # which will cause a FDT_ERR_BADMAGIC error
        if self.mkimage in self.missing_bintools:
            return

        fdt = Fdt.FromData(self.GetData())
        fdt.Scan()

        for image_name, entry in self._entries.items():
            path = f"/images/{image_name}"
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
                offset = None
                size = None
                self.Raise(f'{path}: missing data properties')

            entry.SetOffsetSize(offset, size)
            entry.SetImagePos(image_pos + self.offset)

    def AddBintools(self, btools):
        super().AddBintools(btools)
        self.mkimage = self.AddBintool(btools, 'mkimage')
        self.fdtgrep = self.AddBintool(btools, 'fdtgrep')

    def CheckMissing(self, missing_list):
        # We must use our private entry list for this since generator nodes
        # which are removed from self._entries will otherwise not show up as
        # missing
        for entry in self._priv_entries.values():
            entry.CheckMissing(missing_list)

    def CheckOptional(self, optional_list):
        # We must use our private entry list for this since generator nodes
        # which are removed from self._entries will otherwise not show up as
        # optional
        for entry in self._priv_entries.values():
            entry.CheckOptional(optional_list)

    def CheckEntries(self):
        pass

    def UpdateSignatures(self, privatekey_fname, algo, input_fname):
        uniq = self.GetUniqueName()
        args = [ '-G', privatekey_fname, '-r', '-o', algo, '-F' ]
        if input_fname:
            fname = input_fname
        else:
            fname = tools.get_output_filename(f'{uniq}.fit')
            tools.write_file(fname, self.GetData())
        args.append(fname)

        if self.mkimage.run_cmd(*args) is None:
            self.Raise("Missing tool: 'mkimage'")

        data = tools.read_file(fname)
        self.WriteData(data)
