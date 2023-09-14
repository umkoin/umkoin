#!/usr/bin/env python3
# Copyright (c) 2018-2021 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test the blocksdir option.
"""

import os
import shutil

from test_framework.test_framework import UmkoinTestFramework, initialize_datadir


class BlocksdirTest(UmkoinTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1

    def run_test(self):
        self.stop_node(0)
        assert os.path.isdir(os.path.join(self.nodes[0].blocks_path))
        assert not os.path.isdir(os.path.join(self.nodes[0].datadir, "blocks"))
        shutil.rmtree(self.nodes[0].datadir)
        initialize_datadir(self.options.tmpdir, 0, self.chain)
        self.log.info("Starting with nonexistent blocksdir ...")
        blocksdir_path = os.path.join(self.options.tmpdir, 'blocksdir')
        self.nodes[0].assert_start_raises_init_error([f"-blocksdir={blocksdir_path}"], f'Error: Specified blocks directory "{blocksdir_path}" does not exist.')
        os.mkdir(blocksdir_path)
        self.log.info("Starting with existing blocksdir ...")
        self.start_node(0, [f"-blocksdir={blocksdir_path}"])
        self.log.info("mining blocks..")
        self.generatetoaddress(self.nodes[0], 10, self.nodes[0].get_deterministic_priv_key().address)
        assert os.path.isfile(os.path.join(blocksdir_path, self.chain, "blocks", "blk00000.dat"))
        assert os.path.isdir(os.path.join(self.nodes[0].blocks_path, "index"))


if __name__ == '__main__':
    BlocksdirTest().main()
