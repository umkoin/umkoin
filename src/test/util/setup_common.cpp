// Copyright (c) 2011-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/util/setup_common.h>

#include <kernel/validation_cache_sizes.h>

#include <addrman.h>
#include <banman.h>
#include <chainparams.h>
#include <common/system.h>
#include <common/url.h>
#include <consensus/consensus.h>
#include <consensus/params.h>
#include <consensus/validation.h>
#include <crypto/sha256.h>
#include <init.h>
#include <init/common.h>
#include <interfaces/chain.h>
#include <kernel/mempool_entry.h>
#include <logging.h>
#include <net.h>
#include <net_processing.h>
#include <node/blockstorage.h>
#include <node/chainstate.h>
#include <node/context.h>
#include <node/kernel_notifications.h>
#include <node/mempool_args.h>
#include <node/miner.h>
#include <node/peerman_args.h>
#include <node/validation_cache_args.h>
#include <noui.h>
#include <policy/fees.h>
#include <policy/fees_args.h>
#include <pow.h>
#include <random.h>
#include <rpc/blockchain.h>
#include <rpc/register.h>
#include <rpc/server.h>
#include <scheduler.h>
#include <script/sigcache.h>
#include <shutdown.h>
#include <streams.h>
#include <test/util/net.h>
#include <test/util/random.h>
#include <test/util/txmempool.h>
#include <timedata.h>
#include <txdb.h>
#include <txmempool.h>
#include <util/chaintype.h>
#include <util/strencodings.h>
#include <util/string.h>
#include <util/thread.h>
#include <util/threadnames.h>
#include <util/time.h>
#include <util/translation.h>
#include <util/vector.h>
#include <validation.h>
#include <validationinterface.h>
#include <walletinitinterface.h>

#include <algorithm>
#include <functional>
#include <stdexcept>

using kernel::BlockTreeDB;
using kernel::ValidationCacheSizes;
using node::ApplyArgsManOptions;
using node::BlockAssembler;
using node::BlockManager;
using node::CalculateCacheSizes;
using node::KernelNotifications;
using node::LoadChainstate;
using node::RegenerateCommitments;
using node::VerifyLoadedChainstate;

const std::function<std::string(const char*)> G_TRANSLATION_FUN = nullptr;
UrlDecodeFn* const URL_DECODE = nullptr;

/** Random context to get unique temp data dirs. Separate from g_insecure_rand_ctx, which can be seeded from a const env var */
static FastRandomContext g_insecure_rand_ctx_temp_path;

std::ostream& operator<<(std::ostream& os, const uint256& num)
{
    os << num.ToString();
    return os;
}

BasicTestingSetup::BasicTestingSetup(const ChainType chainType, const std::vector<const char*>& extra_args)
    : m_path_root{fs::temp_directory_path() / "test_common_" PACKAGE_NAME / g_insecure_rand_ctx_temp_path.rand256().ToString()},
      m_args{}
{
    m_node.args = &gArgs;
    std::vector<const char*> arguments = Cat(
        {
            "dummy",
            "-printtoconsole=0",
            "-logsourcelocations",
            "-logtimemicros",
            "-logthreadnames",
            "-loglevel=trace",
            "-debug",
            "-debugexclude=libevent",
            "-debugexclude=leveldb",
        },
        extra_args);
    if (G_TEST_COMMAND_LINE_ARGUMENTS) {
        arguments = Cat(arguments, G_TEST_COMMAND_LINE_ARGUMENTS());
    }
    util::ThreadRename("test");
    fs::create_directories(m_path_root);
    m_args.ForceSetArg("-datadir", fs::PathToString(m_path_root));
    gArgs.ForceSetArg("-datadir", fs::PathToString(m_path_root));
    gArgs.ClearPathCache();
    {
        SetupServerArgs(*m_node.args);
        std::string error;
        if (!m_node.args->ParseParameters(arguments.size(), arguments.data(), error)) {
            m_node.args->ClearArgs();
            throw std::runtime_error{error};
        }
    }
    SelectParams(chainType);
    SeedInsecureRand();
    if (G_TEST_LOG_FUN) LogInstance().PushBackCallback(G_TEST_LOG_FUN);
    InitLogging(*m_node.args);
    AppInitParameterInteraction(*m_node.args);
    LogInstance().StartLogging();
    m_node.kernel = std::make_unique<kernel::Context>();
    SetupEnvironment();
    SetupNetworking();

    ValidationCacheSizes validation_cache_sizes{};
    ApplyArgsManOptions(*m_node.args, validation_cache_sizes);
    Assert(InitSignatureCache(validation_cache_sizes.signature_cache_bytes));
    Assert(InitScriptExecutionCache(validation_cache_sizes.script_execution_cache_bytes));

    m_node.chain = interfaces::MakeChain(m_node);
    static bool noui_connected = false;
    if (!noui_connected) {
        noui_connect();
        noui_connected = true;
    }
}

BasicTestingSetup::~BasicTestingSetup()
{
    SetMockTime(0s); // Reset mocktime for following tests
    LogInstance().DisconnectTestLogger();
    fs::remove_all(m_path_root);
    gArgs.ClearArgs();
}

ChainTestingSetup::ChainTestingSetup(const ChainType chainType, const std::vector<const char*>& extra_args)
    : BasicTestingSetup(chainType, extra_args)
{
    const CChainParams& chainparams = Params();

    // We have to run a scheduler thread to prevent ActivateBestChain
    // from blocking due to queue overrun.
    m_node.scheduler = std::make_unique<CScheduler>();
    m_node.scheduler->m_service_thread = std::thread(util::TraceThread, "scheduler", [&] { m_node.scheduler->serviceQueue(); });
    GetMainSignals().RegisterBackgroundSignalScheduler(*m_node.scheduler);

    m_node.fee_estimator = std::make_unique<CBlockPolicyEstimator>(FeeestPath(*m_node.args), DEFAULT_ACCEPT_STALE_FEE_ESTIMATES);
    m_node.mempool = std::make_unique<CTxMemPool>(MemPoolOptionsForTest(m_node));

    m_cache_sizes = CalculateCacheSizes(m_args);

    m_node.notifications = std::make_unique<KernelNotifications>(m_node.exit_status);

    const ChainstateManager::Options chainman_opts{
        .chainparams = chainparams,
        .datadir = m_args.GetDataDirNet(),
        .adjusted_time_callback = GetAdjustedTime,
        .check_block_index = true,
        .notifications = *m_node.notifications,
    };
    const BlockManager::Options blockman_opts{
        .chainparams = chainman_opts.chainparams,
        .blocks_dir = m_args.GetBlocksDirPath(),
        .notifications = chainman_opts.notifications,
    };
    m_node.chainman = std::make_unique<ChainstateManager>(m_node.kernel->interrupt, chainman_opts, blockman_opts);
    m_node.chainman->m_blockman.m_block_tree_db = std::make_unique<BlockTreeDB>(DBParams{
        .path = m_args.GetDataDirNet() / "blocks" / "index",
        .cache_bytes = static_cast<size_t>(m_cache_sizes.block_tree_db),
        .memory_only = true});

    constexpr int script_check_threads = 2;
    StartScriptCheckWorkerThreads(script_check_threads);
}

ChainTestingSetup::~ChainTestingSetup()
{
    if (m_node.scheduler) m_node.scheduler->stop();
    StopScriptCheckWorkerThreads();
    GetMainSignals().FlushBackgroundCallbacks();
    GetMainSignals().UnregisterBackgroundSignalScheduler();
    m_node.connman.reset();
    m_node.banman.reset();
    m_node.addrman.reset();
    m_node.netgroupman.reset();
    m_node.args = nullptr;
    m_node.mempool.reset();
    m_node.scheduler.reset();
    m_node.chainman.reset();
}

void ChainTestingSetup::LoadVerifyActivateChainstate()
{
    auto& chainman{*Assert(m_node.chainman)};
    node::ChainstateLoadOptions options;
    options.mempool = Assert(m_node.mempool.get());
    options.block_tree_db_in_memory = m_block_tree_db_in_memory;
    options.coins_db_in_memory = m_coins_db_in_memory;
    options.reindex = node::fReindex;
    options.reindex_chainstate = m_args.GetBoolArg("-reindex-chainstate", false);
    options.prune = chainman.m_blockman.IsPruneMode();
    options.check_blocks = m_args.GetIntArg("-checkblocks", DEFAULT_CHECKBLOCKS);
    options.check_level = m_args.GetIntArg("-checklevel", DEFAULT_CHECKLEVEL);
    options.require_full_verification = m_args.IsArgSet("-checkblocks") || m_args.IsArgSet("-checklevel");
    auto [status, error] = LoadChainstate(chainman, m_cache_sizes, options);
    assert(status == node::ChainstateLoadStatus::SUCCESS);

    std::tie(status, error) = VerifyLoadedChainstate(chainman, options);
    assert(status == node::ChainstateLoadStatus::SUCCESS);

    BlockValidationState state;
    if (!chainman.ActiveChainstate().ActivateBestChain(state)) {
        throw std::runtime_error(strprintf("ActivateBestChain failed. (%s)", state.ToString()));
    }
}

TestingSetup::TestingSetup(
    const ChainType chainType,
    const std::vector<const char*>& extra_args,
    const bool coins_db_in_memory,
    const bool block_tree_db_in_memory)
    : ChainTestingSetup(chainType, extra_args)
{
    m_coins_db_in_memory = coins_db_in_memory;
    m_block_tree_db_in_memory = block_tree_db_in_memory;
    // Ideally we'd move all the RPC tests to the functional testing framework
    // instead of unit tests, but for now we need these here.
    RegisterAllCoreRPCCommands(tableRPC);

    LoadVerifyActivateChainstate();

    m_node.netgroupman = std::make_unique<NetGroupManager>(/*asmap=*/std::vector<bool>());
    m_node.addrman = std::make_unique<AddrMan>(*m_node.netgroupman,
                                               /*deterministic=*/false,
                                               m_node.args->GetIntArg("-checkaddrman", 0));
    m_node.banman = std::make_unique<BanMan>(m_args.GetDataDirBase() / "banlist", nullptr, DEFAULT_MISBEHAVING_BANTIME);
    m_node.connman = std::make_unique<ConnmanTestMsg>(0x1337, 0x1337, *m_node.addrman, *m_node.netgroupman, Params()); // Deterministic randomness for tests.
    PeerManager::Options peerman_opts;
    ApplyArgsManOptions(*m_node.args, peerman_opts);
    peerman_opts.deterministic_rng = true;
    m_node.peerman = PeerManager::make(*m_node.connman, *m_node.addrman,
                                       m_node.banman.get(), *m_node.chainman,
                                       *m_node.mempool, peerman_opts);

    {
        CConnman::Options options;
        options.m_msgproc = m_node.peerman.get();
        m_node.connman->Init(options);
    }
}

TestChain100Setup::TestChain100Setup(
        const ChainType chain_type,
        const std::vector<const char*>& extra_args,
        const bool coins_db_in_memory,
        const bool block_tree_db_in_memory)
    : TestingSetup{ChainType::REGTEST, extra_args, coins_db_in_memory, block_tree_db_in_memory}
{
    SetMockTime(1598887952);
    constexpr std::array<unsigned char, 32> vchKey = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}};
    coinbaseKey.Set(vchKey.begin(), vchKey.end(), true);

    // Generate a 100-block chain:
    this->mineBlocks(COINBASE_MATURITY);

    {
        LOCK(::cs_main);
        assert(
            m_node.chainman->ActiveChain().Tip()->GetBlockHash().ToString() ==
            "571d80a9967ae599cec0448b0b0ba1cfb606f584d8069bd7166b86854ba7a191");
    }
}

void TestChain100Setup::mineBlocks(int num_blocks)
{
    CScript scriptPubKey = CScript() << ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;
    for (int i = 0; i < num_blocks; i++) {
        std::vector<CMutableTransaction> noTxns;
        CBlock b = CreateAndProcessBlock(noTxns, scriptPubKey);
        SetMockTime(GetTime() + 1);
        m_coinbase_txns.push_back(b.vtx[0]);
    }
}

CBlock TestChain100Setup::CreateBlock(
    const std::vector<CMutableTransaction>& txns,
    const CScript& scriptPubKey,
    Chainstate& chainstate)
{
    CBlock block = BlockAssembler{chainstate, nullptr}.CreateNewBlock(scriptPubKey)->block;

    Assert(block.vtx.size() == 1);
    for (const CMutableTransaction& tx : txns) {
        block.vtx.push_back(MakeTransactionRef(tx));
    }
    RegenerateCommitments(block, *Assert(m_node.chainman));

    while (!CheckProofOfWork(block.GetHash(), block.nBits, m_node.chainman->GetConsensus())) ++block.nNonce;

    return block;
}

CBlock TestChain100Setup::CreateAndProcessBlock(
    const std::vector<CMutableTransaction>& txns,
    const CScript& scriptPubKey,
    Chainstate* chainstate)
{
    if (!chainstate) {
        chainstate = &Assert(m_node.chainman)->ActiveChainstate();
    }

    CBlock block = this->CreateBlock(txns, scriptPubKey, *chainstate);
    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    Assert(m_node.chainman)->ProcessNewBlock(shared_pblock, true, true, nullptr);

    return block;
}


CMutableTransaction TestChain100Setup::CreateValidMempoolTransaction(CTransactionRef input_transaction,
                                                                     int input_vout,
                                                                     int input_height,
                                                                     CKey input_signing_key,
                                                                     CScript output_destination,
                                                                     CAmount output_amount,
                                                                     bool submit)
{
    // Transaction we will submit to the mempool
    CMutableTransaction mempool_txn;

    // Create an input
    COutPoint outpoint_to_spend(input_transaction->GetHash(), input_vout);
    CTxIn input(outpoint_to_spend);
    mempool_txn.vin.push_back(input);

    // Create an output
    CTxOut output(output_amount, output_destination);
    mempool_txn.vout.push_back(output);

    // Sign the transaction
    // - Add the signing key to a keystore
    FillableSigningProvider keystore;
    keystore.AddKey(input_signing_key);
    // - Populate a CoinsViewCache with the unspent output
    CCoinsView coins_view;
    CCoinsViewCache coins_cache(&coins_view);
    AddCoins(coins_cache, *input_transaction.get(), input_height);
    // - Use GetCoin to properly populate utxo_to_spend,
    Coin utxo_to_spend;
    assert(coins_cache.GetCoin(outpoint_to_spend, utxo_to_spend));
    // - Then add it to a map to pass in to SignTransaction
    std::map<COutPoint, Coin> input_coins;
    input_coins.insert({outpoint_to_spend, utxo_to_spend});
    // - Default signature hashing type
    int nHashType = SIGHASH_ALL;
    std::map<int, bilingual_str> input_errors;
    assert(SignTransaction(mempool_txn, &keystore, input_coins, nHashType, input_errors));

    // If submit=true, add transaction to the mempool.
    if (submit) {
        LOCK(cs_main);
        const MempoolAcceptResult result = m_node.chainman->ProcessTransaction(MakeTransactionRef(mempool_txn));
        assert(result.m_result_type == MempoolAcceptResult::ResultType::VALID);
    }

    return mempool_txn;
}

std::vector<CTransactionRef> TestChain100Setup::PopulateMempool(FastRandomContext& det_rand, size_t num_transactions, bool submit)
{
    std::vector<CTransactionRef> mempool_transactions;
    std::deque<std::pair<COutPoint, CAmount>> unspent_prevouts;
    std::transform(m_coinbase_txns.begin(), m_coinbase_txns.end(), std::back_inserter(unspent_prevouts),
        [](const auto& tx){ return std::make_pair(COutPoint(tx->GetHash(), 0), tx->vout[0].nValue); });
    while (num_transactions > 0 && !unspent_prevouts.empty()) {
        // The number of inputs and outputs are random, between 1 and 24.
        CMutableTransaction mtx = CMutableTransaction();
        const size_t num_inputs = det_rand.randrange(24) + 1;
        CAmount total_in{0};
        for (size_t n{0}; n < num_inputs; ++n) {
            if (unspent_prevouts.empty()) break;
            const auto& [prevout, amount] = unspent_prevouts.front();
            mtx.vin.emplace_back(prevout, CScript());
            total_in += amount;
            unspent_prevouts.pop_front();
        }
        const size_t num_outputs = det_rand.randrange(24) + 1;
        const CAmount fee = 100 * det_rand.randrange(30);
        const CAmount amount_per_output = (total_in - fee) / num_outputs;
        for (size_t n{0}; n < num_outputs; ++n) {
            CScript spk = CScript() << CScriptNum(num_transactions + n);
            mtx.vout.emplace_back(amount_per_output, spk);
        }
        CTransactionRef ptx = MakeTransactionRef(mtx);
        mempool_transactions.push_back(ptx);
        if (amount_per_output > 3000) {
            // If the value is high enough to fund another transaction + fees, keep track of it so
            // it can be used to build a more complex transaction graph. Insert randomly into
            // unspent_prevouts for extra randomness in the resulting structures.
            for (size_t n{0}; n < num_outputs; ++n) {
                unspent_prevouts.emplace_back(COutPoint(ptx->GetHash(), n), amount_per_output);
                std::swap(unspent_prevouts.back(), unspent_prevouts[det_rand.randrange(unspent_prevouts.size())]);
            }
        }
        if (submit) {
            LOCK2(cs_main, m_node.mempool->cs);
            LockPoints lp;
            m_node.mempool->addUnchecked(CTxMemPoolEntry(ptx, /*fee=*/(total_in - num_outputs * amount_per_output),
                                                         /*time=*/0, /*entry_height=*/1, /*entry_sequence=*/0,
                                                         /*spends_coinbase=*/false, /*sigops_cost=*/4, lp));
        }
        --num_transactions;
    }
    return mempool_transactions;
}

void TestChain100Setup::MockMempoolMinFee(const CFeeRate& target_feerate)
{
    LOCK2(cs_main, m_node.mempool->cs);
    // Transactions in the mempool will affect the new minimum feerate.
    assert(m_node.mempool->size() == 0);
    // The target feerate cannot be too low...
    // ...otherwise the transaction's feerate will need to be negative.
    assert(target_feerate > m_node.mempool->m_incremental_relay_feerate);
    // ...otherwise this is not meaningful. The feerate policy uses the maximum of both feerates.
    assert(target_feerate > m_node.mempool->m_min_relay_feerate);

    // Manually create an invalid transaction. Manually set the fee in the CTxMemPoolEntry to
    // achieve the exact target feerate.
    CMutableTransaction mtx = CMutableTransaction();
    mtx.vin.emplace_back(COutPoint{g_insecure_rand_ctx.rand256(), 0});
    mtx.vout.emplace_back(1 * COIN, GetScriptForDestination(WitnessV0ScriptHash(CScript() << OP_TRUE)));
    const auto tx{MakeTransactionRef(mtx)};
    LockPoints lp;
    // The new mempool min feerate is equal to the removed package's feerate + incremental feerate.
    const auto tx_fee = target_feerate.GetFee(GetVirtualTransactionSize(*tx)) -
        m_node.mempool->m_incremental_relay_feerate.GetFee(GetVirtualTransactionSize(*tx));
    m_node.mempool->addUnchecked(CTxMemPoolEntry(tx, /*fee=*/tx_fee,
                                                 /*time=*/0, /*entry_height=*/1, /*entry_sequence=*/0,
                                                 /*spends_coinbase=*/true, /*sigops_cost=*/1, lp));
    m_node.mempool->TrimToSize(0);
    assert(m_node.mempool->GetMinFee() == target_feerate);
}
/**
 * @returns a real block (000000000000eb0d0499247d81f9368107ab176acd0fddee7e442996bbde97f3)
 *      with 9 txs.
 */
CBlock getBlockeb0d0()
{
    CBlock block;
    CDataStream stream(ParseHex("/00000020af876e6efcd7c6a3d8768ce32cce1db4a6045123ea73ea2d47ac0000000000002abd5d5b435c03a757ae5c8990baaaa1030f07c2b8013b793c82aa9c56b335d855c5b55c0b18011b6610b78f01010000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff1f022a680456c5b55c085ffffff2c41400000d2f6e6f64655374726174756d2f00000000030000000000000000266a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf980010b27010000001976a914ac2eb3e500b3670964ef81c2acbe276f8083ecfd88ac80f0fa02000000001976a914a09b7a0ea8e3f56bb71f1af38406a12ffc048fe988ac0120000000000000000000000000000000000000000000000000000000000000000000000000"), SER_NETWORK, PROTOCOL_VERSION);
    stream >> block;
    return block;
}
