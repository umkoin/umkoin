*After branching off for a major version release of Umkoin Core, use this
template to create the initial release notes draft.*

*The release notes draft is a temporary file that can be added to by anyone. See
[/doc/developer-notes.md#release-notes](/doc/developer-notes.md#release-notes)
for the process.*

*Create the draft, named* "*version* Release Notes Draft"
*(e.g. "0.20.0 Release Notes Draft")*

*Before the final release, move the notes back to this git repository.*

*version* Release Notes Draft
===============================

Umkoin Core version *version* is now available from:

  <http://www.umkoin.org/bin/umkoin-core-*version*/>

This release includes new features, various bug fixes and performance
improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/umkoin/umkoin/issues>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over `/Applications/Umkoin-Qt` (on Mac)
or `umkoind`/`umkoin-qt` (on Linux).

Upgrading directly from a version of Umkoin Core that has reached its EOL is
possible, but it might take some time if the datadir needs to be migrated. Old
wallet versions of Umkoin Core are generally supported.

Compatibility
==============

Umkoin Core is supported and extensively tested on operating systems using
the Linux kernel, macOS 10.12+, and Windows 7 and newer. It is not recommended
to use Umkoin Core on unsupported systems.

Umkoin Core should also work on most other Unix-like systems but is not
as frequently tested on them.

From Umkoin Core 0.20.0 onwards, macOS versions earlier than 10.12 are no
longer supported. Additionally, Umkoin Core does not yet change appearance
when macOS "dark mode" is activated.

In addition to previously supported CPU platforms, this release's pre-compiled
distribution provides binaries for the RISC-V platform.

Notable changes
===============

Build System
------------

- OpenSSL is no longer used by Umkoin Core. The last usage of the library
was removed in #17265.

New RPCs
--------

New settings
------------

Updated settings
----------------

Updated RPCs
------------

Note: some low-level RPC changes mainly useful for testing are described in the
Low-level Changes section below.

GUI changes
-----------

Wallet
------

- The wallet now by default uses bech32 addresses when using RPC, and creates native segwit change outputs.
- The way that output trust was computed has been fixed in #16766, which impacts confirmed/unconfirmed balance status and coin selection.

Low-level changes
=================

Tests
-----

- `-fallbackfee` was 0 (disabled) by default for the main chain, but 0.0002 by default for the test chains. Now it is 0
  by default for all chains. Testnet and regtest users will have to add `fallbackfee=0.0002` to their configuration if
  they weren't setting it and they want it to keep working like before. (#16524)

Credits
=======

Thanks to everyone who directly contributed to this release:


As well as to everyone that helped with translations on
[Transifex](https://www.transifex.com/umkoin/umkoin-core/).
