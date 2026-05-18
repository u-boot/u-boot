# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2026 Simon Glass <sjg@chromium.org>
#
"""Sphinx extension to auto-generate binman entry and bintool documentation.

This parses etype and btool source files using the ast module to extract
class docstrings, avoiding the need to import binman modules (which have
dependencies like libfdt that may not be available in the doc-build
environment).

The generated files are written to doc/develop/package/ (alongside
binman.rst) and included via toctree directives. They are .gitignore'd
since they are always regenerated during the build.

To use, add 'binman_docs' to the extensions list in conf.py.
"""

import ast
import os


def get_entry_docstring(source_file):
    """Extract the Entry_ class docstring from an etype source file.

    Some files contain helper classes before the Entry_ class, so we look
    specifically for a class whose name starts with 'Entry_'.

    Args:
        source_file: Path to the source file

    Returns:
        The docstring of the Entry_ class, or None
    """
    with open(source_file) as inf:
        tree = ast.parse(inf.read())
    for node in ast.iter_child_nodes(tree):
        if isinstance(node, ast.ClassDef) and node.name.startswith('Entry_'):
            return ast.get_docstring(node, clean=False)
    return None


def get_bintool_docstring(source_file):
    """Extract the Bintool class docstring from a btool source file.

    Args:
        source_file: Path to the source file

    Returns:
        The docstring of the Bintool class, or None
    """
    with open(source_file) as inf:
        tree = ast.parse(inf.read())
    for node in ast.iter_child_nodes(tree):
        if isinstance(node, ast.ClassDef) and node.name.startswith('Bintool'):
            return ast.get_docstring(node, clean=False)
    return None


def generate_entry_docs(srcdir):
    """Generate entries.rst content from etype source files.

    Args:
        srcdir: Root of the U-Boot source tree

    Returns:
        String containing RST content
    """
    etype_dir = os.path.join(srcdir, 'tools', 'binman', 'etype')
    modules = sorted([
        os.path.splitext(f)[0] for f in os.listdir(etype_dir)
        if f.endswith('.py') and not f.startswith('_') and f != '__init__.py'
    ])

    parts = ['''\
Binman Entry Documentation
==========================

This file describes the entry types supported by binman. These entry types can
be placed in an image one by one to build up a final firmware image. It is
fairly easy to create new entry types. Just add a new file to the 'etype'
directory. You can use the existing entries as examples.

Note that some entries are subclasses of others, using and extending their
features to produce new behaviours.


''']

    missing = []
    for name in modules:
        source = os.path.join(etype_dir, name + '.py')
        docs = get_entry_docstring(source)
        if docs:
            lines = docs.splitlines()
            first_line = lines[0]
            rest = [line[4:] for line in lines[1:]]
            hdr = 'Entry: %s: %s' % (name.replace('_', '-'), first_line)

            ref_name = 'etype_%s' % name
            parts.append('.. _%s:' % ref_name)
            parts.append('')
            parts.append(hdr)
            parts.append('-' * len(hdr))
            parts.append('\n'.join(rest))
            parts.append('')
            parts.append('')
        else:
            missing.append(name)

    if missing:
        raise ValueError('Documentation is missing for modules: %s' %
                         ', '.join(missing))

    return '\n'.join(parts)


def generate_bintool_docs(srcdir):
    """Generate bintools.rst content from btool source files.

    Args:
        srcdir: Root of the U-Boot source tree

    Returns:
        String containing RST content
    """
    btool_dir = os.path.join(srcdir, 'tools', 'binman', 'btool')
    fnames = [
        f for f in os.listdir(btool_dir)
        if f.endswith('.py') and not f.startswith('_') and f != '__init__.py'
    ]

    def tool_sort_name(fname):
        name = os.path.splitext(fname)[0]
        if name.startswith('btool_'):
            name = name[6:]
        return name

    fnames.sort(key=tool_sort_name)

    parts = ['''\
.. SPDX-License-Identifier: GPL-2.0+

Binman bintool Documentation
============================

This file describes the bintools (binary tools) supported by binman. Bintools
are binman's name for external executables that it runs to generate or process
binaries. It is fairly easy to create new bintools. Just add a new file to the
'btool' directory. You can use existing bintools as examples.


''']

    missing = []
    for fname in fnames:
        name = os.path.splitext(fname)[0]
        # Strip btool_ prefix used for modules that conflict with Python libs
        if name.startswith('btool_'):
            name = name[6:]
        source = os.path.join(btool_dir, fname)
        docs = get_bintool_docstring(source)
        if docs:
            lines = docs.splitlines()
            first_line = lines[0]
            rest = [line[4:] for line in lines[1:]]
            hdr = 'Bintool: %s: %s' % (name, first_line)
            parts.append(hdr)
            parts.append('-' * len(hdr))
            parts.append('\n'.join(rest))
            parts.append('')
            parts.append('')
        else:
            missing.append(name)

    if missing:
        raise ValueError('Documentation is missing for modules: %s' %
                         ', '.join(missing))

    return '\n'.join(parts)


def generate_docs(app):
    """Generate binman documentation RST files.

    Called by Sphinx during the builder-inited event, before any RST files
    are read.

    Args:
        app: The Sphinx application object
    """
    srcdir = os.path.abspath(os.path.join(app.srcdir, '..'))
    outdir = os.path.join(app.srcdir, 'develop', 'package')

    entries_rst = os.path.join(outdir, 'entries.rst')
    content = generate_entry_docs(srcdir)
    with open(entries_rst, 'w') as outf:
        outf.write(content)

    bintools_rst = os.path.join(outdir, 'bintools.rst')
    content = generate_bintool_docs(srcdir)
    with open(bintools_rst, 'w') as outf:
        outf.write(content)


def setup(app):
    app.connect('builder-inited', generate_docs)
    return {'version': '1.0', 'parallel_read_safe': True}
