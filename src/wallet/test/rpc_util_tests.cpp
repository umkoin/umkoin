// Copyright (c) 2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <wallet/rpc/util.h>

#include <boost/test/unit_test.hpp>

namespace wallet {

BOOST_AUTO_TEST_SUITE(wallet_util_tests)

BOOST_AUTO_TEST_CASE(util_ParseISO8601DateTime)
{
    BOOST_CHECK_EQUAL(ParseISO8601DateTime("1970-01-01T00:00:00Z"), 0);
    BOOST_CHECK_EQUAL(ParseISO8601DateTime("1960-01-01T00:00:00Z"), 0);
    BOOST_CHECK_EQUAL(ParseISO8601DateTime("2000-01-01T00:00:01Z"), 946684801);
    BOOST_CHECK_EQUAL(ParseISO8601DateTime("2017-11-24T22:50:12Z"), 1511563812);
    BOOST_CHECK_EQUAL(ParseISO8601DateTime("2100-12-31T23:59:59Z"), 4133980799);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace wallet
