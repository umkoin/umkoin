#!/usr/bin/env python3
# Copyright (c) 2019-2020 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test basic signet functionality"""

from decimal import Decimal

from test_framework.test_framework import UmkoinTestFramework
from test_framework.util import assert_equal

signet_blocks = [
    '00000020f61eee3b63a380a477a063af32b2bbc97c9ff9f01f2c4225e973988108000000f575c83235984e7dc4afc1f30944c170462e84437ab6f2d52e16878a79e4678bd1914d5fae77031eccf4070001010000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff025151feffffff0200f2052a010000001600149243f727dd5343293eb83174324019ec16c2630f0000000000000000776a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf94c4fecc7daa2490047304402205e423a8754336ca99dbe16509b877ef1bf98d008836c725005b3c787c41ebe46022047246e4467ad7cc7f1ad98662afcaf14c115e0095a227c7b05c5182591c23e7e01000120000000000000000000000000000000000000000000000000000000000000000000000000',
    '00000020a868e8514be5e46dabd6a122132f423f36a43b716a40c394e2a8d063e1010000f4c6c717e99d800c699c25a2006a75a0c5c09f432a936f385e6fce139cdbd1a5e9964d5fae77031e7d026e0001010000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff025a51feffffff0200f2052a01000000160014aaa671c82b138e3b8f510cd801e5f2bd0aa305940000000000000000776a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf94c4fecc7daa24900473044022042309f4c3c7a1a2ac8c24f890f962df1c0086cec10be0868087cfc427520cb2702201dafee8911c269b7e786e242045bb57cef3f5b0f177010c6159abae42f646cc501000120000000000000000000000000000000000000000000000000000000000000000000000000',
]

class SignetBasicTest(UmkoinTestFramework):
    def set_test_params(self):
        self.chain = "signet"
        self.num_nodes = 6
        self.setup_clean_chain = True
        shared_args1 = ["-signetchallenge=51"] # OP_TRUE
        shared_args2 = [] # default challenge
        # we use the exact same challenge except we do it as a 2-of-2, which means it should fail
        shared_args3 = ["-signetchallenge=522103ad5e0edad18cb1f0fc0d28a3d4f1f3e445640337489abb10404f2d1e086be430210359ef5021964fe22d6f8e05b2463c9540ce96883fe3b278760f048f5189f2e6c452ae"]

        self.extra_args = [
            shared_args1, shared_args1,
            shared_args2, shared_args2,
            shared_args3, shared_args3,
        ]

    def run_test(self):
        self.log.info("basic tests using OP_TRUE challenge")

        self.log.info('getmininginfo')
        mining_info = self.nodes[0].getmininginfo()
        assert_equal(mining_info['blocks'], 0)
        assert_equal(mining_info['chain'], 'signet')
        assert 'currentblocktx' not in mining_info
        assert 'currentblockweight' not in mining_info
        assert_equal(mining_info['networkhashps'], Decimal('0'))
        assert_equal(mining_info['pooledtx'], 0)

        self.nodes[0].generate(1)

        self.log.info("pregenerated signet blocks check")

        height = 0
        for block in signet_blocks:
            assert_equal(self.nodes[2].submitblock(block), None)
            height += 1
            assert_equal(self.nodes[2].getblockcount(), height)

        self.log.info("pregenerated signet blocks check (incompatible solution)")

        assert_equal(self.nodes[4].submitblock(signet_blocks[0]), 'bad-signet-blksig')

if __name__ == '__main__':
    SignetBasicTest().main()
