
Debian
====================
This directory contains files used to package umkoind/umkoin-qt
for Debian-based Linux systems. If you compile umkoind/umkoin-qt yourself, there are some useful files here.

## bitcoin: URI support ##


umkoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install umkoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your umkoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/bitcoin128.png` to `/usr/share/pixmaps`

umkoin-qt.protocol (KDE)

