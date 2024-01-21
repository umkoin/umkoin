26.x Release Notes
==================

Umkoin Core version 26.x is now available from:

  <http://www.umkoin.org/bin/umkoin-core-26.x/>

This release includes new features, various bug fixes and performance
improvements, as well as updated translations.

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
using the Linux kernel, macOS 11.0+, and Windows 7 and newer. Umkoin
Core should also work on most other Unix-like systems but is not as
frequently tested on them. It is not recommended to use Umkoin Core on
unsupported systems.

Notable changes
===============

### Wallet

- #28994 wallet: skip BnB when SFFO is enabled
- #28920 wallet: birth time update during tx scanning
- #29176 wallet: Fix use-after-free in WalletBatch::EraseRecords

### RPC

- #29003 rpc: fix getrawtransaction segfault

### CI

- #28992 ci: Use Ubuntu 24.04 Noble for asan,tsan,tidy,fuzz
- #29080 ci: Set HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK to avoid unrelated failures

Credits
=======

Thanks to everyone who directly contributed to this release:

- Andrew Chow
- furszy
- Hennadii Stepanov
- MarcoFalke
- Martin Zumsande
- Murch

As well as to everyone that helped with translations on
[Transifex](https://www.transifex.com/umkoin/umkoin-core/).
