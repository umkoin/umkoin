New command line interface
--------------------------

A new `umkoin` command line tool has been added to make features more
discoverable and convenient to use. The `umkoin` tool just calls other
executables and does not implement any functionality on its own. Specifically
`umkoin node` is a synonym for `umkoind`, `umkoin gui` is a synonym for
`umkoin-qt`, and `umkoin rpc` is a synonym for `umkoin-cli -named`. Other
commands and options can be listed with `umkoin help`. The new tool does not
replace other tools, so existing commands should continue working and there are
no plans to deprecate them.

Install changes
---------------

The `test_umkoin` executable is now located in `libexec/` rather than `bin/`.
It can still be executed directly, or accessed through the new `umkoin` command
line tool as `umkoin test`.

Other executables which are only part of source releases and not built by
default: `test_umkoin-qt`, `bench_umkoin`, `umkoin-chainstate`,
`umkoin-node`, and `umkoin-gui` are also now installed in `libexec/`
instead of `bin/` and can be accessed through the `umkoin` command line tool.
See `umkoin help` output for details.
