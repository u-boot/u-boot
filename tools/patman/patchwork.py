# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2025 Simon Glass <sjg@chromium.org>
#
"""Provides a basic API for the patchwork server
"""

import asyncio

import aiohttp

# Number of retries
RETRIES = 3

# Max concurrent request
MAX_CONCURRENT = 50

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
