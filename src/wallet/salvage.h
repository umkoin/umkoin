// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UMKOIN_WALLET_SALVAGE_H
#define UMKOIN_WALLET_SALVAGE_H

#include <fs.h>
#include <streams.h>

bool RecoverDatabaseFile(const fs::path& file_path);

#endif // UMKOIN_WALLET_SALVAGE_H
