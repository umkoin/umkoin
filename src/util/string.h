// Copyright (c) 2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UMKOIN_UTIL_STRING_H
#define UMKOIN_UTIL_STRING_H

#include <attributes.h>

#include <cstring>
#include <string>
#include <vector>

/**
 * Join a list of items
 *
 * @param list       The list to join
 * @param separator  The separator
 * @param unary_op   Apply this operator to each item in the list
 */
template <typename T, typename UnaryOp>
std::string Join(const std::vector<T>& list, const std::string& separator, UnaryOp unary_op)
{
    std::string ret;
    for (size_t i = 0; i < list.size(); ++i) {
        if (i > 0) ret += separator;
        ret += unary_op(list.at(i));
    }
    return ret;
}

inline std::string Join(const std::vector<std::string>& list, const std::string& separator)
{
    return Join(list, separator, [](const std::string& i) { return i; });
}

/**
 * Check if a string does not contain any embedded NUL (\0) characters
 */
NODISCARD inline bool ValidAsCString(const std::string& str) noexcept
{
    return str.size() == strlen(str.c_str());
}

#endif // UMKOIN_UTIL_STRENCODINGS_H
