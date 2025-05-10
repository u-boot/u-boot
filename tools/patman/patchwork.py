# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2025 Simon Glass <sjg@chromium.org>
#
"""Provides a basic API for the patchwork server
"""

import asyncio
import re

import aiohttp
from u_boot_pylib import terminal

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
        data (dict or None): Patch data:
    """
    def __init__(self, pid, state=None, data=None, comments=None,
                 series_data=None):
        super().__init__()
        self.id = pid  # Use 'id' to match what the Rest API provides
        self.seq = None
        self.count = None
        self.prefix = None
        self.version = None
        self.raw_subject = None
        self.subject = None
        self.state = state
        self.data = data
        self.comments = comments
        self.series_data = series_data
        self.name = None

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
    def __init__(self, url, show_progress=True, single_thread=False):
        """Set up a new patchwork handler

        Args:
            url (str): URL of patchwork server, e.g.
               'https://patchwork.ozlabs.org'
        """
        self.url = url
        self.fake_request = None
        self.proj_id = None
        self.link_name = None
        self._show_progress = show_progress
        self.semaphore = asyncio.Semaphore(
            1 if single_thread else MAX_CONCURRENT)
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
        if self.fake_request:
            return self.fake_request(subpath)

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

    @staticmethod
    def for_testing(func):
        """Get an instance to use for testing

        Args:
            func (function): Function to call to handle requests. The function
                is passed a URL and is expected to return a dict with the
                resulting data

        Returns:
            Patchwork: testing instance
        """
        pwork = Patchwork(None, show_progress=False)
        pwork.fake_request = func
        return pwork

    async def get_series(self, client, link):
        """Read information about a series

        Args:
            client (aiohttp.ClientSession): Session to use
            link (str): Patchwork series ID

        Returns: dict containing patchwork's series information
            id (int): series ID unique across patchwork instance, e.g. 3
            url (str): Full URL, e.g.
                'https://patchwork.ozlabs.org/api/1.2/series/3/'
            web_url (str): Full URL, e.g.
                'https://patchwork.ozlabs.org/project/uboot/list/?series=3
            project (dict): project information (id, url, name, link_name,
                list_id, list_email, etc.
            name (str): Series name, e.g. '[U-Boot] moveconfig: fix error'
            date (str): Date, e.g. '2017-08-27T08:00:51'
            submitter (dict): id, url, name, email, e.g.:
                "id": 6125,
                "url": "https://patchwork.ozlabs.org/api/1.2/people/6125/",
                "name": "Chris Packham",
                "email": "judge.packham@gmail.com"
            version (int): Version number
            total (int): Total number of patches based on subject
            received_total (int): Total patches received by patchwork
            received_all (bool): True if all patches were received
            mbox (str): URL of mailbox, e.g.
                'https://patchwork.ozlabs.org/series/3/mbox/'
            cover_letter (dict) or None, e.g.:
                "id": 806215,
                "url": "https://patchwork.ozlabs.org/api/1.2/covers/806215/",
                "web_url": "https://patchwork.ozlabs.org/project/uboot/cover/
                    20170827094411.8583-1-judge.packham@gmail.com/",
                "msgid": "<20170827094411.8583-1-judge.packham@gmail.com>",
                "list_archive_url": null,
                "date": "2017-08-27T09:44:07",
                "name": "[U-Boot,v2,0/4] usb: net: Migrate USB Ethernet",
                "mbox": "https://patchwork.ozlabs.org/project/uboot/cover/
                    20170827094411.8583-1-judge.packham@gmail.com/mbox/"
            patches (list of dict), each e.g.:
                "id": 806202,
                "url": "https://patchwork.ozlabs.org/api/1.2/patches/806202/",
                "web_url": "https://patchwork.ozlabs.org/project/uboot/patch/
                    20170827080051.816-1-judge.packham@gmail.com/",
                "msgid": "<20170827080051.816-1-judge.packham@gmail.com>",
                "list_archive_url": null,
                "date": "2017-08-27T08:00:51",
                "name": "[U-Boot] moveconfig: fix error message do_autoconf()",
                "mbox": "https://patchwork.ozlabs.org/project/uboot/patch/
                    20170827080051.816-1-judge.packham@gmail.com/mbox/"
        """
        return await self._request(client, f'series/{link}/')

    async def get_patch(self, client, patch_id):
        """Read information about a patch

        Args:
            client (aiohttp.ClientSession): Session to use
            patch_id (str): Patchwork patch ID

        Returns: dict containing patchwork's patch information
            "id": 185,
            "url": "https://patchwork.ozlabs.org/api/1.2/patches/185/",
            "web_url": "https://patchwork.ozlabs.org/project/cbe-oss-dev/patch/
                200809050416.27831.adetsch@br.ibm.com/",
            project (dict): project information (id, url, name, link_name,
                    list_id, list_email, etc.
            "msgid": "<200809050416.27831.adetsch@br.ibm.com>",
            "list_archive_url": null,
            "date": "2008-09-05T07:16:27",
            "name": "powerpc/spufs: Fix possible scheduling of a context",
            "commit_ref": "b2e601d14deb2083e2a537b47869ab3895d23a28",
            "pull_url": null,
            "state": "accepted",
            "archived": false,
            "hash": "bc1c0b80d7cff66c0d1e5f3f8f4d10eb36176f0d",
            "submitter": {
                "id": 93,
                "url": "https://patchwork.ozlabs.org/api/1.2/people/93/",
                "name": "Andre Detsch",
                "email": "adetsch@br.ibm.com"
            },
            "delegate": {
                "id": 1,
                "url": "https://patchwork.ozlabs.org/api/1.2/users/1/",
                "username": "jk",
                "first_name": "Jeremy",
                "last_name": "Kerr",
                "email": "jk@ozlabs.org"
            },
            "mbox": "https://patchwork.ozlabs.org/project/cbe-oss-dev/patch/
                200809050416.27831.adetsch@br.ibm.com/mbox/",
            "series": [],
            "comments": "https://patchwork.ozlabs.org/api/patches/185/
                comments/",
            "check": "pending",
            "checks": "https://patchwork.ozlabs.org/api/patches/185/checks/",
            "tags": {},
            "related": [],
            "headers": {...}
            "content": "We currently have a race when scheduling a context
                after we have found a runnable context in spusched_tick, the
                context may have been scheduled by spu_activate().

                This may result in a panic if we try to unschedule a context
                been freed in the meantime.

                This change exits spu_schedule() if the context has already
                scheduled, so we don't end up scheduling it twice.

                Signed-off-by: Andre Detsch <adetsch@br.ibm.com>",
            "diff": '''Index: spufs/arch/powerpc/platforms/cell/spufs/sched.c
                =======================================================
                --- spufs.orig/arch/powerpc/platforms/cell/spufs/sched.c
                +++ spufs/arch/powerpc/platforms/cell/spufs/sched.c
                @@ -727,7 +727,8 @@ static void spu_schedule(struct spu *spu
                 \t/* not a candidate for interruptible because it's called
                 \t   from the scheduler thread or from spu_deactivate */
                 \tmutex_lock(&ctx->state_mutex);
                -\t__spu_schedule(spu, ctx);
                +\tif (ctx->state == SPU_STATE_SAVED)
                +\t\t__spu_schedule(spu, ctx);
                 \tspu_release(ctx);
                 }
                '''
            "prefixes": ["3/3", ...]
        """
        return await self._request(client, f'patches/{patch_id}/')

    async def _get_patch_comments(self, client, patch_id):
        """Read comments about a patch

        Args:
            client (aiohttp.ClientSession): Session to use
            patch_id (str): Patchwork patch ID

        Returns: list of dict: list of comments:
            id (int): series ID unique across patchwork instance, e.g. 3331924
            web_url (str): Full URL, e.g.
                'https://patchwork.ozlabs.org/comment/3331924/'
            msgid (str): Message ID, e.g.
                '<d2526c98-8198-4b8b-ab10-20bda0151da1@gmx.de>'
            list_archive_url: (unknown?)
            date (str): Date, e.g. '2024-06-20T13:38:03'
            subject (str): email subject, e.g. 'Re: [PATCH 3/5] buildman:
                Support building within a Python venv'
            date (str): Date, e.g. '2017-08-27T08:00:51'
            submitter (dict): id, url, name, email, e.g.:
                "id": 61270,
                "url": "https://patchwork.ozlabs.org/api/people/61270/",
                "name": "Heinrich Schuchardt",
                "email": "xypron.glpk@gmx.de"
            content (str): Content of email, e.g. 'On 20.06.24 15:19,
                Simon Glass wrote:
                >...'
            headers: dict: email headers, see get_cover() for an example
        """
        return await self._request(client, f'patches/{patch_id}/comments/')

    async def get_patch_comments(self, patch_id):
        async with aiohttp.ClientSession() as client:
            return await self._get_patch_comments(client, patch_id)

    async def _get_patch_status(self, client, patch_id):
        """Get the patch status

        Args:
            client (aiohttp.ClientSession): Session to use
            patch_id (int): Patch ID to look up in patchwork

        Return:
            PATCH: Patch information

        Requests:
            1 for patch, 1 for patch comments
        """
        data = await self.get_patch(client, patch_id)
        state = data['state']
        comment_data = await self._get_patch_comments(client, patch_id)

        return Patch(patch_id, state, data, comment_data)

    async def series_get_state(self, client, link, read_comments,
                               read_cover_comments):
        """Sync the series information against patchwork, to find patch status

        Args:
            client (aiohttp.ClientSession): Session to use
            link (str): Patchwork series ID
            read_comments (bool): True to read the comments on the patches
            read_cover_comments (bool): True to read the comments on the cover
                letter

        Return: tuple:
            COVER object, or None if none or not read_cover_comments
            list of PATCH objects
        """
        data = await self.get_series(client, link)
        patch_list = list(data['patches'])

        count = len(patch_list)
        patches = []
        if read_comments:
            # Returns a list of Patch objects
            tasks = [self._get_patch_status(client, patch_list[i]['id'])
                     for i in range(count)]

            patch_status = await asyncio.gather(*tasks)
            for patch_data, status in zip(patch_list, patch_status):
                status.series_data = patch_data
                patches.append(status)
        else:
            for i in range(count):
                info = patch_list[i]
                pat = Patch(info['id'], series_data=info)
                pat.raw_subject = info['name']
                patches.append(pat)
        if self._show_progress:
            terminal.print_clear()

        # TODO: Implement this
        cover = None

        return cover, patches
