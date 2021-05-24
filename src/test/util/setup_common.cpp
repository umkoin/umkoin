// Copyright (c) 2011-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/util/setup_common.h>

#include <addrman.h>
#include <banman.h>
#include <chainparams.h>
#include <consensus/consensus.h>
#include <consensus/params.h>
#include <consensus/validation.h>
#include <crypto/sha256.h>
#include <init.h>
#include <interfaces/chain.h>
#include <miner.h>
#include <net.h>
#include <net_processing.h>
#include <noui.h>
#include <policy/fees.h>
#include <pow.h>
#include <rpc/blockchain.h>
#include <rpc/register.h>
#include <rpc/server.h>
#include <scheduler.h>
#include <script/sigcache.h>
#include <streams.h>
#include <txdb.h>
#include <util/strencodings.h>
#include <util/string.h>
#include <util/thread.h>
#include <util/threadnames.h>
#include <util/time.h>
#include <util/translation.h>
#include <util/url.h>
#include <util/vector.h>
#include <validation.h>
#include <validationinterface.h>
#include <walletinitinterface.h>

#include <functional>

const std::function<std::string(const char*)> G_TRANSLATION_FUN = nullptr;
UrlDecodeFn* const URL_DECODE = nullptr;

FastRandomContext g_insecure_rand_ctx;
/** Random context to get unique temp data dirs. Separate from g_insecure_rand_ctx, which can be seeded from a const env var */
static FastRandomContext g_insecure_rand_ctx_temp_path;

/** Return the unsigned from the environment var if available, otherwise 0 */
static uint256 GetUintFromEnv(const std::string& env_name)
{
    const char* num = std::getenv(env_name.c_str());
    if (!num) return {};
    return uint256S(num);
}

void Seed(FastRandomContext& ctx)
{
    // Should be enough to get the seed once for the process
    static uint256 seed{};
    static const std::string RANDOM_CTX_SEED{"RANDOM_CTX_SEED"};
    if (seed.IsNull()) seed = GetUintFromEnv(RANDOM_CTX_SEED);
    if (seed.IsNull()) seed = GetRandHash();
    LogPrintf("%s: Setting random seed for current tests to %s=%s\n", __func__, RANDOM_CTX_SEED, seed.GetHex());
    ctx = FastRandomContext(seed);
}

std::ostream& operator<<(std::ostream& os, const uint256& num)
{
    os << num.ToString();
    return os;
}

BasicTestingSetup::BasicTestingSetup(const std::string& chainName, const std::vector<const char*>& extra_args)
    : m_path_root{fs::temp_directory_path() / "test_common_" PACKAGE_NAME / g_insecure_rand_ctx_temp_path.rand256().ToString()},
      m_args{}
{
    const std::vector<const char*> arguments = Cat(
        {
            "dummy",
            "-printtoconsole=0",
            "-logsourcelocations",
            "-logtimemicros",
            "-logthreadnames",
            "-debug",
            "-debugexclude=libevent",
            "-debugexclude=leveldb",
        },
        extra_args);
    util::ThreadRename("test");
    fs::create_directories(m_path_root);
    m_args.ForceSetArg("-datadir", m_path_root.string());
    gArgs.ForceSetArg("-datadir", m_path_root.string());
    gArgs.ClearPathCache();
    {
        SetupServerArgs(m_node);
        std::string error;
        const bool success{m_node.args->ParseParameters(arguments.size(), arguments.data(), error)};
        assert(success);
        assert(error.empty());
    }
    SelectParams(chainName);
    SeedInsecureRand();
    if (G_TEST_LOG_FUN) LogInstance().PushBackCallback(G_TEST_LOG_FUN);
    InitLogging(*m_node.args);
    AppInitParameterInteraction(*m_node.args);
    LogInstance().StartLogging();
    SHA256AutoDetect();
    ECC_Start();
    SetupEnvironment();
    SetupNetworking();
    InitSignatureCache();
    InitScriptExecutionCache();
    m_node.chain = interfaces::MakeChain(m_node);
    g_wallet_init_interface.Construct(m_node);
    fCheckBlockIndex = true;
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
    ECC_Stop();
}

ChainTestingSetup::ChainTestingSetup(const std::string& chainName, const std::vector<const char*>& extra_args)
    : BasicTestingSetup(chainName, extra_args)
{
    // We have to run a scheduler thread to prevent ActivateBestChain
    // from blocking due to queue overrun.
    m_node.scheduler = std::make_unique<CScheduler>();
    m_node.scheduler->m_service_thread = std::thread(util::TraceThread, "scheduler", [&] { m_node.scheduler->serviceQueue(); });
    GetMainSignals().RegisterBackgroundSignalScheduler(*m_node.scheduler);

    pblocktree.reset(new CBlockTreeDB(1 << 20, true));

    m_node.fee_estimator = std::make_unique<CBlockPolicyEstimator>();
    m_node.mempool = std::make_unique<CTxMemPool>(m_node.fee_estimator.get(), 1);

    m_node.chainman = &::g_chainman;

    // Start script-checking threads. Set g_parallel_script_checks to true so they are used.
    constexpr int script_check_threads = 2;
    StartScriptCheckWorkerThreads(script_check_threads);
    g_parallel_script_checks = true;
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
    m_node.args = nullptr;
    UnloadBlockIndex(m_node.mempool.get(), *m_node.chainman);
    m_node.mempool.reset();
    m_node.scheduler.reset();
    m_node.chainman->Reset();
    m_node.chainman = nullptr;
    pblocktree.reset();
}

TestingSetup::TestingSetup(const std::string& chainName, const std::vector<const char*>& extra_args)
    : ChainTestingSetup(chainName, extra_args)
{
    const CChainParams& chainparams = Params();
    // Ideally we'd move all the RPC tests to the functional testing framework
    // instead of unit tests, but for now we need these here.
    RegisterAllCoreRPCCommands(tableRPC);

    m_node.chainman->InitializeChainstate(*m_node.mempool);
    ::ChainstateActive().InitCoinsDB(
        /* cache_size_bytes */ 1 << 23, /* in_memory */ true, /* should_wipe */ false);
    assert(!::ChainstateActive().CanFlushToDisk());
    ::ChainstateActive().InitCoinsCache(1 << 23);
    assert(::ChainstateActive().CanFlushToDisk());
    if (!::ChainstateActive().LoadGenesisBlock(chainparams)) {
        throw std::runtime_error("LoadGenesisBlock failed.");
    }

    BlockValidationState state;
    if (!::ChainstateActive().ActivateBestChain(state, chainparams)) {
        throw std::runtime_error(strprintf("ActivateBestChain failed. (%s)", state.ToString()));
    }

    m_node.addrman = std::make_unique<CAddrMan>();
    m_node.banman = std::make_unique<BanMan>(m_args.GetDataDirBase() / "banlist.dat", nullptr, DEFAULT_MISBEHAVING_BANTIME);
    m_node.connman = std::make_unique<CConnman>(0x1337, 0x1337, *m_node.addrman); // Deterministic randomness for tests.
    m_node.peerman = PeerManager::make(chainparams, *m_node.connman, *m_node.addrman,
                                       m_node.banman.get(), *m_node.scheduler, *m_node.chainman,
                                       *m_node.mempool, false);
    {
        CConnman::Options options;
        options.m_msgproc = m_node.peerman.get();
        m_node.connman->Init(options);
    }
}

TestChain100Setup::TestChain100Setup()
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

CBlock TestChain100Setup::CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns, const CScript& scriptPubKey)
{
    const CChainParams& chainparams = Params();
    CTxMemPool empty_pool;
    CBlock block = BlockAssembler(::ChainstateActive(), empty_pool, chainparams).CreateNewBlock(scriptPubKey)->block;

    Assert(block.vtx.size() == 1);
    for (const CMutableTransaction& tx : txns) {
        block.vtx.push_back(MakeTransactionRef(tx));
    }
    CBlockIndex* prev_block = WITH_LOCK(::cs_main, return g_chainman.m_blockman.LookupBlockIndex(block.hashPrevBlock));
    RegenerateCommitments(block, prev_block);

    while (!CheckProofOfWork(block.GetHash(), block.nBits, chainparams.GetConsensus())) ++block.nNonce;

    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    Assert(m_node.chainman)->ProcessNewBlock(chainparams, shared_pblock, true, nullptr);

    return block;
}


CMutableTransaction TestChain100Setup::CreateValidMempoolTransaction(CTransactionRef input_transaction,
                                                                     int input_vout,
                                                                     int input_height,
                                                                     CKey input_signing_key,
                                                                     CScript output_destination,
                                                                     CAmount output_amount)
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
    std::map<int, std::string> input_errors;
    assert(SignTransaction(mempool_txn, &keystore, input_coins, nHashType, input_errors));

    // Add transaction to the mempool
    {
        LOCK(cs_main);
        const MempoolAcceptResult result = AcceptToMemoryPool(::ChainstateActive(), *m_node.mempool.get(), MakeTransactionRef(mempool_txn), /* bypass_limits */ false);
        assert(result.m_result_type == MempoolAcceptResult::ResultType::VALID);
    }

    return mempool_txn;
}

TestChain100Setup::~TestChain100Setup()
{
    gArgs.ForceSetArg("-segwitheight", "0");
}

CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CMutableTransaction& tx) const
{
    return FromTx(MakeTransactionRef(tx));
}

CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CTransactionRef& tx) const
{
    return CTxMemPoolEntry(tx, nFee, nTime, nHeight,
                           spendsCoinbase, sigOpCost, lp);
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
