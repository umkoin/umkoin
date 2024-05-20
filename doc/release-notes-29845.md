RPC
---

- the `warnings` field in `getblockchaininfo`, `getmininginfo` and
  `getnetworkinfo` now returns all the active node warnings as an array
  of strings, instead of just a single warning. The current behaviour
  can temporarily be restored by running umkoind with configuration
  option `-deprecatedrpc=warnings`.