// Copyright (c) 2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bench/data.h>

namespace benchmark {
namespace data {

#include <bench/data/block47789.raw.h>
const std::vector<uint8_t> block47789{block47789_raw, block47789_raw + sizeof(block47789_raw) / sizeof(block47789_raw[0])};

} // namespace data
} // namespace benchmark
