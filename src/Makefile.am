# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# Pattern rule to print variables, e.g. make print-top_srcdir
print-%:
	@echo '$*' = '$($*)'

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS) $(GPROF_LDFLAGS) $(SANITIZER_LDFLAGS)
AM_CXXFLAGS = $(DEBUG_CXXFLAGS) $(HARDENED_CXXFLAGS) $(WARN_CXXFLAGS) $(NOWARN_CXXFLAGS) $(ERROR_CXXFLAGS) $(GPROF_CXXFLAGS) $(SANITIZER_CXXFLAGS)
AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(HARDENED_CPPFLAGS)
AM_LIBTOOLFLAGS = --preserve-dup-deps
PTHREAD_FLAGS = $(PTHREAD_CFLAGS) $(PTHREAD_LIBS)
EXTRA_LIBRARIES =

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

UMKOIN_INCLUDES=-I$(builddir) -I$(srcdir)/secp256k1/include $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS)

UMKOIN_INCLUDES += $(UNIVALUE_CFLAGS)

LIBUMKOIN_SERVER=libumkoin_server.a
LIBUMKOIN_COMMON=libumkoin_common.a
LIBUMKOIN_CONSENSUS=libumkoin_consensus.a
LIBUMKOIN_CLI=libumkoin_cli.a
LIBUMKOIN_UTIL=libumkoin_util.a
LIBUMKOIN_CRYPTO_BASE=crypto/libumkoin_crypto_base.a
LIBUMKOINQT=qt/libumkoinqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

if ENABLE_ZMQ
LIBUMKOIN_ZMQ=libumkoin_zmq.a
endif
if BUILD_UMKOIN_LIBS
LIBUMKOINCONSENSUS=libumkoinconsensus.la
endif
if ENABLE_WALLET
LIBUMKOIN_WALLET=libumkoin_wallet.a
LIBUMKOIN_WALLET_TOOL=libumkoin_wallet_tool.a
endif

LIBUMKOIN_CRYPTO= $(LIBUMKOIN_CRYPTO_BASE)
if ENABLE_SSE41
LIBUMKOIN_CRYPTO_SSE41 = crypto/libumkoin_crypto_sse41.a
LIBUMKOIN_CRYPTO += $(LIBUMKOIN_CRYPTO_SSE41)
endif
if ENABLE_AVX2
LIBUMKOIN_CRYPTO_AVX2 = crypto/libumkoin_crypto_avx2.a
LIBUMKOIN_CRYPTO += $(LIBUMKOIN_CRYPTO_AVX2)
endif
if ENABLE_SHANI
LIBUMKOIN_CRYPTO_SHANI = crypto/libumkoin_crypto_shani.a
LIBUMKOIN_CRYPTO += $(LIBUMKOIN_CRYPTO_SHANI)
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*.h) $(wildcard secp256k1/src/*.c) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBUMKOIN_CRYPTO) \
  $(LIBUMKOIN_UTIL) \
  $(LIBUMKOIN_COMMON) \
  $(LIBUMKOIN_CONSENSUS) \
  $(LIBUMKOIN_SERVER) \
  $(LIBUMKOIN_CLI) \
  $(LIBUMKOIN_WALLET) \
  $(LIBUMKOIN_WALLET_TOOL) \
  $(LIBUMKOIN_ZMQ)

lib_LTLIBRARIES = $(LIBUMKOINCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_UMKOIND
  bin_PROGRAMS += umkoind
endif

if BUILD_UMKOIN_NODE
  bin_PROGRAMS += umkoin-node
endif

if BUILD_UMKOIN_CLI
  bin_PROGRAMS += umkoin-cli
endif

if BUILD_UMKOIN_TX
  bin_PROGRAMS += umkoin-tx
endif

if ENABLE_WALLET
if BUILD_UMKOIN_WALLET
  bin_PROGRAMS += umkoin-wallet
endif
endif

if BUILD_UMKOIN_UTIL
  bin_PROGRAMS += umkoin-util
endif

.PHONY: FORCE check-symbols check-security
# umkoin core #
UMKOIN_CORE_H = \
  addrdb.h \
  addrman.h \
  attributes.h \
  banman.h \
  base58.h \
  bech32.h \
  blockencodings.h \
  blockfilter.h \
  bloom.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkqueue.h \
  clientversion.h \
  coins.h \
  compat.h \
  compat/assumptions.h \
  compat/byteswap.h \
  compat/cpuid.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  consensus/tx_check.h \
  consensus/tx_verify.h \
  core_io.h \
  core_memusage.h \
  cuckoocache.h \
  dbwrapper.h \
  flatfile.h \
  fs.h \
  httprpc.h \
  httpserver.h \
  i2p.h \
  index/base.h \
  index/blockfilterindex.h \
  index/disktxpos.h \
  index/txindex.h \
  indirectmap.h \
  init.h \
  interfaces/chain.h \
  interfaces/handler.h \
  interfaces/node.h \
  interfaces/wallet.h \
  key.h \
  key_io.h \
  logging.h \
  logging/timer.h \
  mapport.h \
  memusage.h \
  merkleblock.h \
  miner.h \
  net.h \
  net_permissions.h \
  net_processing.h \
  net_types.h \
  netaddress.h \
  netbase.h \
  netmessagemaker.h \
  node/coin.h \
  node/coinstats.h \
  node/context.h \
  node/psbt.h \
  node/transaction.h \
  node/ui_interface.h \
  node/utxo_snapshot.h \
  noui.h \
  optional.h \
  outputtype.h \
  policy/feerate.h \
  policy/fees.h \
  policy/policy.h \
  policy/rbf.h \
  policy/settings.h \
  pow.h \
  protocol.h \
  psbt.h \
  random.h \
  randomenv.h \
  reverse_iterator.h \
  rpc/blockchain.h \
  rpc/client.h \
  rpc/mining.h \
  rpc/protocol.h \
  rpc/rawtransaction_util.h \
  rpc/register.h \
  rpc/request.h \
  rpc/server.h \
  rpc/util.h \
  scheduler.h \
  script/descriptor.h \
  script/keyorigin.h \
  script/sigcache.h \
  script/sign.h \
  script/signingprovider.h \
  script/standard.h \
  shutdown.h \
  signet.h \
  streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/events.h \
  support/lockedpool.h \
  sync.h \
  threadinterrupt.h \
  threadsafety.h \
  timedata.h \
  torcontrol.h \
  txdb.h \
  txrequest.h \
  txmempool.h \
  undo.h \
  util/asmap.h \
  util/bip32.h \
  util/bytevectorhash.h \
  util/check.h \
  util/epochguard.h \
  util/error.h \
  util/fees.h \
  util/getuniquepath.h \
  util/golombrice.h \
  util/hasher.h \
  util/macros.h \
  util/memory.h \
  util/message.h \
  util/moneystr.h \
  util/rbf.h \
  util/readwritefile.h \
  util/ref.h \
  util/settings.h \
  util/sock.h \
  util/spanparsing.h \
  util/string.h \
  util/system.h \
  util/threadnames.h \
  util/time.h \
  util/trace.h \
  util/translation.h \
  util/ui_change_type.h \
  util/url.h \
  util/vector.h \
  validation.h \
  validationinterface.h \
  versionbits.h \
  versionbitsinfo.h \
  wallet/bdb.h \
  wallet/coincontrol.h \
  wallet/coinselection.h \
  wallet/context.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/dump.h \
  wallet/external_signer.h \
  wallet/external_signer_scriptpubkeyman.h \
  wallet/feebumper.h \
  wallet/fees.h \
  wallet/ismine.h \
  wallet/load.h \
  wallet/rpcsigner.h \
  wallet/rpcwallet.h \
  wallet/salvage.h \
  wallet/scriptpubkeyman.h \
  wallet/sqlite.h \
  wallet/wallet.h \
  wallet/walletdb.h \
  wallet/wallettool.h \
  wallet/walletutil.h \
  walletinitinterface.h \
  warnings.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h \
  zmq/zmqrpc.h \
  zmq/zmqutil.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh "$(abs_top_builddir)/src/obj/build.h" \
	  "$(abs_top_srcdir)"
libumkoin_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between umkoind and umkoin-qt
# Contains code accessing mempool and chain state that is meant to be separated
# from wallet and gui code (see node/README.md). Shared code should go in
# libumkoin_common or libumkoin_util libraries, instead.
libumkoin_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libumkoin_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libumkoin_server_a_SOURCES = \
  addrdb.cpp \
  addrman.cpp \
  banman.cpp \
  blockencodings.cpp \
  blockfilter.cpp \
  chain.cpp \
  consensus/tx_verify.cpp \
  dbwrapper.cpp \
  flatfile.cpp \
  httprpc.cpp \
  httpserver.cpp \
  i2p.cpp \
  index/base.cpp \
  index/blockfilterindex.cpp \
  index/txindex.cpp \
  init.cpp \
  mapport.cpp \
  miner.cpp \
  net.cpp \
  net_processing.cpp \
  node/coin.cpp \
  node/coinstats.cpp \
  node/context.cpp \
  node/interfaces.cpp \
  node/psbt.cpp \
  node/transaction.cpp \
  node/ui_interface.cpp \
  noui.cpp \
  policy/fees.cpp \
  policy/rbf.cpp \
  policy/settings.cpp \
  pow.cpp \
  rest.cpp \
  rpc/blockchain.cpp \
  rpc/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/rawtransaction.cpp \
  rpc/server.cpp \
  script/sigcache.cpp \
  shutdown.cpp \
  signet.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  txrequest.cpp \
  txmempool.cpp \
  validation.cpp \
  validationinterface.cpp \
  versionbits.cpp \
  $(UMKOIN_CORE_H)

if ENABLE_WALLET
libumkoin_server_a_SOURCES += wallet/init.cpp
endif
if !ENABLE_WALLET
libumkoin_server_a_SOURCES += dummywallet.cpp
endif

if ENABLE_ZMQ
libumkoin_zmq_a_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES) $(ZMQ_CFLAGS)
libumkoin_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libumkoin_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp \
  zmq/zmqrpc.cpp \
  zmq/zmqutil.cpp
endif


# wallet: shared between umkoind and umkoin-qt, but only linked
# when wallet enabled
libumkoin_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES) $(SQLITE_CFLAGS)
libumkoin_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libumkoin_wallet_a_SOURCES = \
  wallet/coincontrol.cpp \
  wallet/context.cpp \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/dump.cpp \
  wallet/external_signer_scriptpubkeyman.cpp \
  wallet/external_signer.cpp \
  wallet/feebumper.cpp \
  wallet/fees.cpp \
  wallet/interfaces.cpp \
  wallet/load.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcsigner.cpp \
  wallet/rpcwallet.cpp \
  wallet/scriptpubkeyman.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  wallet/walletutil.cpp \
  wallet/coinselection.cpp \
  $(UMKOIN_CORE_H)

if USE_SQLITE
libumkoin_wallet_a_SOURCES += wallet/sqlite.cpp
endif
if USE_BDB
libumkoin_wallet_a_SOURCES += wallet/bdb.cpp wallet/salvage.cpp
endif

libumkoin_wallet_tool_a_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES)
libumkoin_wallet_tool_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libumkoin_wallet_tool_a_SOURCES = \
  wallet/wallettool.cpp \
  $(UMKOIN_CORE_H)

# crypto primitives library
crypto_libumkoin_crypto_base_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libumkoin_crypto_base_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libumkoin_crypto_base_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/chacha_poly_aead.h \
  crypto/chacha_poly_aead.cpp \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/common.h \
  crypto/hkdf_sha256_32.cpp \
  crypto/hkdf_sha256_32.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/poly1305.h \
  crypto/poly1305.cpp \
  crypto/muhash.h \
  crypto/muhash.cpp \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha3.cpp \
  crypto/sha3.h \
  crypto/sha512.cpp \
  crypto/sha512.h \
  crypto/siphash.cpp \
  crypto/siphash.h

if USE_ASM
crypto_libumkoin_crypto_base_a_SOURCES += crypto/sha256_sse4.cpp
endif

crypto_libumkoin_crypto_sse41_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libumkoin_crypto_sse41_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libumkoin_crypto_sse41_a_CXXFLAGS += $(SSE41_CXXFLAGS)
crypto_libumkoin_crypto_sse41_a_CPPFLAGS += -DENABLE_SSE41
crypto_libumkoin_crypto_sse41_a_SOURCES = crypto/sha256_sse41.cpp

crypto_libumkoin_crypto_avx2_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libumkoin_crypto_avx2_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libumkoin_crypto_avx2_a_CXXFLAGS += $(AVX2_CXXFLAGS)
crypto_libumkoin_crypto_avx2_a_CPPFLAGS += -DENABLE_AVX2
crypto_libumkoin_crypto_avx2_a_SOURCES = crypto/sha256_avx2.cpp

crypto_libumkoin_crypto_shani_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libumkoin_crypto_shani_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libumkoin_crypto_shani_a_CXXFLAGS += $(SHANI_CXXFLAGS)
crypto_libumkoin_crypto_shani_a_CPPFLAGS += -DENABLE_SHANI
crypto_libumkoin_crypto_shani_a_SOURCES = crypto/sha256_shani.cpp

# consensus: shared between all executables that validate any consensus rules.
libumkoin_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES)
libumkoin_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libumkoin_consensus_a_SOURCES = \
  amount.h \
  arith_uint256.cpp \
  arith_uint256.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.h \
  consensus/tx_check.cpp \
  consensus/validation.h \
  hash.cpp \
  hash.h \
  prevector.h \
  primitives/block.cpp \
  primitives/block.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  pubkey.cpp \
  pubkey.h \
  script/umkoinconsensus.cpp \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  serialize.h \
  span.h \
  tinyformat.h \
  uint256.cpp \
  uint256.h \
  util/strencodings.cpp \
  util/strencodings.h \
  version.h

# common: shared between umkoind, and umkoin-qt and non-server tools
libumkoin_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES)
libumkoin_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libumkoin_common_a_SOURCES = \
  base58.cpp \
  bech32.cpp \
  bloom.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  key.cpp \
  key_io.cpp \
  merkleblock.cpp \
  netaddress.cpp \
  netbase.cpp \
  net_permissions.cpp \
  outputtype.cpp \
  policy/feerate.cpp \
  policy/policy.cpp \
  protocol.cpp \
  psbt.cpp \
  rpc/rawtransaction_util.cpp \
  rpc/util.cpp \
  scheduler.cpp \
  script/descriptor.cpp \
  script/sign.cpp \
  script/signingprovider.cpp \
  script/standard.cpp \
  versionbitsinfo.cpp \
  warnings.cpp \
  $(UMKOIN_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libumkoin_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES)
libumkoin_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libumkoin_util_a_SOURCES = \
  support/lockedpool.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  fs.cpp \
  interfaces/handler.cpp \
  logging.cpp \
  random.cpp \
  randomenv.cpp \
  rpc/request.cpp \
  support/cleanse.cpp \
  sync.cpp \
  threadinterrupt.cpp \
  util/asmap.cpp \
  util/bip32.cpp \
  util/bytevectorhash.cpp \
  util/error.cpp \
  util/fees.cpp \
  util/getuniquepath.cpp \
  util/hasher.cpp \
  util/sock.cpp \
  util/system.cpp \
  util/message.cpp \
  util/moneystr.cpp \
  util/rbf.cpp \
  util/readwritefile.cpp \
  util/settings.cpp \
  util/threadnames.cpp \
  util/spanparsing.cpp \
  util/strencodings.cpp \
  util/string.cpp \
  util/time.cpp \
  $(UMKOIN_CORE_H)

if USE_LIBEVENT
libumkoin_util_a_SOURCES += util/url.cpp
endif

if GLIBC_BACK_COMPAT
libumkoin_util_a_SOURCES += compat/glibc_compat.cpp
AM_LDFLAGS += $(COMPAT_LDFLAGS)
endif

# cli: shared between umkoin-cli and umkoin-qt
libumkoin_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES)
libumkoin_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libumkoin_cli_a_SOURCES = \
  compat/stdin.h \
  compat/stdin.cpp \
  rpc/client.cpp \
  $(UMKOIN_CORE_H)

nodist_libumkoin_util_a_SOURCES = $(srcdir)/obj/build.h
#

# umkoind & umkoin-node binaries #
umkoin_daemon_sources = umkoind.cpp
umkoin_bin_cppflags = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES)
umkoin_bin_cxxflags = $(AM_CXXFLAGS) $(PIE_FLAGS)
umkoin_bin_ldflags = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
umkoin_daemon_sources += umkoind-res.rc
endif

umkoin_bin_ldadd = \
  $(LIBUMKOIN_WALLET) \
  $(LIBUMKOIN_COMMON) \
  $(LIBUMKOIN_UTIL) \
  $(LIBUNIVALUE) \
  $(LIBUMKOIN_ZMQ) \
  $(LIBUMKOIN_CONSENSUS) \
  $(LIBUMKOIN_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

umkoin_bin_ldadd += $(BOOST_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(NATPMP_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS) $(SQLITE_LIBS)

umkoind_SOURCES = $(umkoin_daemon_sources)
umkoind_CPPFLAGS = $(umkoin_bin_cppflags)
umkoind_CXXFLAGS = $(umkoin_bin_cxxflags)
umkoind_LDFLAGS = $(umkoin_bin_ldflags)
umkoind_LDADD = $(LIBUMKOIN_SERVER) $(umkoin_bin_ldadd)

umkoin_node_SOURCES = $(umkoin_daemon_sources)
umkoin_node_CPPFLAGS = $(umkoin_bin_cppflags)
umkoin_node_CXXFLAGS = $(umkoin_bin_cxxflags)
umkoin_node_LDFLAGS = $(umkoin_bin_ldflags)
umkoin_node_LDADD = $(LIBUMKOIN_SERVER) $(umkoin_bin_ldadd)

# umkoin-cli binary #
umkoin_cli_SOURCES = umkoin-cli.cpp
umkoin_cli_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES) $(EVENT_CFLAGS)
umkoin_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
umkoin_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
umkoin_cli_SOURCES += umkoin-cli-res.rc
endif

umkoin_cli_LDADD = \
  $(LIBUMKOIN_CLI) \
  $(LIBUNIVALUE) \
  $(LIBUMKOIN_UTIL) \
  $(LIBUMKOIN_CRYPTO)

umkoin_cli_LDADD += $(BOOST_LIBS) $(EVENT_LIBS)
#

# umkoin-tx binary #
umkoin_tx_SOURCES = umkoin-tx.cpp
umkoin_tx_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES)
umkoin_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
umkoin_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
umkoin_tx_SOURCES += umkoin-tx-res.rc
endif

umkoin_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBUMKOIN_COMMON) \
  $(LIBUMKOIN_UTIL) \
  $(LIBUMKOIN_CONSENSUS) \
  $(LIBUMKOIN_CRYPTO) \
  $(LIBSECP256K1)

umkoin_tx_LDADD += $(BOOST_LIBS)
#

# umkoin-wallet binary #
umkoin_wallet_SOURCES = umkoin-wallet.cpp
umkoin_wallet_CPPFLAGS = $(umkoin_bin_cppflags)
umkoin_wallet_CXXFLAGS = $(umkoin_bin_cxxflags)
umkoin_wallet_LDFLAGS = $(umkoin_bin_ldflags)
umkoin_wallet_LDADD = $(LIBUMKOIN_WALLET_TOOL) $(umkoin_bin_ldadd)

if TARGET_WINDOWS
umkoin_wallet_SOURCES += umkoin-wallet-res.rc
endif
#

# umkoin-util binary #
umkoin_util_SOURCES = umkoin-util.cpp
umkoin_util_CPPFLAGS = $(AM_CPPFLAGS) $(UMKOIN_INCLUDES)
umkoin_util_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
umkoin_util_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
umkoin_util_SOURCES += umkoin-util-res.rc
endif

umkoin_util_LDADD = \
  $(LIBUMKOIN_COMMON) \
  $(LIBUMKOIN_UTIL) \
  $(LIBUNIVALUE) \
  $(LIBUMKOIN_CONSENSUS) \
  $(LIBUMKOIN_CRYPTO) \
  $(LIBSECP256K1)

umkoin_util_LDADD += $(BOOST_LIBS)
#

# umkoinconsensus library #
if BUILD_UMKOIN_LIBS
include_HEADERS = script/umkoinconsensus.h
libumkoinconsensus_la_SOURCES = support/cleanse.cpp $(crypto_libumkoin_crypto_base_a_SOURCES) $(libumkoin_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libumkoinconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libumkoinconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libumkoinconsensus_la_LIBADD = $(LIBSECP256K1)
libumkoinconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_UMKOIN_INTERNAL
libumkoinconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crc32c/src/*.gcda crc32c/src/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += index/*.gcda index/*.gcno
CLEANFILES += interfaces/*.gcda interfaces/*.gcno
CLEANFILES += node/*.gcda node/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += rpc/*.gcda rpc/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += util/*.gcda util/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno
CLEANFILES += obj/build.h

EXTRA_DIST = $(CTAES_DIST)


config/umkoin-config.h: config/stamp-h1
	@$(MAKE) -C $(top_builddir) $(subdir)/$(@)
config/stamp-h1: $(top_srcdir)/$(subdir)/config/umkoin-config.h.in $(top_builddir)/config.status
	$(AM_V_at)$(MAKE) -C $(top_builddir) $(subdir)/$(@)
$(top_srcdir)/$(subdir)/config/umkoin-config.h.in:  $(am__configure_deps)
	$(AM_V_at)$(MAKE) -C $(top_srcdir) $(subdir)/config/umkoin-config.h.in

clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -f config.h
	-rm -rf test/__pycache__

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

check-symbols: $(bin_PROGRAMS)
if TARGET_DARWIN
	@echo "Checking macOS dynamic libraries..."
	$(AM_V_at) OTOOL=$(OTOOL) $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS)
endif

if TARGET_WINDOWS
	@echo "Checking Windows dynamic libraries..."
	$(AM_V_at) OBJDUMP=$(OBJDUMP) $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS)
endif

if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) CPPFILT=$(CPPFILT) $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) OBJDUMP=$(OBJDUMP) OTOOL=$(OTOOL) $(PYTHON) $(top_srcdir)/contrib/devtools/security-check.py $(bin_PROGRAMS)
endif

if EMBEDDED_LEVELDB
include Makefile.crc32c.include
include Makefile.leveldb.include
endif

include Makefile.test_util.include
include Makefile.test_fuzz.include

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_BENCH
include Makefile.bench.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

if ENABLE_QT_TESTS
include Makefile.qttest.include
endif
