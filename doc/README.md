Umkoin Core
=============

Setup
---------------------
Umkoin Core is the original Umkoin client and it builds the backbone of the network. It downloads and, by default, stores the entire history of Umkoin transactions, which requires some free disk space. Depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few minutes to several hours or more.

To download Umkoin Core, visit [www.umkoin.org](http://www.umkoin.org/en/download.php).

Running
---------------------
The following are some helpful notes on how to run Umkoin Core on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/umkoin-qt` (GUI) or
- `bin/umkoind` (headless)

### Windows

Unpack the files into a directory, and then run umkoin-qt.exe.

### macOS

Drag Umkoin Core to your applications folder, and then run Umkoin Core.

Building
---------------------
The following are developer notes on how to build Umkoin Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [macOS Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [FreeBSD Build Notes](build-freebsd.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [NetBSD Build Notes](build-netbsd.md)
- [Android Build Notes](build-android.md)
- [Gitian Building Guide](gitian-building.md)

Development
---------------------
The Umkoin repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Productivity Notes](productivity.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://doxygen.bitcoincore.org/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [JSON-RPC Interface](JSON-RPC-interface.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [umkoin.conf Configuration File](umkoin-conf.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [I2P Support](i2p.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [Managing Wallets](managing-wallets.md)
- [PSBT support](psbt.md)
- [Reduce Memory](reduce-memory.md)
- [Reduce Traffic](reduce-traffic.md)
- [Tor Support](tor.md)
- [ZMQ](zmq.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
