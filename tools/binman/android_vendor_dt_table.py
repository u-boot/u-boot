# SPDX-License-Identifier: GPL-2.0+

from binman.entry import Entry
from binman.etype.section import Entry_section
from dtoc import fdt_util


class Entry_Android_vendor_dt_table(Entry_section):
    """Base class for legacy Android vendor DT table entries"""

    @staticmethod
    def _DtbEntryName(node):
        return '_dtb_%s' % node.name

    def ReadNode(self):
        super().ReadNode()
        self._page_size = fdt_util.GetInt(self._node, 'page-size')
        if (self._page_size is not None and
                (self._page_size <= 0 or
                 self._page_size & (self._page_size - 1))):
            self.Raise('page-size must be a power of two')

    def _GetPayloadSubnodes(self, node):
        return [subnode for subnode in node.subnodes
                if not self.IsSpecialSubnode(subnode)]

    def ReadEntries(self):
        for node in self._node.subnodes:
            if self.IsSpecialSubnode(node):
                continue

            payloads = self._GetPayloadSubnodes(node)
            if len(payloads) > 1:
                self.Raise("subnode '%s': must contain exactly one DTB "
                           "payload subnode" % node.name)
            if not payloads:
                continue

            entry = Entry.Create(self, payloads[0],
                                 expanded=self.GetImage().use_expanded,
                                 missing_etype=self.GetImage().missing_etype)
            entry.ReadNode()
            entry.SetPrefix(self._name_prefix)
            self._entries[self._DtbEntryName(node)] = entry

    def _GetPageSize(self):
        if self._page_size is not None:
            return self._page_size

        section = self.section
        while section:
            if section.etype == 'android-boot':
                return section.page_size
            section = section.section

        return 2048

    def _GetU32Cells(self, node, propname):
        prop = node.props.get(propname)
        if not prop:
            self.Raise("subnode '%s': Missing required property '%s'" %
                       (node.name, propname))

        values = prop.value if isinstance(prop.value, list) else [prop.value]
        return [fdt_util.fdt32_to_cpu(value) for value in values]

    def _GetU32Tuple(self, node, propname, width):
        values = self._GetU32Cells(node, propname)
        if len(values) != width:
            self.Raise("subnode '%s': Property '%s' must contain exactly "
                       "%d cells" % (node.name, propname, width))

        return tuple(values)

    def _GetDtbData(self, node, required):
        entry = self._entries.get(self._DtbEntryName(node))
        if not entry:
            self.Raise("subnode '%s': Missing required DTB payload subnode" %
                       node.name)

        data = entry.GetData(required)
        if data is None and not required:
            return None

        return data

    def _GetDtbRecordData(self, node, required):
        return self._GetDtbData(node, required)

    def _ReadDtbRecords(self, required, read_record):
        records = []
        for node in self._node.subnodes:
            if self.IsSpecialSubnode(node):
                continue

            data = self._GetDtbRecordData(node, required)
            if data is None and not required:
                return None
            records.append(read_record(node, data))

        if not records:
            self.Raise('Missing required DTB subnodes')

        return records
