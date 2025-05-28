New command line interface
--------------------------

A new `umkoin` command line tool has been added to make features more
discoverable and convenient to use. The `umkoin` tool just calls other
executables and does not implement any functionality on its own.  Specifically
`umkoin node` is a synonym for `umkoind`, `umkoin gui` is a synonym for
`umkoin-qt`, and `umkoin rpc` is a synonym for `umkoin-cli -named`. Other
commands and options can be listed with `umkoin help`. The new tool does not
replace other tools, so all existing commands should continue working and there
are no plans to deprecate them.
