FROM debian:stable

RUN dpkg --add-architecture i386
RUN dpkg --add-architecture s390x
<<<<<<< HEAD
=======
RUN dpkg --add-architecture armhf
RUN dpkg --add-architecture arm64
RUN dpkg --add-architecture ppc64el
>>>>>>> 6d46dfbfa26f2e243b4eab1e8b34b7cc0e51401a
RUN apt-get update

# dkpg-dev: to make pkg-config work in cross-builds
RUN apt-get install --no-install-recommends --no-upgrade -y \
        git ca-certificates \
        make automake libtool pkg-config dpkg-dev valgrind qemu-user \
<<<<<<< HEAD
        gcc clang libc6-dbg \
        gcc-i686-linux-gnu libc6-dev-i386-cross libc6-dbg:i386 \
        gcc-s390x-linux-gnu libc6-dev-s390x-cross libc6-dbg:s390x
=======
        gcc clang llvm libc6-dbg \
        gcc-i686-linux-gnu libc6-dev-i386-cross libc6-dbg:i386 libubsan1:i386 libasan5:i386 \
        gcc-s390x-linux-gnu libc6-dev-s390x-cross libc6-dbg:s390x \
        gcc-arm-linux-gnueabihf libc6-dev-armhf-cross libc6-dbg:armhf \
        gcc-aarch64-linux-gnu libc6-dev-arm64-cross libc6-dbg:arm64 \
        gcc-powerpc64le-linux-gnu libc6-dev-ppc64el-cross libc6-dbg:ppc64el \
        wine gcc-mingw-w64-x86-64

# Run a dummy command in wine to make it set up configuration
RUN wine64-stable xcopy || true
>>>>>>> 6d46dfbfa26f2e243b4eab1e8b34b7cc0e51401a
