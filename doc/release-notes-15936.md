GUI changes
-----------

Configuration changes made in the umkoin GUI (such as the pruning setting,
proxy settings, UPNP preferences) are now saved to `<datadir>/settings.json`
file rather than to the Qt settings backend (windows registry or unix desktop
config files), so these settings will now apply to umkoind, instead of being
ignored.

Also, the interaction between GUI settings and `umkoin.conf` settings is
simplified. Settings from `umkoin.conf` are now displayed normally in the GUI
settings dialog, instead of in a separate warning message ("Options set in this
dialog are overridden by the configuration file: -setting=value"). And these
settings can now be edited because `settings.json` values take precedence over
`umkoin.conf` values.
