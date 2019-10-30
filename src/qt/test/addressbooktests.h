#ifndef UMKOIN_QT_TEST_ADDRESSBOOKTESTS_H
#define UMKOIN_QT_TEST_ADDRESSBOOKTESTS_H

#include <QObject>
#include <QTest>

namespace interfaces {
class Node;
} // namespace interfaces

class AddressBookTests : public QObject
{
public:
    AddressBookTests(interfaces::Node& node) : m_node(node) {}
    interfaces::Node& m_node;

    Q_OBJECT

private Q_SLOTS:
    void addressBookTests();
};

#endif // UMKOIN_QT_TEST_ADDRESSBOOKTESTS_H
