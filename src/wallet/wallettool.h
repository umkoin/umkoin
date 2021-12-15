// Copyright (c) 2016-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UMKOIN_WALLET_WALLETTOOL_H
#define UMKOIN_WALLET_WALLETTOOL_H

#include <string>

class ArgsManager;

namespace WalletTool {

bool ExecuteWalletToolFunc(const ArgsManager& args, const std::string& command);

} // namespace WalletTool

#endif // UMKOIN_WALLET_WALLETTOOL_H
