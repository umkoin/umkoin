v31.x Release Notes
===================

Umkoin Core version 31.x is now available from:

  <http://www.umkoin.org/bin/umkoin-core-31.x/>

This release includes new features, various bug fixes and performance
improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/umkoin/umkoin/issues>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes in some cases), then run the installer
(on Windows) or just copy over `/Applications/Umkoin-Qt` (on macOS) or
`umkoind`/`umkoin-qt` (on Linux).

Upgrading directly from a version of Umkoin Core that has reached its EOL is
possible, but it might take some time if the data directory needs to be
migrated. Old wallet versions of Umkoin Core are generally supported.

Compatibility
==============

Umkoin Core is supported and tested on the following operating systems or
newer: Linux Kernel 3.17, macOS 14, and Windows 10 (version 1903). Umkoin Core
should also work on most other Unix-like systems but is not as frequently tested
on them. It is not recommended to use Umkoin Core on unsupported systems.

Notable changes
===============

### P2P

- #35032 net_processing: don't modify addrman for private broadcast connections

### Test

- #34425 test: Fix all races after a socket is closed gracefully
- #34863 test: Clean shutdown in Socks5Server
- #35080 test: Add missing self.options.timeout_factor scale in tool_umkoin_chainstate.py

### CI

- #35202 ci: restore sockets in i686, no IPC job

### Misc

- #35175 multi_index: fix compilation failure with boost >= 1.91

Credits
=======

Thanks to everyone who directly contributed to this release:

- Cory Fields
- Greg Sanders
- Lőrinc
- MarcoFalke
- optout21
- Vasil Dimov

As well as to everyone that helped with translations on
[Transifex](https://explore.transifex.com/umkoin/umkoin-core/).
