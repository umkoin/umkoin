#!/bin/bash

export LC_ALL=C
TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
BUILDDIR=${BUILDDIR:-$TOPDIR}

BINDIR=${BINDIR:-$BUILDDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

UMKOIND=${UMKOIND:-$BINDIR/umkoind}
UMKOINCLI=${UMKOINCLI:-$BINDIR/umkoin-cli}
UMKOINTX=${UMKOINTX:-$BINDIR/umkoin-tx}
UMKOINQT=${UMKOINQT:-$BINDIR/qt/umkoin-qt}

[ ! -x $UMKOIND ] && echo "$UMKOIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
UMKVER=($($UMKOINCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for umkoind if --version-string is not set,
# but has different outcomes for umkoin-qt and umkoin-cli.
echo "[COPYRIGHT]" > footer.h2m
$UMKOIND --version | sed -n '1!p' >> footer.h2m

for cmd in $UMKOIND $UMKOINCLI $UMKOINTX $UMKOINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${UMKVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${UMKVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
