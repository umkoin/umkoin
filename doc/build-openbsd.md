OpenBSD build guide
======================
(updated for OpenBSD 6.2)

This guide describes how to build umkoind and command-line utilities on OpenBSD.

OpenBSD is most commonly used as a server OS, so this guide does not contain instructions for building the GUI.

Preparation
-------------

Run the following as root to install the base dependencies for building:

```bash
pkg_add git gmake libevent libtool
pkg_add autoconf # (select highest version, e.g. 2.69)
pkg_add automake # (select highest version, e.g. 1.15)
pkg_add python # (select highest version, e.g. 3.6)
pkg_add boost

git clone https://github.com/umkoin/umkoin.git
```

See [dependencies.md](dependencies.md) for a complete overview.

GCC
-------

The default C++ compiler that comes with OpenBSD 6.2 is g++ 4.2.1. This version is old (from 2007), and is not able to compile the current version of Umkoin Core because it has no C++11 support. We'll install a newer version of GCC:

```bash
 pkg_add g++
 ```

 This compiler will not overwrite the system compiler, it will be installed as `egcc` and `eg++` in `/usr/local/bin`.

### Building BerkeleyDB

BerkeleyDB is only necessary for the wallet functionality. To skip this, pass `--disable-wallet` to `./configure`.

It is recommended to use Berkeley DB 4.8. You cannot use the BerkeleyDB library
from ports, for the same reason as boost above (g++/libstd++ incompatibility).
If you have to build it yourself, you can use [the installation script included
in contrib/](contrib/install_db4.sh) like so

```shell
./contrib/install_db4.sh `pwd` CC=cc CXX=c++
```

from the root of the repository. Then set `BDB_PREFIX` for the next section:

```shell
export BDB_PREFIX="$PWD/db4"
```

### Building Umkoin Core

**Important**: use `gmake`, not `make`. The non-GNU `make` will exit with a horrible error.

Preparation:
```bash
export AUTOCONF_VERSION=2.69 # replace this with the autoconf version that you installed
export AUTOMAKE_VERSION=1.15 # replace this with the automake version that you installed
./autogen.sh
```
Make sure `BDB_PREFIX` is set to the appropriate path from the above steps.

To configure with wallet:
```bash
./configure --with-gui=no CC=cc CXX=c++ \
    BDB_LIBS="-L${BDB_PREFIX}/lib -ldb_cxx-4.8" BDB_CFLAGS="-I${BDB_PREFIX}/include"
```

To configure without wallet:
```bash
./configure --disable-wallet --with-gui=no CC=cc CXX=c++
```

Build and run the tests:
```bash
gmake # use -jX here for parallelism
gmake check
```

Clang
------------------------------

```bash
pkg_add llvm

./configure --disable-wallet --with-gui=no CC=clang CXX=clang++
gmake # use -jX here for parallelism
gmake check
```
