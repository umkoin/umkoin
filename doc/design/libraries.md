# Libraries

| Name                     | Description |
|--------------------------|-------------|
| *libumkoin_cli*         | RPC client functionality used by *umkoin-cli* executable |
| *libumkoin_common*      | Home for common functionality shared by different executables and libraries. Similar to *libumkoin_util*, but higher-level (see [Dependencies](#dependencies)). |
| *libumkoin_consensus*   | Stable, backwards-compatible consensus functionality used by *libumkoin_node* and *libumkoin_wallet*. |
| *libumkoin_kernel*      | Consensus engine and support library used for validation by *libumkoin_node*. |
| *libumkoinqt*           | GUI functionality used by *umkoin-qt* and *umkoin-gui* executables. |
| *libumkoin_ipc*         | IPC functionality used by *umkoin-node*, *umkoin-wallet*, *umkoin-gui* executables to communicate when [`--enable-multiprocess`](multiprocess.md) is used. |
| *libumkoin_node*        | P2P and RPC server functionality used by *umkoind* and *umkoin-qt* executables. |
| *libumkoin_util*        | Home for common functionality shared by different executables and libraries. Similar to *libumkoin_common*, but lower-level (see [Dependencies](#dependencies)). |
| *libumkoin_wallet*      | Wallet functionality used by *umkoind* and *umkoin-wallet* executables. |
| *libumkoin_wallet_tool* | Lower-level wallet functionality used by *umkoin-wallet* executable. |
| *libumkoin_zmq*         | [ZeroMQ](../zmq.md) functionality used by *umkoind* and *umkoin-qt* executables. |

## Conventions

- Most libraries are internal libraries and have APIs which are completely unstable! There are few or no restrictions on backwards compatibility or rules about external dependencies. An exception is *libumkoin_kernel*, which, at some future point, will have a documented external interface.

- Generally each library should have a corresponding source directory and namespace. Source code organization is a work in progress, so it is true that some namespaces are applied inconsistently, and if you look at [`libumkoin_*_SOURCES`](../../src/Makefile.am) lists you can see that many libraries pull in files from outside their source directory. But when working with libraries, it is good to follow a consistent pattern like:

  - *libumkoin_node* code lives in `src/node/` in the `node::` namespace
  - *libumkoin_wallet* code lives in `src/wallet/` in the `wallet::` namespace
  - *libumkoin_ipc* code lives in `src/ipc/` in the `ipc::` namespace
  - *libumkoin_util* code lives in `src/util/` in the `util::` namespace
  - *libumkoin_consensus* code lives in `src/consensus/` in the `Consensus::` namespace

## Dependencies

- Libraries should minimize what other libraries they depend on, and only reference symbols following the arrows shown in the dependency graph below:

<table><tr><td>

```mermaid

%%{ init : { "flowchart" : { "curve" : "basis" }}}%%

graph TD;

umkoin-cli[umkoin-cli]-->libumkoin_cli;

umkoind[umkoind]-->libumkoin_node;
umkoind[umkoind]-->libumkoin_wallet;

umkoin-qt[umkoin-qt]-->libumkoin_node;
umkoin-qt[umkoin-qt]-->libumkoinqt;
umkoin-qt[umkoin-qt]-->libumkoin_wallet;

umkoin-wallet[umkoin-wallet]-->libumkoin_wallet;
umkoin-wallet[umkoin-wallet]-->libumkoin_wallet_tool;

libumkoin_cli-->libumkoin_util;
libumkoin_cli-->libumkoin_common;

libumkoin_common-->libumkoin_consensus;
libumkoin_common-->libumkoin_util;

libumkoin_kernel-->libumkoin_consensus;
libumkoin_kernel-->libumkoin_util;

libumkoin_node-->libumkoin_consensus;
libumkoin_node-->libumkoin_kernel;
libumkoin_node-->libumkoin_common;
libumkoin_node-->libumkoin_util;

libumkoinqt-->libumkoin_common;
libumkoinqt-->libumkoin_util;

libumkoin_wallet-->libumkoin_common;
libumkoin_wallet-->libumkoin_util;

libumkoin_wallet_tool-->libumkoin_wallet;
libumkoin_wallet_tool-->libumkoin_util;

classDef bold stroke-width:2px, font-weight:bold, font-size: smaller;
class umkoin-qt,umkoind,umkoin-cli,umkoin-wallet bold
```
</td></tr><tr><td>

**Dependency graph**. Arrows show linker symbol dependencies. *Consensus* lib depends on nothing. *Util* lib is depended on by everything. *Kernel* lib depends only on consensus and util.

</td></tr></table>

- The graph shows what _linker symbols_ (functions and variables) from each library other libraries can call and reference directly, but it is not a call graph. For example, there is no arrow connecting *libumkoin_wallet* and *libumkoin_node* libraries, because these libraries are intended to be modular and not depend on each other's internal implementation details. But wallet code is still able to call node code indirectly through the `interfaces::Chain` abstract class in [`interfaces/chain.h`](../../src/interfaces/chain.h) and node code calls wallet code through the `interfaces::ChainClient` and `interfaces::Chain::Notifications` abstract classes in the same file. In general, defining abstract classes in [`src/interfaces/`](../../src/interfaces/) can be a convenient way of avoiding unwanted direct dependencies or circular dependencies between libraries.

- *libumkoin_consensus* should be a standalone dependency that any library can depend on, and it should not depend on any other libraries itself.

- *libumkoin_util* should also be a standalone dependency that any library can depend on, and it should not depend on other internal libraries.

- *libumkoin_common* should serve a similar function as *libumkoin_util* and be a place for miscellaneous code used by various daemon, GUI, and CLI applications and libraries to live. It should not depend on anything other than *libumkoin_util* and *libumkoin_consensus*. The boundary between _util_ and _common_ is a little fuzzy but historically _util_ has been used for more generic, lower-level things like parsing hex, and _common_ has been used for umkoin-specific, higher-level things like parsing base58. The difference between util and common is mostly important because *libumkoin_kernel* is not supposed to depend on *libumkoin_common*, only *libumkoin_util*. In general, if it is ever unclear whether it is better to add code to *util* or *common*, it is probably better to add it to *common* unless it is very generically useful or useful particularly to include in the kernel.


- *libumkoin_kernel* should only depend on *libumkoin_util* and *libumkoin_consensus*.

- The only thing that should depend on *libumkoin_kernel* internally should be *libumkoin_node*. GUI and wallet libraries *libumkoinqt* and *libumkoin_wallet* in particular should not depend on *libumkoin_kernel* and the unneeded functionality it would pull in, like block validation. To the extent that GUI and wallet code need scripting and signing functionality, they should be get able it from *libumkoin_consensus*, *libumkoin_common*, and *libumkoin_util*, instead of *libumkoin_kernel*.

- GUI, node, and wallet code internal implementations should all be independent of each other, and the *libumkoinqt*, *libumkoin_node*, *libumkoin_wallet* libraries should never reference each other's symbols. They should only call each other through [`src/interfaces/`](`../../src/interfaces/`) abstract interfaces.

## Work in progress

- Validation code is moving from *libbitcoin_node* to *libbitcoin_kernel* as part of [The libbitcoinkernel Project #24303](https://github.com/bitcoin/bitcoin/issues/24303)
- Source code organization is discussed in general in [Library source code organization #15732](https://github.com/bitcoin/bitcoin/issues/15732)
