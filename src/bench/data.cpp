// Copyright (c) 2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bench/data.h>

namespace benchmark {
namespace data {

#include <bench/data/block47789.raw.h>
const std::vector<uint8_t> block47789{std::begin(block47789_raw), std::end(block47789_raw)};

} // namespace data
} // namespace benchmark
