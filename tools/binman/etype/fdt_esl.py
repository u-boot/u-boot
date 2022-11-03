import os

from binman.entry import Entry
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_fdt_esl(Entry):
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.dtbs = []

    def ReadNode(self):
        super().ReadNode()

        self.esl = fdt_util.GetString(self._node, 'esl-file')
        self.dtbs = fdt_util.GetStringList(self._node, 'dtb')
        self.concat = fdt_util.GetStringList(self._node, 'concat')

        if not self.esl:
            self.Raise('Path to the ESL file needs to be provided')

        if not self.dtbs:
            self.Raise('At least one DTB needs to be provided')

    def dtb_embed_esl(self, esl, dtb):
        print('ESL File => ' + self.esl)
        print('DTB File => ' + dtb)

        self.keypair = os.path.split(esl)
        print('Calling the fdt_add_pubkey')
        self.fdt_add_pubkey.add_esl(self.keypair, dtb)

    def concat_binaries(self):
        bins = self.concat
        bin0_fname = tools.get_input_filename(bins[0])
        bin1_fname = tools.get_input_filename(bins[1])
        bin2_fname = tools.get_input_filename(bins[2])
        with open(bin0_fname, 'rb') as fd1, open(bin1_fname, 'rb') as fd2, open(bin2_fname, 'wb') as fd3:
            bin1 = fd1.read()
            bin2 = fd2.read()
            bin1 += bin2

            fd3.write(bin1)

    def ObtainContents(self):
        for dtb in self.dtbs:
            self.dtb_embed_esl(self.esl, dtb)

        if self.concat:
            self.concat_binaries()

    def AddBintools(self, btools):
        self.fdt_add_pubkey = self.AddBintool(btools, 'fdt_add_pubkey')
