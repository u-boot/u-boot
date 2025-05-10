# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2025 Simon Glass <sjg@chromium.org>
#
"""Provides a basic API for the patchwork server
"""

import asyncio
import re

import aiohttp
from collections import namedtuple

from u_boot_pylib import terminal

# Information passed to series_get_states()
# link (str): Patchwork link for series
# series_id (int): Series ID in database
# series_name (str): Series name
# version (int): Version number of series
# show_comments (bool): True to show comments
# show_cover_comments (bool): True to show cover-letter comments
STATE_REQ = namedtuple(
    'state_req',
    'link,series_id,series_name,version,show_comments,show_cover_comments')

# Responses from series_get_states()
# int: ser_ver ID number
# COVER: Cover-letter info
# list of Patch: Information on each patch in the series
# list of dict: patches, see get_series()['patches']
STATE_RESP = namedtuple('state_resp', 'svid,cover,patches,patch_list')

# Information about a cover-letter on patchwork
# id (int): Patchwork ID of cover letter
# state (str): Current state, e.g. 'accepted'
# num_comments (int): Number of comments
# name (str): Series name
# comments (list of dict): Comments
COVER = namedtuple('cover', 'id,num_comments,name,comments')

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

    class _Stats:
        def __init__(self, parent):
            self.parent = parent
            self.request_count = 0

        def __enter__(self):
            return self

        def __exit__(self, exc_type, exc_val, exc_tb):
            self.request_count = self.parent.request_count

    def collect_stats(self):
        """Context manager to count requests across a range of patchwork calls

        Usage:
            pwork = Patchwork(...)
            with pwork.count_requests() as counter:
                pwork.something()
            print(f'{counter.count} requests')
        """
        self.request_count = 0
        return self._Stats(self)

    async def get_projects(self):
        """Get a list of projects on the server

        Returns:
            list of dict, one for each project
                'name' (str): Project name, e.g. 'U-Boot'
                'id' (int): Project ID, e.g. 9
                'link_name' (str): Project's link-name, e.g. 'uboot'
        """
        async with aiohttp.ClientSession() as client:
            return await self._request(client, 'projects/')

    async def _query_series(self, client, desc):
        """Query series by name

        Args:
            client (aiohttp.ClientSession): Session to use
            desc: String to search for

        Return:
            list of series matches, each a dict, see get_series()
        """
        query = desc.replace(' ', '+')
        return await self._request(
            client, f'series/?project={self.proj_id}&q={query}')

    async def _find_series(self, client, svid, ser_id, version, ser):
        """Find a series on the server

        Args:
            client (aiohttp.ClientSession): Session to use
            svid (int): ser_ver ID
            ser_id (int): series ID
            version (int): Version number to search for
            ser (Series): Contains description (cover-letter title)

        Returns:
            tuple:
                int: ser_ver ID (as passed in)
                int: series ID (as passed in)
                str: Series link, or None if not found
                list of dict, or None if found
                    each dict is the server result from a possible series
        """
        desc = ser.desc
        name_found = []

        # Do a series query on the description
        res = await self._query_series(client, desc)
        for pws in res:
            if pws['name'] == desc:
                if int(pws['version']) == version:
                    return svid, ser_id, pws['id'], None
                name_found.append(pws)

        # When there is no cover letter, patchwork uses the first patch as the
        # series name
        cmt = ser.commits[0]

        res = await self._query_series(client, cmt.subject)
        for pws in res:
            patch = Patch(0)
            patch.parse_subject(pws['name'])
            if patch.subject == cmt.subject:
                if int(pws['version']) == version:
                    return svid, ser_id, pws['id'], None
                name_found.append(pws)

        return svid, ser_id, None, name_found or res

    async def find_series(self, ser, version):
        """Find a series based on its description and version

        Args:
            ser (Series): Contains description (cover-letter title)
            version (int): Version number

        Return: tuple:
            tuple:
                str: Series ID, or None if not found
                list of dict, or None if found
                    each dict is the server result from a possible series
            int: number of server requests done
        """
        async with aiohttp.ClientSession() as client:
            # We don't know the svid and it isn't needed, so use -1
            _, _, link, options = await self._find_series(client, -1, -1,
                                                          version, ser)
        return link, options

    async def find_series_list(self, to_find):
        """Find the link for each series in a list

        Args:
            to_find (dict of svids to sync):
                key (int): ser_ver ID
                value (tuple):
                    int: Series ID
                    int: Series version
                    str: Series link
                    str: Series description

        Return: tuple:
            list of tuple, one for each item in to_find:
                int: ser_ver_ID
                int: series ID
                int: Series version
                str: Series link, or None if not found
                list of dict, or None if found
                    each dict is the server result from a possible series
            int: number of server requests done
        """
        self.request_count = 0
        async with aiohttp.ClientSession() as client:
            tasks = [asyncio.create_task(
                self._find_series(client, svid, ser_id, version, desc))
                for svid, (ser_id, version, link, desc) in to_find.items()]
            results = await asyncio.gather(*tasks)

        return results, self.request_count

    def project_set(self, project_id, link_name):
        """Set the project ID

        The patchwork server has multiple projects. This allows the ID and
        link_name of the relevant project to be selected

        This function is used for testing

        Args:
            project_id (int): Project ID to use, e.g. 6
            link_name (str): Name to use for project URL links, e.g. 'uboot'
        """
        self.proj_id = project_id
        self.link_name = link_name

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

    async def get_cover(self, client, cover_id):
        """Read information about a cover letter

        Args:
            client (aiohttp.ClientSession): Session to use
            cover_id (int): Patchwork cover-letter ID

        Returns: dict containing patchwork's cover-letter information:
            id (int): series ID unique across patchwork instance, e.g. 3
            url (str): Full URL, e.g. https://patchwork.ozlabs.org/project/uboot/list/?series=3
            project (dict): project information (id, url, name, link_name,
                list_id, list_email, etc.
            url (str): Full URL, e.g. 'https://patchwork.ozlabs.org/api/1.2/covers/2054866/'
            web_url (str): Full URL, e.g. 'https://patchwork.ozlabs.org/project/uboot/cover/20250304130947.109799-1-sjg@chromium.org/'
            project (dict): project information (id, url, name, link_name,
                list_id, list_email, etc.
            msgid (str): Message ID, e.g. '20250304130947.109799-1-sjg@chromium.org>'
            list_archive_url (?)
            date (str): Date, e.g. '2017-08-27T08:00:51'
            name (str): Series name, e.g. '[U-Boot] moveconfig: fix error'
            submitter (dict): id, url, name, email, e.g.:
                "id": 6170,
                "url": "https://patchwork.ozlabs.org/api/1.2/people/6170/",
                "name": "Simon Glass",
                "email": "sjg@chromium.org"
            mbox (str): URL to mailbox, e.g. 'https://patchwork.ozlabs.org/project/uboot/cover/20250304130947.109799-1-sjg@chromium.org/mbox/'
            series (list of dict) each e.g.:
                "id": 446956,
                "url": "https://patchwork.ozlabs.org/api/1.2/series/446956/",
                "web_url": "https://patchwork.ozlabs.org/project/uboot/list/?series=446956",
                "date": "2025-03-04T13:09:37",
                "name": "binman: Check code-coverage requirements",
                "version": 1,
                "mbox": "https://patchwork.ozlabs.org/series/446956/mbox/"
            comments: Web URL to comments: 'https://patchwork.ozlabs.org/api/covers/2054866/comments/'
            headers: dict: e.g.:
                "Return-Path": "<u-boot-bounces@lists.denx.de>",
                "X-Original-To": "incoming@patchwork.ozlabs.org",
                "Delivered-To": "patchwork-incoming@legolas.ozlabs.org",
                "Authentication-Results": [
                    "legolas.ozlabs.org;
\tdkim=pass (1024-bit key;
 unprotected) header.d=chromium.org header.i=@chromium.org header.a=rsa-sha256
 header.s=google header.b=dG8yqtoK;
\tdkim-atps=neutral",
                    "legolas.ozlabs.org;
 spf=pass (sender SPF authorized) smtp.mailfrom=lists.denx.de
 (client-ip=85.214.62.61; helo=phobos.denx.de;
 envelope-from=u-boot-bounces@lists.denx.de; receiver=patchwork.ozlabs.org)",
                    "phobos.denx.de;
 dmarc=pass (p=none dis=none) header.from=chromium.org",
                    "phobos.denx.de;
 spf=pass smtp.mailfrom=u-boot-bounces@lists.denx.de",
                    "phobos.denx.de;
\tdkim=pass (1024-bit key;
 unprotected) header.d=chromium.org header.i=@chromium.org
 header.b=\"dG8yqtoK\";
\tdkim-atps=neutral",
                    "phobos.denx.de;
 dmarc=pass (p=none dis=none) header.from=chromium.org",
                    "phobos.denx.de;
 spf=pass smtp.mailfrom=sjg@chromium.org"
                ],
                "Received": [
                    "from phobos.denx.de (phobos.denx.de [85.214.62.61])
\t(using TLSv1.3 with cipher TLS_AES_256_GCM_SHA384 (256/256 bits)
\t key-exchange X25519 server-signature ECDSA (secp384r1))
\t(No client certificate requested)
\tby legolas.ozlabs.org (Postfix) with ESMTPS id 4Z6bd50jLhz1yD0
\tfor <incoming@patchwork.ozlabs.org>; Wed,  5 Mar 2025 00:10:00 +1100 (AEDT)",
                    "from h2850616.stratoserver.net (localhost [IPv6:::1])
\tby phobos.denx.de (Postfix) with ESMTP id 434E88144A;
\tTue,  4 Mar 2025 14:09:58 +0100 (CET)",
                    "by phobos.denx.de (Postfix, from userid 109)
 id 8CBF98144A; Tue,  4 Mar 2025 14:09:57 +0100 (CET)",
                    "from mail-io1-xd2e.google.com (mail-io1-xd2e.google.com
 [IPv6:2607:f8b0:4864:20::d2e])
 (using TLSv1.3 with cipher TLS_AES_128_GCM_SHA256 (128/128 bits))
 (No client certificate requested)
 by phobos.denx.de (Postfix) with ESMTPS id 48AE281426
 for <u-boot@lists.denx.de>; Tue,  4 Mar 2025 14:09:55 +0100 (CET)",
                    "by mail-io1-xd2e.google.com with SMTP id
 ca18e2360f4ac-85ae33109f6so128326139f.2
 for <u-boot@lists.denx.de>; Tue, 04 Mar 2025 05:09:55 -0800 (PST)",
                    "from chromium.org (c-73-203-119-151.hsd1.co.comcast.net.
 [73.203.119.151]) by smtp.gmail.com with ESMTPSA id
 ca18e2360f4ac-858753cd304sm287383839f.33.2025.03.04.05.09.49
 (version=TLS1_3 cipher=TLS_AES_256_GCM_SHA384 bits=256/256);
 Tue, 04 Mar 2025 05:09:50 -0800 (PST)"
                ],
                "X-Spam-Checker-Version": "SpamAssassin 3.4.2 (2018-09-13) on phobos.denx.de",
                "X-Spam-Level": "",
                "X-Spam-Status": "No, score=-2.1 required=5.0 tests=BAYES_00,DKIMWL_WL_HIGH,
 DKIM_SIGNED,DKIM_VALID,DKIM_VALID_AU,DKIM_VALID_EF,
 RCVD_IN_DNSWL_BLOCKED,SPF_HELO_NONE,SPF_PASS autolearn=ham
 autolearn_force=no version=3.4.2",
                "DKIM-Signature": "v=1; a=rsa-sha256; c=relaxed/relaxed;
 d=chromium.org; s=google; t=1741093792; x=1741698592; darn=lists.denx.de;
 h=content-transfer-encoding:mime-version:message-id:date:subject:cc
 :to:from:from:to:cc:subject:date:message-id:reply-to;
 bh=B2zsLws430/BEZfatNjeaNnrcxmYUstVjp1pSXgNQjc=;
 b=dG8yqtoKpSy15RHagnPcppzR8KbFCRXa2OBwXfwGoyN6M15tOJsUu2tpCdBFYiL5Mk
 hQz5iDLV8p0Bs+fP4XtNEx7KeYfTZhiqcRFvdCLwYtGray/IHtOZaNoHLajrstic/OgE
 01ymu6gOEboU32eQ8uC8pdCYQ4UCkfKJwmiiU=",
                "X-Google-DKIM-Signature": "v=1; a=rsa-sha256; c=relaxed/relaxed;
 d=1e100.net; s=20230601; t=1741093792; x=1741698592;
 h=content-transfer-encoding:mime-version:message-id:date:subject:cc
 :to:from:x-gm-message-state:from:to:cc:subject:date:message-id
 :reply-to;
 bh=B2zsLws430/BEZfatNjeaNnrcxmYUstVjp1pSXgNQjc=;
 b=eihzJf4i9gin9usvz4hnAvvbLV9/yB7hGPpwwW/amgnPUyWCeQstgvGL7WDLYYnukH
 161p4mt7+cCj7Hao/jSPvVZeuKiBNPkS4YCuP3QjXfdk2ziQ9IjloVmGarWZUOlYJ5iQ
 dZnxypUkuFfLcEDSwUmRO1dvLi3nH8PDlae3yT2H87LeHaxhXWdzHxQdPc86rkYyCqCr
 qBC2CTS31jqSuiaI+7qB3glvbJbSEXkunz0iDewTJDvZfmuloxTipWUjRJ1mg9UJcZt5
 9xIuTq1n9aYf1RcQlrEOQhdBAQ0/IJgvmZtzPZi9L+ppBva1ER/xm06nMA7GEUtyGwun
 c6pA==",
                "X-Gm-Message-State": "AOJu0Yybx3b1+yClf/IfIbQd9u8sxzK9ixPP2HimXF/dGZfSiS7Cb+O5
 WrAkvtp7m3KPM/Mpv0sSZ5qrfTnKnb3WZyv6Oe5Q1iUjAftGNwbSxob5eJ/0y3cgrTdzE4sIWPE
 =",
                "X-Gm-Gg": "ASbGncu5gtgpXEPGrpbTRJulqFrFj1YPAAmKk4MiXA8/3J1A+25F0Uug2KeFUrZEjkG
 KMdPg/C7e2emIvfM+Jl+mKv0ITBvhbyNCyY1q2U1s1cayZF05coZ9ewzGxXJGiEqLMG69uBmmIi
 rBEvCnkXS+HVZobDQMtOsezpc+Ju8JRA7+y1R0WIlutl1mQARct6p0zTkuZp75QyB6dm/d0KYgd
 iux/t/f0HC2CxstQlTlJYzKL6UJgkB5/UorY1lW/0NDRS6P1iemPQ7I3EPLJO8tM5ZrpJE7qgNP
 xy0jXbUv44c48qJ1VszfY5USB8fRG7nwUYxNu6N1PXv9xWbl+z2xL68qNYUrFlHsB8ILTXAyzyr
 Cdj+Sxg==",
                "X-Google-Smtp-Source": "
 AGHT+IFeVk5D4YEfJgPxOfg3ikO6Q7IhaDzABGkAPI6HA0ubK85OPhUHK08gV7enBQ8OdoE/ttqEjw==",
                "X-Received": "by 2002:a05:6602:640f:b0:855:63c8:abb5 with SMTP id
 ca18e2360f4ac-85881fdba3amr1839428939f.13.1741093792636;
 Tue, 04 Mar 2025 05:09:52 -0800 (PST)",
                "From": "Simon Glass <sjg@chromium.org>",
                "To": "U-Boot Mailing List <u-boot@lists.denx.de>",
                "Cc": "Simon Glass <sjg@chromium.org>, Alexander Kochetkov <al.kochet@gmail.com>,
 Alper Nebi Yasak <alpernebiyasak@gmail.com>,
 Brandon Maier <brandon.maier@collins.com>,
 Jerome Forissier <jerome.forissier@linaro.org>,
 Jiaxun Yang <jiaxun.yang@flygoat.com>,
 Neha Malcom Francis <n-francis@ti.com>,
 Patrick Rudolph <patrick.rudolph@9elements.com>,
 Paul HENRYS <paul.henrys_ext@softathome.com>, Peng Fan <peng.fan@nxp.com>,
 Philippe Reynes <philippe.reynes@softathome.com>,
 Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>,
 Tom Rini <trini@konsulko.com>",
                "Subject": "[PATCH 0/7] binman: Check code-coverage requirements",
                "Date": "Tue,  4 Mar 2025 06:09:37 -0700",
                "Message-ID": "<20250304130947.109799-1-sjg@chromium.org>",
                "X-Mailer": "git-send-email 2.43.0",
                "MIME-Version": "1.0",
                "Content-Transfer-Encoding": "8bit",
                "X-BeenThere": "u-boot@lists.denx.de",
                "X-Mailman-Version": "2.1.39",
                "Precedence": "list",
                "List-Id": "U-Boot discussion <u-boot.lists.denx.de>",
                "List-Unsubscribe": "<https://lists.denx.de/options/u-boot>,
 <mailto:u-boot-request@lists.denx.de?subject=unsubscribe>",
                "List-Archive": "<https://lists.denx.de/pipermail/u-boot/>",
                "List-Post": "<mailto:u-boot@lists.denx.de>",
                "List-Help": "<mailto:u-boot-request@lists.denx.de?subject=help>",
                "List-Subscribe": "<https://lists.denx.de/listinfo/u-boot>,
 <mailto:u-boot-request@lists.denx.de?subject=subscribe>",
                "Errors-To": "u-boot-bounces@lists.denx.de",
                "Sender": "\"U-Boot\" <u-boot-bounces@lists.denx.de>",
                "X-Virus-Scanned": "clamav-milter 0.103.8 at phobos.denx.de",
                "X-Virus-Status": "Clean"
            content (str): Email content, e.g. 'This series adds a cover-coverage check to CI for Binman. The iMX8 tests
are still not completed,...'
        """
        async with aiohttp.ClientSession() as client:
            return await self._request(client, f'covers/{cover_id}/')

    async def get_cover_comments(self, client, cover_id):
        """Read comments about a cover letter

        Args:
            client (aiohttp.ClientSession): Session to use
            cover_id (str): Patchwork cover-letter ID

        Returns: list of dict: list of comments, each:
            id (int): series ID unique across patchwork instance, e.g. 3472068
            web_url (str): Full URL, e.g. 'https://patchwork.ozlabs.org/comment/3472068/'
            list_archive_url: (unknown?)

            project (dict): project information (id, url, name, link_name,
                list_id, list_email, etc.
            url (str): Full URL, e.g. 'https://patchwork.ozlabs.org/api/1.2/covers/2054866/'
            web_url (str): Full URL, e.g. 'https://patchwork.ozlabs.org/project/uboot/cover/20250304130947.109799-1-sjg@chromium.org/'
            project (dict): project information (id, url, name, link_name,
                list_id, list_email, etc.
            date (str): Date, e.g. '2025-03-04T13:16:15'
            subject (str): 'Re: [PATCH 0/7] binman: Check code-coverage requirements'
            submitter (dict): id, url, name, email, e.g.:
                "id": 6170,
                "url": "https://patchwork.ozlabs.org/api/people/6170/",
                "name": "Simon Glass",
                "email": "sjg@chromium.org"
            content (str): Email content, e.g. 'Hi,

On Tue, 4 Mar 2025 at 06:09, Simon Glass <sjg@chromium.org> wrote:
>
> This '...
            headers: dict: email headers, see get_cover() for an example
        """
        return await self._request(client, f'covers/{cover_id}/comments/')

    async def get_series_url(self, link):
        """Get the URL for a series

        Args:
            link (str): Patchwork series ID

        Returns:
            str: URL for the series page
        """
        return f'{self.url}/project/{self.link_name}/list/?series={link}&state=*&archive=both'

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

    async def get_series_cover(self, client, data):
        """Get the cover information (including comments)

        Args:
            client (aiohttp.ClientSession): Session to use
            data (dict): Return value from self.get_series()

        Returns:
            COVER object, or None if no cover letter
        """
        # Patchwork should always provide this, but use get() so that we don't
        # have to provide it in our fake patchwork _fake_patchwork_cser()
        cover = data.get('cover_letter')
        cover_id = None
        if cover:
            cover_id = cover['id']
            info = await self.get_cover_comments(client, cover_id)
            cover = COVER(cover_id, len(info), cover['name'], info)
        return cover

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

        if read_cover_comments:
            cover = await self.get_series_cover(client, data)
        else:
            cover = None

        return cover, patches
