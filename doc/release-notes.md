(note: this is a temporary file, to be added-to by anybody, and moved to
release-notes at release time)

Umkoin Core version *version* is now available from:

  <http://umkoin.org/releases/umkoin-*version*/>

This is a new major version release, including new features, various bugfixes
and performance improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/vmta/umkoin/issues>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over `/Applications/Umkoin-Qt` (on Mac)
or `umkoind`/`umkoin-qt` (on Linux).

The first time you run version 0.15.0, your chainstate database will be converted to a
new format, which will take anywhere from a few minutes to half an hour,
depending on the speed of your machine.

Note that the block database format also changed in version 0.8.0 and there is no
automatic upgrade code from before version 0.8 to version 0.15.0. Upgrading
directly from 0.7.x and earlier without redownloading the blockchain is not supported.
However, as usual, old wallet versions are still supported.

Downgrading warning
-------------------

The chainstate database for this release is not compatible with previous
releases, so if you run 0.15 and then decide to switch back to any
older version, you will need to run the old release with the `-reindex-chainstate`
option to rebuild the chainstate data structures in the old format.

If your node has pruning enabled, this will entail re-downloading and
processing the entire blockchain.

Compatibility
==============

Umkoin Core is tested on multiple operating systems using
the Linux, and Windows 8, 10. Windows XP is not supported.

Umkoin Core should also work on most other Unix-like systems but is not
frequently tested on them.

Notable changes
===============

Credits
=======

Thanks to everyone who directly contributed to this release
