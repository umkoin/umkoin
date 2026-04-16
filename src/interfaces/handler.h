// Copyright (c) 2018-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UMKOIN_INTERFACES_HANDLER_H
#define UMKOIN_INTERFACES_HANDLER_H

#include <functional>
#include <memory>

namespace umksignals {
    class connection;
} // namespace umksignals

namespace interfaces {

//! Generic interface for managing an event handler or callback function
//! registered with another interface. Has a single disconnect method to cancel
//! the registration and prevent any future notifications.
class Handler
{
public:
    virtual ~Handler() = default;

    //! Disconnect the handler.
    virtual void disconnect() = 0;
};

//! Return handler wrapping a umksignals connection.
std::unique_ptr<Handler> MakeSignalHandler(umksignals::connection connection);

//! Return handler wrapping a cleanup function.
std::unique_ptr<Handler> MakeCleanupHandler(std::function<void()> cleanup);

} // namespace interfaces

#endif // UMKOIN_INTERFACES_HANDLER_H
