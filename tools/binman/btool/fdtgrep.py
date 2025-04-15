# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Bintool implementation for fdtgrep

fdtgrepprovides a way to grep devicetree-binary files to extract or remove
certain elements.

Usage: fdtgrep - extract portions from device tree

Usage:
	fdtgrep <options> <dt file>|-

Output formats are:
	dts - device tree soure text
	dtb - device tree blob (sets -Hmt automatically)
	bin - device tree fragment (may not be a valid .dtb)

Options: -[haAc:b:C:defg:G:HIlLmn:N:o:O:p:P:rRsStTvhV]
  -a, --show-address                 Display address
  -A, --colour                       Show all nodes/tags, colour those that match
  -b, --include-node-with-prop <arg> Include contains containing property
  -c, --include-compat <arg>         Compatible nodes to include in grep
  -C, --exclude-compat <arg>         Compatible nodes to exclude in grep
  -d, --diff                         Diff: Mark matching nodes with +, others with -
  -e, --enter-node                   Enter direct subnode names of matching nodes
  -f, --show-offset                  Display offset
  -g, --include-match <arg>          Node/property/compatible string to include in grep
  -G, --exclude-match <arg>          Node/property/compatible string to exclude in grep
  -H, --show-header                  Output a header
  -I, --show-version                 Put "/dts-v1/;" on first line of dts output
  -l, --list-regions                 Output a region list
  -L, --list-strings                 List strings in string table
  -m, --include-mem                  Include mem_rsvmap section in binary output
  -n, --include-node <arg>           Node to include in grep
  -N, --exclude-node <arg>           Node to exclude in grep
  -p, --include-prop <arg>           Property to include in grep
  -P, --exclude-prop <arg>           Property to exclude in grep
  -r, --remove-strings               Remove unused strings from string table
  -R, --include-root                 Include root node and all properties
  -s, --show-subnodes                Show all subnodes matching nodes
  -S, --skip-supernodes              Don't include supernodes of matching nodes
  -t, --show-stringtab               Include string table in binary output
  -T, --show-aliases                 Include matching aliases in output
  -o, --out <arg>                    -o <output file>
  -O, --out-format <arg>             -O <output format>
  -v, --invert-match                 Invert the sense of matching (select non-matching lines)
  -h, --help                         Print this help and exit
  -V, --version                      Print version and exit
"""

import tempfile

from u_boot_pylib import tools
from binman import bintool

class Bintoolfdtgrep(bintool.Bintool):
    """Handles the 'fdtgrep' tool

    This bintool supports running `fdtgrep` with parameters suitable for
    producing SPL devicetrees from the main one.
    """
    def __init__(self, name):
        super().__init__(name, 'Grep devicetree files')

    def create_for_phase(self, infile, phase, outfile, remove_props):
        """Create the FDT for a particular phase

        Args:
            infile (str): Input filename containing the full FDT contents (with
                all nodes and properties)
            phase (str): Phase to generate for ('tpl', 'vpl', 'spl')
            outfile (str): Output filename to write the grepped FDT contents to
                (with only neceesary nodes and properties)

        Returns:
            str or bytes: Resulting stdout from the bintool
        """
        if phase == 'tpl':
            tag = 'bootph-pre-sram'
        elif phase == 'vpl':
            tag = 'bootph-verify'
        elif phase == 'spl':
            tag = 'bootph-pre-ram'
        else:
            raise ValueError(f"Invalid U-Boot phase '{phase}': Use tpl/vpl/spl")

        # These args mirror those in cmd_fdtgrep in scripts/Makefile.lib
        # First do the first stage
        with tempfile.NamedTemporaryFile(prefix='fdtgrep.tmp',
                                         dir=tools.get_output_dir()) as tmp:
            args = [
                infile,
                '-o', tmp.name,
                '-b', 'bootph-all',
                '-b', tag,
                '-u',
                '-RT',
                '-n', '/chosen',
                 '-n', '/config',
                 '-O', 'dtb',
                ]
            self.run_cmd(*args)
            args = [
                    tmp.name,
                    '-o', outfile,
                    '-r',
                     '-O', 'dtb',
                    '-P', 'bootph-all',
                    '-P', 'bootph-pre-ram',
                    '-P', 'bootph-pre-sram',
                    '-P', 'bootph-verify',
                    ]
            for prop_name in remove_props:
                args += ['-P', prop_name]
            return self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for fdtgrep

        This installs fdtgrep using the apt utility, which assumes that it is
        packaged in u-boot tools, which it is not.

        Args:
            method (FETCH_...): Method to use

        Returns:
            True if the file was fetched and now installed, None if a method
            other than FETCH_BIN was requested

        Raises:
            Valuerror: Fetching could not be completed
        """
        if method != bintool.FETCH_BIN:
            return None
        return self.apt_install('u-boot-tools')
