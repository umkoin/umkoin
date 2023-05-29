// Copyright (c) 2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UMKOIN_KERNEL_CHECKS_H
#define UMKOIN_KERNEL_CHECKS_H

#include <util/result.h>

struct bilingual_str;

namespace kernel {

struct Context;

/**
 *  Ensure a usable environment with all necessary library support.
 */
util::Result<void> SanityChecks(const Context&);

}

#endif // UMKOIN_KERNEL_CHECKS_H
