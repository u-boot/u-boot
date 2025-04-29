# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2025 Simon Glass <sjg@chromium.org>
#
"""Provides a basic API for the patchwork server
"""

import asyncio
import re

import aiohttp

# Number of retries
RETRIES = 3

# Max concurrent request
MAX_CONCURRENT = 50

# Patches which are part of a multi-patch series are shown with a prefix like
# [prefix, version, sequence], for example '[RFC, v2, 3/5]'. All but the last
# part is optional. This decodes the string into groups. For single patches
# the [] part is not present:
# Groups: (ignore, ignore, ignore, prefix, version, sequence, subject)
RE_PATCH = re.compile(r'(\[(((.*),)?(.*),)?(.*)\]\s)?(.*)$')

# This decodes the sequence string into a patch number and patch count
RE_SEQ = re.compile(r'(\d+)/(\d+)')


class Patch(dict):
    """Models a patch in patchwork

    This class records information obtained from patchwork

    Some of this information comes from the 'Patch' column:

        [RFC,v2,1/3] dm: Driver and uclass changes for tiny-dm

    This shows the prefix, version, seq, count and subject.

    The other properties come from other columns in the display.

    Properties:
        pid (str): ID of the patch (typically an integer)
        seq (int): Sequence number within series (1=first) parsed from sequence
            string
        count (int): Number of patches in series, parsed from sequence string
        raw_subject (str): Entire subject line, e.g.
            "[1/2,v2] efi_loader: Sort header file ordering"
        prefix (str): Prefix string or None (e.g. 'RFC')
        version (str): Version string or None (e.g. 'v2')
        raw_subject (str): Raw patch subject
        subject (str): Patch subject with [..] part removed (same as commit
            subject)
    """
    def __init__(self, pid):
        super().__init__()
        self.id = pid  # Use 'id' to match what the Rest API provides
        self.seq = None
        self.count = None
        self.prefix = None
        self.version = None
        self.raw_subject = None
        self.subject = None

    # These make us more like a dictionary
    def __setattr__(self, name, value):
        self[name] = value

    def __getattr__(self, name):
        return self[name]

    def __hash__(self):
        return hash(frozenset(self.items()))

    def __str__(self):
        return self.raw_subject

    def parse_subject(self, raw_subject):
        """Parse the subject of a patch into its component parts

        See RE_PATCH for details. The parsed info is placed into seq, count,
        prefix, version, subject

        Args:
            raw_subject (str): Subject string to parse

        Raises:
            ValueError: the subject cannot be parsed
        """
        self.raw_subject = raw_subject.strip()
        mat = RE_PATCH.search(raw_subject.strip())
        if not mat:
            raise ValueError(f"Cannot parse subject '{raw_subject}'")
        self.prefix, self.version, seq_info, self.subject = mat.groups()[3:]
        mat_seq = RE_SEQ.match(seq_info) if seq_info else False
        if mat_seq is None:
            self.version = seq_info
            seq_info = None
        if self.version and not self.version.startswith('v'):
            self.prefix = self.version
            self.version = None
        if seq_info:
            if mat_seq:
                self.seq = int(mat_seq.group(1))
                self.count = int(mat_seq.group(2))
        else:
            self.seq = 1
            self.count = 1


class Review:
    """Represents a single review email collected in Patchwork

    Patches can attract multiple reviews. Each consists of an author/date and
    a variable number of 'snippets', which are groups of quoted and unquoted
    text.
    """
    def __init__(self, meta, snippets):
        """Create new Review object

        Args:
            meta (str): Text containing review author and date
            snippets (list): List of snippets in th review, each a list of text
                lines
        """
        self.meta = ' : '.join([line for line in meta.splitlines() if line])
        self.snippets = snippets


class Patchwork:
    """Class to handle communication with patchwork
    """
    def __init__(self, url, show_progress=True):
        """Set up a new patchwork handler

        Args:
            url (str): URL of patchwork server, e.g.
               'https://patchwork.ozlabs.org'
        """
        self.url = url
        self.proj_id = None
        self.link_name = None
        self._show_progress = show_progress
        self.semaphore = asyncio.Semaphore(MAX_CONCURRENT)
        self.request_count = 0

    async def _request(self, client, subpath):
        """Call the patchwork API and return the result as JSON

        Args:
            client (aiohttp.ClientSession): Session to use
            subpath (str): URL subpath to use

        Returns:
            dict: Json result

        Raises:
            ValueError: the URL could not be read
        """
        # print('subpath', subpath)
        self.request_count += 1

        full_url = f'{self.url}/api/1.2/{subpath}'
        async with self.semaphore:
            # print('full_url', full_url)
            for i in range(RETRIES + 1):
                try:
                    async with client.get(full_url) as response:
                        if response.status != 200:
                            raise ValueError(
                                f"Could not read URL '{full_url}'")
                        result = await response.json()
                        # print('- done', full_url)
                        return result
                    break
                except aiohttp.client_exceptions.ServerDisconnectedError:
                    if i == RETRIES:
                        raise
