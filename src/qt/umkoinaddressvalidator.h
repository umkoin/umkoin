// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UMKOIN_QT_UMKOINADDRESSVALIDATOR_H
#define UMKOIN_QT_UMKOINADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class UmkoinAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit UmkoinAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

/** Umkoin address widget validator, checks for a valid umkoin address.
 */
class UmkoinAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit UmkoinAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

#endif // UMKOIN_QT_UMKOINADDRESSVALIDATOR_H
