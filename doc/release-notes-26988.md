Tools and Utilities
--------

- CLI -addrinfo now returns the full set of known addresses. In previous versions (v22.0 - v30.0) the set of returned
  addresses was filtered for quality and recency. This was changed since it does not match the logic for selecting peers
  to connect to, which does not filter. Note: CLI -addrinfo now requires umkoind v26.0 or later, as it uses the
  getaddrmaninfo RPC internally. Users querying older, unmaintained node versions would need to use an older umkoin-cli
  version. (#26988)
