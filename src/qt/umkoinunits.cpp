// Copyright (c) 2011-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/umkoinunits.h>

#include <QStringList>

UmkoinUnits::UmkoinUnits(QObject *parent):
        QAbstractListModel(parent),
        unitlist(availableUnits())
{
}

QList<UmkoinUnits::Unit> UmkoinUnits::availableUnits()
{
    QList<UmkoinUnits::Unit> unitlist;
    unitlist.append(UMK);
    unitlist.append(mUMK);
    unitlist.append(uUMK);
    unitlist.append(SAT);
    return unitlist;
}

bool UmkoinUnits::valid(int unit)
{
    switch(unit)
    {
    case UMK:
    case mUMK:
    case uUMK:
    case SAT:
        return true;
    default:
        return false;
    }
}

QString UmkoinUnits::longName(int unit)
{
    switch(unit)
    {
    case UMK: return QString("UMK");
    case mUMK: return QString("mUMK");
    case uUMK: return QString::fromUtf8("µUMK (bits)");
    case SAT: return QString("Satoshi (sat)");
    default: return QString("???");
    }
}

QString UmkoinUnits::shortName(int unit)
{
    switch(unit)
    {
    case uUMK: return QString::fromUtf8("bits");
    case SAT: return QString("sat");
    default: return longName(unit);
    }
}

QString UmkoinUnits::description(int unit)
{
    switch(unit)
    {
    case UMK: return QString("Umkoins");
    case mUMK: return QString("Milli-Umkoins (1 / 1" THIN_SP_UTF8 "000)");
    case uUMK: return QString("Micro-Umkoins (bits) (1 / 1" THIN_SP_UTF8 "000" THIN_SP_UTF8 "000)");
    case SAT: return QString("Satoshi (sat) (1 / 100" THIN_SP_UTF8 "000" THIN_SP_UTF8 "000)");
    default: return QString("???");
    }
}

qint64 UmkoinUnits::factor(int unit)
{
    switch(unit)
    {
    case UMK: return 100000000;
    case mUMK: return 100000;
    case uUMK: return 100;
    case SAT: return 1;
    default: return 100000000;
    }
}

int UmkoinUnits::decimals(int unit)
{
    switch(unit)
    {
    case UMK: return 8;
    case mUMK: return 5;
    case uUMK: return 2;
    case SAT: return 0;
    default: return 0;
    }
}

QString UmkoinUnits::format(int unit, const CAmount& nIn, bool fPlus, SeparatorStyle separators)
{
    // Note: not using straight sprintf here because we do NOT want
    // localized number formatting.
    if(!valid(unit))
        return QString(); // Refuse to format invalid unit
    qint64 n = (qint64)nIn;
    qint64 coin = factor(unit);
    int num_decimals = decimals(unit);
    qint64 n_abs = (n > 0 ? n : -n);
    qint64 quotient = n_abs / coin;
    QString quotient_str = QString::number(quotient);

    // Use SI-style thin space separators as these are locale independent and can't be
    // confused with the decimal marker.
    QChar thin_sp(THIN_SP_CP);
    int q_size = quotient_str.size();
    if (separators == separatorAlways || (separators == separatorStandard && q_size > 4))
        for (int i = 3; i < q_size; i += 3)
            quotient_str.insert(q_size - i, thin_sp);

    if (n < 0)
        quotient_str.insert(0, '-');
    else if (fPlus && n > 0)
        quotient_str.insert(0, '+');

    if (num_decimals > 0) {
        qint64 remainder = n_abs % coin;
        QString remainder_str = QString::number(remainder).rightJustified(num_decimals, '0');
        return quotient_str + QString(".") + remainder_str;
    } else {
        return quotient_str;
    }
}


// NOTE: Using formatWithUnit in an HTML context risks wrapping
// quantities at the thousands separator. More subtly, it also results
// in a standard space rather than a thin space, due to a bug in Qt's
// XML whitespace canonicalisation
//
// Please take care to use formatHtmlWithUnit instead, when
// appropriate.

QString UmkoinUnits::formatWithUnit(int unit, const CAmount& amount, bool plussign, SeparatorStyle separators)
{
    return format(unit, amount, plussign, separators) + QString(" ") + shortName(unit);
}

QString UmkoinUnits::formatHtmlWithUnit(int unit, const CAmount& amount, bool plussign, SeparatorStyle separators)
{
    QString str(formatWithUnit(unit, amount, plussign, separators));
    str.replace(QChar(THIN_SP_CP), QString(THIN_SP_HTML));
    return QString("<span style='white-space: nowrap;'>%1</span>").arg(str);
}


bool UmkoinUnits::parse(int unit, const QString &value, CAmount *val_out)
{
    if(!valid(unit) || value.isEmpty())
        return false; // Refuse to parse invalid unit or empty string
    int num_decimals = decimals(unit);

    // Ignore spaces and thin spaces when parsing
    QStringList parts = removeSpaces(value).split(".");

    if(parts.size() > 2)
    {
        return false; // More than one dot
    }
    QString whole = parts[0];
    QString decimals;

    if(parts.size() > 1)
    {
        decimals = parts[1];
    }
    if(decimals.size() > num_decimals)
    {
        return false; // Exceeds max precision
    }
    bool ok = false;
    QString str = whole + decimals.leftJustified(num_decimals, '0');

    if(str.size() > 18)
    {
        return false; // Longer numbers will exceed 63 bits
    }
    CAmount retvalue(str.toLongLong(&ok));
    if(val_out)
    {
        *val_out = retvalue;
    }
    return ok;
}

QString UmkoinUnits::getAmountColumnTitle(int unit)
{
    QString amountTitle = QObject::tr("Amount");
    if (UmkoinUnits::valid(unit))
    {
        amountTitle += " ("+UmkoinUnits::shortName(unit) + ")";
    }
    return amountTitle;
}

int UmkoinUnits::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return unitlist.size();
}

QVariant UmkoinUnits::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(row >= 0 && row < unitlist.size())
    {
        Unit unit = unitlist.at(row);
        switch(role)
        {
        case Qt::EditRole:
        case Qt::DisplayRole:
            return QVariant(longName(unit));
        case Qt::ToolTipRole:
            return QVariant(description(unit));
        case UnitRole:
            return QVariant(static_cast<int>(unit));
        }
    }
    return QVariant();
}

CAmount UmkoinUnits::maxMoney()
{
    return MAX_MONEY;
}
