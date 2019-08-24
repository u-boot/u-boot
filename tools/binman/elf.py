# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Handle various things related to ELF images
#

from __future__ import print_function

from collections import namedtuple, OrderedDict
import command
import io
import os
import re
import shutil
import struct
import tempfile

import tools
import tout

ELF_TOOLS = True
try:
    from elftools.elf.elffile import ELFFile
    from elftools.elf.sections import SymbolTableSection
except:  # pragma: no cover
    ELF_TOOLS = False

Symbol = namedtuple('Symbol', ['section', 'address', 'size', 'weak'])

# Information about an ELF file:
#    data: Extracted program contents of ELF file (this would be loaded by an
#           ELF loader when reading this file
#    load: Load address of code
#    entry: Entry address of code
#    memsize: Number of bytes in memory occupied by loading this ELF file
ElfInfo = namedtuple('ElfInfo', ['data', 'load', 'entry', 'memsize'])


def GetSymbols(fname, patterns):
    """Get the symbols from an ELF file

    Args:
        fname: Filename of the ELF file to read
        patterns: List of regex patterns to search for, each a string

    Returns:
        None, if the file does not exist, or Dict:
          key: Name of symbol
          value: Hex value of symbol
    """
    stdout = tools.Run('objdump', '-t', fname)
    lines = stdout.splitlines()
    if patterns:
        re_syms = re.compile('|'.join(patterns))
    else:
        re_syms = None
    syms = {}
    syms_started = False
    for line in lines:
        if not line or not syms_started:
            if 'SYMBOL TABLE' in line:
                syms_started = True
            line = None  # Otherwise code coverage complains about 'continue'
            continue
        if re_syms and not re_syms.search(line):
            continue

        space_pos = line.find(' ')
        value, rest = line[:space_pos], line[space_pos + 1:]
        flags = rest[:7]
        parts = rest[7:].split()
        section, size =  parts[:2]
        if len(parts) > 2:
            name = parts[2] if parts[2] != '.hidden' else parts[3]
            syms[name] = Symbol(section, int(value, 16), int(size,16),
                                flags[1] == 'w')

    # Sort dict by address
    return OrderedDict(sorted(syms.items(), key=lambda x: x[1].address))

def GetSymbolAddress(fname, sym_name):
    """Get a value of a symbol from an ELF file

    Args:
        fname: Filename of the ELF file to read
        patterns: List of regex patterns to search for, each a string

    Returns:
        Symbol value (as an integer) or None if not found
    """
    syms = GetSymbols(fname, [sym_name])
    sym = syms.get(sym_name)
    if not sym:
        return None
    return sym.address

def LookupAndWriteSymbols(elf_fname, entry, section):
    """Replace all symbols in an entry with their correct values

    The entry contents is updated so that values for referenced symbols will be
    visible at run time. This is done by finding out the symbols offsets in the
    entry (using the ELF file) and replacing them with values from binman's data
    structures.

    Args:
        elf_fname: Filename of ELF image containing the symbol information for
            entry
        entry: Entry to process
        section: Section which can be used to lookup symbol values
    """
    fname = tools.GetInputFilename(elf_fname)
    syms = GetSymbols(fname, ['image', 'binman'])
    if not syms:
        return
    base = syms.get('__image_copy_start')
    if not base:
        return
    for name, sym in syms.items():
        if name.startswith('_binman'):
            msg = ("Section '%s': Symbol '%s'\n   in entry '%s'" %
                   (section.GetPath(), name, entry.GetPath()))
            offset = sym.address - base.address
            if offset < 0 or offset + sym.size > entry.contents_size:
                raise ValueError('%s has offset %x (size %x) but the contents '
                                 'size is %x' % (entry.GetPath(), offset,
                                                 sym.size, entry.contents_size))
            if sym.size == 4:
                pack_string = '<I'
            elif sym.size == 8:
                pack_string = '<Q'
            else:
                raise ValueError('%s has size %d: only 4 and 8 are supported' %
                                 (msg, sym.size))

            # Look up the symbol in our entry tables.
            value = section.LookupSymbol(name, sym.weak, msg)
            if value is not None:
                value += base.address
            else:
                value = -1
                pack_string = pack_string.lower()
            value_bytes = struct.pack(pack_string, value)
            tout.Debug('%s:\n   insert %s, offset %x, value %x, length %d' %
                       (msg, name, offset, value, len(value_bytes)))
            entry.data = (entry.data[:offset] + value_bytes +
                        entry.data[offset + sym.size:])

def MakeElf(elf_fname, text, data):
    """Make an elf file with the given data in a single section

    The output file has a several section including '.text' and '.data',
    containing the info provided in arguments.

    Args:
        elf_fname: Output filename
        text: Text (code) to put in the file's .text section
        data: Data to put in the file's .data section
    """
    outdir = tempfile.mkdtemp(prefix='binman.elf.')
    s_file = os.path.join(outdir, 'elf.S')

    # Spilt the text into two parts so that we can make the entry point two
    # bytes after the start of the text section
    text_bytes1 = ['\t.byte\t%#x' % tools.ToByte(byte) for byte in text[:2]]
    text_bytes2 = ['\t.byte\t%#x' % tools.ToByte(byte) for byte in text[2:]]
    data_bytes = ['\t.byte\t%#x' % tools.ToByte(byte) for byte in data]
    with open(s_file, 'w') as fd:
        print('''/* Auto-generated C program to produce an ELF file for testing */

.section .text
.code32
.globl _start
.type _start, @function
%s
_start:
%s
.ident "comment"

.comm fred,8,4

.section .empty
.globl _empty
_empty:
.byte 1

.globl ernie
.data
.type ernie, @object
.size ernie, 4
ernie:
%s
''' % ('\n'.join(text_bytes1), '\n'.join(text_bytes2), '\n'.join(data_bytes)),
        file=fd)
    lds_file = os.path.join(outdir, 'elf.lds')

    # Use a linker script to set the alignment and text address.
    with open(lds_file, 'w') as fd:
        print('''/* Auto-generated linker script to produce an ELF file for testing */

PHDRS
{
    text PT_LOAD ;
    data PT_LOAD ;
    empty PT_LOAD FLAGS ( 6 ) ;
    note PT_NOTE ;
}

SECTIONS
{
    . = 0xfef20000;
    ENTRY(_start)
    .text . : SUBALIGN(0)
    {
        *(.text)
    } :text
    .data : {
        *(.data)
    } :data
    _bss_start = .;
    .empty : {
        *(.empty)
    } :empty
    /DISCARD/ : {
        *(.note.gnu.property)
    }
    .note : {
        *(.comment)
    } :note
    .bss _bss_start  (OVERLAY) : {
        *(.bss)
    }
}
''', file=fd)
    # -static: Avoid requiring any shared libraries
    # -nostdlib: Don't link with C library
    # -Wl,--build-id=none: Don't generate a build ID, so that we just get the
    #   text section at the start
    # -m32: Build for 32-bit x86
    # -T...: Specifies the link script, which sets the start address
    stdout = command.Output('cc', '-static', '-nostdlib', '-Wl,--build-id=none',
                            '-m32','-T', lds_file, '-o', elf_fname, s_file)
    shutil.rmtree(outdir)

def DecodeElf(data, location):
    """Decode an ELF file and return information about it

    Args:
        data: Data from ELF file
        location: Start address of data to return

    Returns:
        ElfInfo object containing information about the decoded ELF file
    """
    file_size = len(data)
    with io.BytesIO(data) as fd:
        elf = ELFFile(fd)
        data_start = 0xffffffff;
        data_end = 0;
        mem_end = 0;
        virt_to_phys = 0;

        for i in range(elf.num_segments()):
            segment = elf.get_segment(i)
            if segment['p_type'] != 'PT_LOAD' or not segment['p_memsz']:
                skipped = 1  # To make code-coverage see this line
                continue
            start = segment['p_paddr']
            mend = start + segment['p_memsz']
            rend = start + segment['p_filesz']
            data_start = min(data_start, start)
            data_end = max(data_end, rend)
            mem_end = max(mem_end, mend)
            if not virt_to_phys:
                virt_to_phys = segment['p_paddr'] - segment['p_vaddr']

        output = bytearray(data_end - data_start)
        for i in range(elf.num_segments()):
            segment = elf.get_segment(i)
            if segment['p_type'] != 'PT_LOAD' or not segment['p_memsz']:
                skipped = 1  # To make code-coverage see this line
                continue
            start = segment['p_paddr']
            offset = 0
            if start < location:
                offset = location - start
                start = location
            # A legal ELF file can have a program header with non-zero length
            # but zero-length file size and a non-zero offset which, added
            # together, are greater than input->size (i.e. the total file size).
            #  So we need to not even test in the case that p_filesz is zero.
            # Note: All of this code is commented out since we don't have a test
            # case for it.
            size = segment['p_filesz']
            #if not size:
                #continue
            #end = segment['p_offset'] + segment['p_filesz']
            #if end > file_size:
                #raise ValueError('Underflow copying out the segment. File has %#x bytes left, segment end is %#x\n',
                                 #file_size, end)
            output[start - data_start:start - data_start + size] = (
                segment.data()[offset:])
    return ElfInfo(output, data_start, elf.header['e_entry'] + virt_to_phys,
                   mem_end - data_start)
