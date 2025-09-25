Umkoin Core version 29.1 is now available from:

  <http://www.umkoin.org/bin/umkoin-core-29.1/>

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

Umkoin Core is supported and tested on operating systems using the
Linux Kernel 3.17+, macOS 13+, and Windows 10+. Umkoin
Core should also work on most other Unix-like systems but is not as
frequently tested on them. It is not recommended to use Umkoin Core on
unsupported systems.

Notable changes
===============

### CI

- #32999 ci: Use APT_LLVM_V in msan task
- #33099 ci: allow for any libc++ intrumentation & use it for TSAN
- #33258 ci: use LLVM 21

Credits
=======

Thanks to everyone who directly contributed to this release:

- fanquake
- MarcoFalke

As well as to everyone that helped with translations on
[Transifex](https://explore.transifex.com/umkoin/umkoin-core/).
