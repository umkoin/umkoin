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
the Linux kernel, macOS 10.10+, and Windows 7 and newer. It is not recommended
to use Umkoin Core on unsupported systems.

Umkoin Core should also work on most other Unix-like systems but is not
as frequently tested on them.

From Umkoin Core 0.17.0 onwards, macOS versions earlier than 10.10 are no
longer supported, as Umkoin Core is now built using Qt 5.9.x which requires
macOS 10.10+. Additionally, Umkoin Core does not yet change appearance when
macOS "dark mode" is activated.

In addition to previously supported CPU platforms, this release's pre-compiled
distribution provides binaries for the RISC-V platform.

Notable changes
===============

Credits
=======

Thanks to everyone who directly contributed to this release:


As well as to everyone that helped with translations on
[Transifex](https://www.transifex.com/umkoin/umkoin-core/).
