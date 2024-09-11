25.x Release Notes
==================

Umkoin Core version 25.x is now available from:

  <http://www.umkoin.org/bin/umkoin-core-25.x/>

This release includes various bug fixes and performance
improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/umkoin/umkoin/issues>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes in some cases), then run the
installer (on Windows) or just copy over `/Applications/Umkoin-Qt` (on macOS)
or `umkoind`/`umkoin-qt` (on Linux).

Upgrading directly from a version of Umkoin Core that has reached its EOL is
possible, but it might take some time if the data directory needs to be migrated. Old
wallet versions of Umkoin Core are generally supported.

Compatibility
==============

Umkoin Core is supported and extensively tested on operating systems
using the Linux kernel, macOS 10.15+, and Windows 7 and newer. Umkoin
Core should also work on most other Unix-like systems but is not as
frequently tested on them. It is not recommended to use Umkoin Core on
unsupported systems.

Notable changes
===============

### Gui

- gui#774 Fix crash on selecting "Mask values" in transaction view

### RPC

- #29003 rpc: fix getrawtransaction segfault

### Wallet

- #29176 wallet: Fix use-after-free in WalletBatch::EraseRecords
- #29510 wallet: `getrawchangeaddress` and `getnewaddress` failures should not affect keypools for descriptor wallets

### P2P and network changes

- #29412 p2p: Don't process mutated blocks
- #29524 p2p: Don't consider blocks mutated if they don't connect to known prev block

Credits
=======

Thanks to everyone who directly contributed to this release:

- Martin Zumsande
- Sebastian Falbesoner
- MarcoFalke
- UdjinM6
- dergoegge
- Greg Sanders

As well as to everyone that helped with translations on
[Transifex](https://www.transifex.com/umkoin-core/umkoin/).
