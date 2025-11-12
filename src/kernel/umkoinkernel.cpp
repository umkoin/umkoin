// Copyright (c) 2022-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define UMKOINKERNEL_BUILD

#include <kernel/umkoinkernel.h>

#include <chain.h>
#include <coins.h>
#include <consensus/amount.h>
#include <consensus/validation.h>
#include <kernel/caches.h>
#include <kernel/chainparams.h>
#include <kernel/checks.h>
#include <kernel/context.h>
#include <kernel/cs_main.h>
#include <kernel/notifications_interface.h>
#include <kernel/warning.h>
#include <logging.h>
#include <node/blockstorage.h>
#include <node/chainstate.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <serialize.h>
#include <streams.h>
#include <sync.h>
#include <tinyformat.h>
#include <uint256.h>
#include <undo.h>
#include <util/fs.h>
#include <util/result.h>
#include <util/signalinterrupt.h>
#include <util/task_runner.h>
#include <util/translation.h>
#include <validation.h>
#include <validationinterface.h>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using util::ImmediateTaskRunner;

// Define G_TRANSLATION_FUN symbol in libumkoinkernel library so users of the
// library aren't required to export this symbol
extern const std::function<std::string(const char*)> G_TRANSLATION_FUN{nullptr};

static const kernel::Context umkk_context_static{};

namespace {

bool is_valid_flag_combination(script_verify_flags flags)
{
    if (flags & SCRIPT_VERIFY_CLEANSTACK && ~flags & (SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_WITNESS)) return false;
    if (flags & SCRIPT_VERIFY_WITNESS && ~flags & SCRIPT_VERIFY_P2SH) return false;
    return true;
}

class WriterStream
{
private:
    umkk_WriteBytes m_writer;
    void* m_user_data;

public:
    WriterStream(umkk_WriteBytes writer, void* user_data)
        : m_writer{writer}, m_user_data{user_data} {}

    //
    // Stream subset
    //
    void write(std::span<const std::byte> src)
    {
        if (m_writer(std::data(src), src.size(), m_user_data) != 0) {
            throw std::runtime_error("Failed to write serialization data");
        }
    }

    template <typename T>
    WriterStream& operator<<(const T& obj)
    {
        ::Serialize(*this, obj);
        return *this;
    }
};

template <typename C, typename CPP>
struct Handle {
    static C* ref(CPP* cpp_type)
    {
        return reinterpret_cast<C*>(cpp_type);
    }

    static const C* ref(const CPP* cpp_type)
    {
        return reinterpret_cast<const C*>(cpp_type);
    }

    template <typename... Args>
    static C* create(Args&&... args)
    {
        auto cpp_obj{std::make_unique<CPP>(std::forward<Args>(args)...)};
        return reinterpret_cast<C*>(cpp_obj.release());
    }

    static C* copy(const C* ptr)
    {
        auto cpp_obj{std::make_unique<CPP>(get(ptr))};
        return reinterpret_cast<C*>(cpp_obj.release());
    }

    static const CPP& get(const C* ptr)
    {
        return *reinterpret_cast<const CPP*>(ptr);
    }

    static CPP& get(C* ptr)
    {
        return *reinterpret_cast<CPP*>(ptr);
    }

    static void operator delete(void* ptr)
    {
        delete reinterpret_cast<CPP*>(ptr);
    }
};

} // namespace

struct umkk_BlockTreeEntry: Handle<umkk_BlockTreeEntry, CBlockIndex> {};
struct umkk_Block : Handle<umkk_Block, std::shared_ptr<const CBlock>> {};
struct umkk_BlockValidationState : Handle<umkk_BlockValidationState, BlockValidationState> {};

namespace {

BCLog::Level get_bclog_level(umkk_LogLevel level)
{
    switch (level) {
    case umkk_LogLevel_INFO: {
        return BCLog::Level::Info;
    }
    case umkk_LogLevel_DEBUG: {
        return BCLog::Level::Debug;
    }
    case umkk_LogLevel_TRACE: {
        return BCLog::Level::Trace;
    }
    }
    assert(false);
}

BCLog::LogFlags get_bclog_flag(umkk_LogCategory category)
{
    switch (category) {
    case umkk_LogCategory_BENCH: {
        return BCLog::LogFlags::BENCH;
    }
    case umkk_LogCategory_BLOCKSTORAGE: {
        return BCLog::LogFlags::BLOCKSTORAGE;
    }
    case umkk_LogCategory_COINDB: {
        return BCLog::LogFlags::COINDB;
    }
    case umkk_LogCategory_LEVELDB: {
        return BCLog::LogFlags::LEVELDB;
    }
    case umkk_LogCategory_MEMPOOL: {
        return BCLog::LogFlags::MEMPOOL;
    }
    case umkk_LogCategory_PRUNE: {
        return BCLog::LogFlags::PRUNE;
    }
    case umkk_LogCategory_RAND: {
        return BCLog::LogFlags::RAND;
    }
    case umkk_LogCategory_REINDEX: {
        return BCLog::LogFlags::REINDEX;
    }
    case umkk_LogCategory_VALIDATION: {
        return BCLog::LogFlags::VALIDATION;
    }
    case umkk_LogCategory_KERNEL: {
        return BCLog::LogFlags::KERNEL;
    }
    case umkk_LogCategory_ALL: {
        return BCLog::LogFlags::ALL;
    }
    }
    assert(false);
}

umkk_SynchronizationState cast_state(SynchronizationState state)
{
    switch (state) {
    case SynchronizationState::INIT_REINDEX:
        return umkk_SynchronizationState_INIT_REINDEX;
    case SynchronizationState::INIT_DOWNLOAD:
        return umkk_SynchronizationState_INIT_DOWNLOAD;
    case SynchronizationState::POST_INIT:
        return umkk_SynchronizationState_POST_INIT;
    } // no default case, so the compiler can warn about missing cases
    assert(false);
}

umkk_Warning cast_umkk_warning(kernel::Warning warning)
{
    switch (warning) {
    case kernel::Warning::UNKNOWN_NEW_RULES_ACTIVATED:
        return umkk_Warning_UNKNOWN_NEW_RULES_ACTIVATED;
    case kernel::Warning::LARGE_WORK_INVALID_CHAIN:
        return umkk_Warning_LARGE_WORK_INVALID_CHAIN;
    } // no default case, so the compiler can warn about missing cases
    assert(false);
}

struct LoggingConnection {
    std::unique_ptr<std::list<std::function<void(const std::string&)>>::iterator> m_connection;
    void* m_user_data;
    std::function<void(void* user_data)> m_deleter;

    LoggingConnection(umkk_LogCallback callback, void* user_data, umkk_DestroyCallback user_data_destroy_callback)
    {
        LOCK(cs_main);

        auto connection{LogInstance().PushBackCallback([callback, user_data](const std::string& str) { callback(user_data, str.c_str(), str.length()); })};

        // Only start logging if we just added the connection.
        if (LogInstance().NumConnections() == 1 && !LogInstance().StartLogging()) {
            LogError("Logger start failed.");
            LogInstance().DeleteCallback(connection);
            if (user_data && user_data_destroy_callback) {
                user_data_destroy_callback(user_data);
            }
            throw std::runtime_error("Failed to start logging");
        }

        m_connection = std::make_unique<std::list<std::function<void(const std::string&)>>::iterator>(connection);
        m_user_data = user_data;
        m_deleter = user_data_destroy_callback;

        LogDebug(BCLog::KERNEL, "Logger connected.");
    }

    ~LoggingConnection()
    {
        LOCK(cs_main);
        LogDebug(BCLog::KERNEL, "Logger disconnecting.");

        // Switch back to buffering by calling DisconnectTestLogger if the
        // connection that we are about to remove is the last one.
        if (LogInstance().NumConnections() == 1) {
            LogInstance().DisconnectTestLogger();
        } else {
            LogInstance().DeleteCallback(*m_connection);
        }

        m_connection.reset();
        if (m_user_data && m_deleter) {
            m_deleter(m_user_data);
        }
    }
};

class KernelNotifications final : public kernel::Notifications
{
private:
    umkk_NotificationInterfaceCallbacks m_cbs;

public:
    KernelNotifications(umkk_NotificationInterfaceCallbacks cbs)
        : m_cbs{cbs}
    {
    }

    ~KernelNotifications()
    {
        if (m_cbs.user_data && m_cbs.user_data_destroy) {
            m_cbs.user_data_destroy(m_cbs.user_data);
        }
        m_cbs.user_data_destroy = nullptr;
        m_cbs.user_data = nullptr;
    }

    kernel::InterruptResult blockTip(SynchronizationState state, const CBlockIndex& index, double verification_progress) override
    {
        if (m_cbs.block_tip) m_cbs.block_tip(m_cbs.user_data, cast_state(state), umkk_BlockTreeEntry::ref(&index), verification_progress);
        return {};
    }
    void headerTip(SynchronizationState state, int64_t height, int64_t timestamp, bool presync) override
    {
        if (m_cbs.header_tip) m_cbs.header_tip(m_cbs.user_data, cast_state(state), height, timestamp, presync ? 1 : 0);
    }
    void progress(const bilingual_str& title, int progress_percent, bool resume_possible) override
    {
        if (m_cbs.progress) m_cbs.progress(m_cbs.user_data, title.original.c_str(), title.original.length(), progress_percent, resume_possible ? 1 : 0);
    }
    void warningSet(kernel::Warning id, const bilingual_str& message) override
    {
        if (m_cbs.warning_set) m_cbs.warning_set(m_cbs.user_data, cast_umkk_warning(id), message.original.c_str(), message.original.length());
    }
    void warningUnset(kernel::Warning id) override
    {
        if (m_cbs.warning_unset) m_cbs.warning_unset(m_cbs.user_data, cast_umkk_warning(id));
    }
    void flushError(const bilingual_str& message) override
    {
        if (m_cbs.flush_error) m_cbs.flush_error(m_cbs.user_data, message.original.c_str(), message.original.length());
    }
    void fatalError(const bilingual_str& message) override
    {
        if (m_cbs.fatal_error) m_cbs.fatal_error(m_cbs.user_data, message.original.c_str(), message.original.length());
    }
};

class KernelValidationInterface final : public CValidationInterface
{
public:
    umkk_ValidationInterfaceCallbacks m_cbs;

    explicit KernelValidationInterface(const umkk_ValidationInterfaceCallbacks vi_cbs) : m_cbs{vi_cbs} {}

    ~KernelValidationInterface()
    {
        if (m_cbs.user_data && m_cbs.user_data_destroy) {
            m_cbs.user_data_destroy(m_cbs.user_data);
        }
        m_cbs.user_data = nullptr;
        m_cbs.user_data_destroy = nullptr;
    }

protected:
    void BlockChecked(const std::shared_ptr<const CBlock>& block, const BlockValidationState& stateIn) override
    {
        if (m_cbs.block_checked) {
            m_cbs.block_checked(m_cbs.user_data,
                                umkk_Block::copy(umkk_Block::ref(&block)),
                                umkk_BlockValidationState::ref(&stateIn));
        }
    }

    void NewPoWValidBlock(const CBlockIndex* pindex, const std::shared_ptr<const CBlock>& block) override
    {
        if (m_cbs.pow_valid_block) {
            m_cbs.pow_valid_block(m_cbs.user_data,
                                  umkk_Block::copy(umkk_Block::ref(&block)),
                                  umkk_BlockTreeEntry::ref(pindex));
        }
    }

    void BlockConnected(ChainstateRole role, const std::shared_ptr<const CBlock>& block, const CBlockIndex* pindex) override
    {
        if (m_cbs.block_connected) {
            m_cbs.block_connected(m_cbs.user_data,
                                  umkk_Block::copy(umkk_Block::ref(&block)),
                                  umkk_BlockTreeEntry::ref(pindex));
        }
    }

    void BlockDisconnected(const std::shared_ptr<const CBlock>& block, const CBlockIndex* pindex) override
    {
        if (m_cbs.block_disconnected) {
            m_cbs.block_disconnected(m_cbs.user_data,
                                     umkk_Block::copy(umkk_Block::ref(&block)),
                                     umkk_BlockTreeEntry::ref(pindex));
        }
    }
};

struct ContextOptions {
    mutable Mutex m_mutex;
    std::unique_ptr<const CChainParams> m_chainparams GUARDED_BY(m_mutex);
    std::shared_ptr<KernelNotifications> m_notifications GUARDED_BY(m_mutex);
    std::shared_ptr<KernelValidationInterface> m_validation_interface GUARDED_BY(m_mutex);
};

class Context
{
public:
    std::unique_ptr<kernel::Context> m_context;

    std::shared_ptr<KernelNotifications> m_notifications;

    std::unique_ptr<util::SignalInterrupt> m_interrupt;

    std::unique_ptr<ValidationSignals> m_signals;

    std::unique_ptr<const CChainParams> m_chainparams;

    std::shared_ptr<KernelValidationInterface> m_validation_interface;

    Context(const ContextOptions* options, bool& sane)
        : m_context{std::make_unique<kernel::Context>()},
          m_interrupt{std::make_unique<util::SignalInterrupt>()}
    {
        if (options) {
            LOCK(options->m_mutex);
            if (options->m_chainparams) {
                m_chainparams = std::make_unique<const CChainParams>(*options->m_chainparams);
            }
            if (options->m_notifications) {
                m_notifications = options->m_notifications;
            }
            if (options->m_validation_interface) {
                m_signals = std::make_unique<ValidationSignals>(std::make_unique<ImmediateTaskRunner>());
                m_validation_interface = options->m_validation_interface;
                m_signals->RegisterSharedValidationInterface(m_validation_interface);
            }
        }

        if (!m_chainparams) {
            m_chainparams = CChainParams::Main();
        }
        if (!m_notifications) {
            m_notifications = std::make_shared<KernelNotifications>(umkk_NotificationInterfaceCallbacks{
                nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr});
        }

        if (!kernel::SanityChecks(*m_context)) {
            sane = false;
        }
    }

    ~Context()
    {
        if (m_signals) {
            m_signals->UnregisterSharedValidationInterface(m_validation_interface);
        }
    }
};

//! Helper struct to wrap the ChainstateManager-related Options
struct ChainstateManagerOptions {
    mutable Mutex m_mutex;
    ChainstateManager::Options m_chainman_options GUARDED_BY(m_mutex);
    node::BlockManager::Options m_blockman_options GUARDED_BY(m_mutex);
    std::shared_ptr<const Context> m_context;
    node::ChainstateLoadOptions m_chainstate_load_options GUARDED_BY(m_mutex);

    ChainstateManagerOptions(const std::shared_ptr<const Context>& context, const fs::path& data_dir, const fs::path& blocks_dir)
        : m_chainman_options{ChainstateManager::Options{
              .chainparams = *context->m_chainparams,
              .datadir = data_dir,
              .notifications = *context->m_notifications,
              .signals = context->m_signals.get()}},
          m_blockman_options{node::BlockManager::Options{
              .chainparams = *context->m_chainparams,
              .blocks_dir = blocks_dir,
              .notifications = *context->m_notifications,
              .block_tree_db_params = DBParams{
                  .path = data_dir / "blocks" / "index",
                  .cache_bytes = kernel::CacheSizes{DEFAULT_KERNEL_CACHE}.block_tree_db,
              }}},
          m_context{context}, m_chainstate_load_options{node::ChainstateLoadOptions{}}
    {
    }
};

struct ChainMan {
    std::unique_ptr<ChainstateManager> m_chainman;
    std::shared_ptr<const Context> m_context;

    ChainMan(std::unique_ptr<ChainstateManager> chainman, std::shared_ptr<const Context> context)
        : m_chainman(std::move(chainman)), m_context(std::move(context)) {}
};

} // namespace

struct umkk_Transaction : Handle<umkk_Transaction, std::shared_ptr<const CTransaction>> {};
struct umkk_TransactionOutput : Handle<umkk_TransactionOutput, CTxOut> {};
struct umkk_ScriptPubkey : Handle<umkk_ScriptPubkey, CScript> {};
struct umkk_LoggingConnection : Handle<umkk_LoggingConnection, LoggingConnection> {};
struct umkk_ContextOptions : Handle<umkk_ContextOptions, ContextOptions> {};
struct umkk_Context : Handle<umkk_Context, std::shared_ptr<const Context>> {};
struct umkk_ChainParameters : Handle<umkk_ChainParameters, CChainParams> {};
struct umkk_ChainstateManagerOptions : Handle<umkk_ChainstateManagerOptions, ChainstateManagerOptions> {};
struct umkk_ChainstateManager : Handle<umkk_ChainstateManager, ChainMan> {};
struct umkk_Chain : Handle<umkk_Chain, CChain> {};
struct umkk_BlockSpentOutputs : Handle<umkk_BlockSpentOutputs, std::shared_ptr<CBlockUndo>> {};
struct umkk_TransactionSpentOutputs : Handle<umkk_TransactionSpentOutputs, CTxUndo> {};
struct umkk_Coin : Handle<umkk_Coin, Coin> {};
struct umkk_BlockHash : Handle<umkk_BlockHash, uint256> {};
struct umkk_TransactionInput : Handle<umkk_TransactionInput, CTxIn> {};
struct umkk_TransactionOutPoint: Handle<umkk_TransactionOutPoint, COutPoint> {};
struct umkk_Txid: Handle<umkk_Txid, Txid> {};

umkk_Transaction* umkk_transaction_create(const void* raw_transaction, size_t raw_transaction_len)
{
    if (raw_transaction == nullptr && raw_transaction_len != 0) {
        return nullptr;
    }
    try {
        DataStream stream{std::span{reinterpret_cast<const std::byte*>(raw_transaction), raw_transaction_len}};
        return umkk_Transaction::create(std::make_shared<const CTransaction>(deserialize, TX_WITH_WITNESS, stream));
    } catch (...) {
        return nullptr;
    }
}

size_t umkk_transaction_count_outputs(const umkk_Transaction* transaction)
{
    return umkk_Transaction::get(transaction)->vout.size();
}

const umkk_TransactionOutput* umkk_transaction_get_output_at(const umkk_Transaction* transaction, size_t output_index)
{
    const CTransaction& tx = *umkk_Transaction::get(transaction);
    assert(output_index < tx.vout.size());
    return umkk_TransactionOutput::ref(&tx.vout[output_index]);
}

size_t umkk_transaction_count_inputs(const umkk_Transaction* transaction)
{
    return umkk_Transaction::get(transaction)->vin.size();
}

const umkk_TransactionInput* umkk_transaction_get_input_at(const umkk_Transaction* transaction, size_t input_index)
{
    assert(input_index < umkk_Transaction::get(transaction)->vin.size());
    return umkk_TransactionInput::ref(&umkk_Transaction::get(transaction)->vin[input_index]);
}

const umkk_Txid* umkk_transaction_get_txid(const umkk_Transaction* transaction)
{
    return umkk_Txid::ref(&umkk_Transaction::get(transaction)->GetHash());
}

umkk_Transaction* umkk_transaction_copy(const umkk_Transaction* transaction)
{
    return umkk_Transaction::copy(transaction);
}

int umkk_transaction_to_bytes(const umkk_Transaction* transaction, umkk_WriteBytes writer, void* user_data)
{
    try {
        WriterStream ws{writer, user_data};
        ws << TX_WITH_WITNESS(umkk_Transaction::get(transaction));
        return 0;
    } catch (...) {
        return -1;
    }
}

void umkk_transaction_destroy(umkk_Transaction* transaction)
{
    delete transaction;
}

umkk_ScriptPubkey* umkk_script_pubkey_create(const void* script_pubkey, size_t script_pubkey_len)
{
    if (script_pubkey == nullptr && script_pubkey_len != 0) {
        return nullptr;
    }
    auto data = std::span{reinterpret_cast<const uint8_t*>(script_pubkey), script_pubkey_len};
    return umkk_ScriptPubkey::create(data.begin(), data.end());
}

int umkk_script_pubkey_to_bytes(const umkk_ScriptPubkey* script_pubkey_, umkk_WriteBytes writer, void* user_data)
{
    const auto& script_pubkey{umkk_ScriptPubkey::get(script_pubkey_)};
    return writer(script_pubkey.data(), script_pubkey.size(), user_data);
}

umkk_ScriptPubkey* umkk_script_pubkey_copy(const umkk_ScriptPubkey* script_pubkey)
{
    return umkk_ScriptPubkey::copy(script_pubkey);
}

void umkk_script_pubkey_destroy(umkk_ScriptPubkey* script_pubkey)
{
    delete script_pubkey;
}

umkk_TransactionOutput* umkk_transaction_output_create(const umkk_ScriptPubkey* script_pubkey, int64_t amount)
{
    return umkk_TransactionOutput::create(amount, umkk_ScriptPubkey::get(script_pubkey));
}

umkk_TransactionOutput* umkk_transaction_output_copy(const umkk_TransactionOutput* output)
{
    return umkk_TransactionOutput::copy(output);
}

const umkk_ScriptPubkey* umkk_transaction_output_get_script_pubkey(const umkk_TransactionOutput* output)
{
    return umkk_ScriptPubkey::ref(&umkk_TransactionOutput::get(output).scriptPubKey);
}

int64_t umkk_transaction_output_get_amount(const umkk_TransactionOutput* output)
{
    return umkk_TransactionOutput::get(output).nValue;
}

void umkk_transaction_output_destroy(umkk_TransactionOutput* output)
{
    delete output;
}

int umkk_script_pubkey_verify(const umkk_ScriptPubkey* script_pubkey,
                              const int64_t amount,
                              const umkk_Transaction* tx_to,
                              const umkk_TransactionOutput** spent_outputs_, size_t spent_outputs_len,
                              const unsigned int input_index,
                              const umkk_ScriptVerificationFlags flags,
                              umkk_ScriptVerifyStatus* status)
{
    // Assert that all specified flags are part of the interface before continuing
    assert((flags & ~umkk_ScriptVerificationFlags_ALL) == 0);

    if (!is_valid_flag_combination(script_verify_flags::from_int(flags))) {
        if (status) *status = umkk_ScriptVerifyStatus_ERROR_INVALID_FLAGS_COMBINATION;
        return 0;
    }

    if (flags & umkk_ScriptVerificationFlags_TAPROOT && spent_outputs_ == nullptr) {
        if (status) *status = umkk_ScriptVerifyStatus_ERROR_SPENT_OUTPUTS_REQUIRED;
        return 0;
    }

    if (status) *status = umkk_ScriptVerifyStatus_OK;

    const CTransaction& tx{*umkk_Transaction::get(tx_to)};
    std::vector<CTxOut> spent_outputs;
    if (spent_outputs_ != nullptr) {
        assert(spent_outputs_len == tx.vin.size());
        spent_outputs.reserve(spent_outputs_len);
        for (size_t i = 0; i < spent_outputs_len; i++) {
            const CTxOut& tx_out{umkk_TransactionOutput::get(spent_outputs_[i])};
            spent_outputs.push_back(tx_out);
        }
    }

    assert(input_index < tx.vin.size());
    PrecomputedTransactionData txdata{tx};

    if (spent_outputs_ != nullptr && flags & umkk_ScriptVerificationFlags_TAPROOT) {
        txdata.Init(tx, std::move(spent_outputs));
    }

    bool result = VerifyScript(tx.vin[input_index].scriptSig,
                               umkk_ScriptPubkey::get(script_pubkey),
                               &tx.vin[input_index].scriptWitness,
                               script_verify_flags::from_int(flags),
                               TransactionSignatureChecker(&tx, input_index, amount, txdata, MissingDataBehavior::FAIL),
                               nullptr);
    return result ? 1 : 0;
}

umkk_TransactionInput* umkk_transaction_input_copy(const umkk_TransactionInput* input)
{
    return umkk_TransactionInput::copy(input);
}

const umkk_TransactionOutPoint* umkk_transaction_input_get_out_point(const umkk_TransactionInput* input)
{
    return umkk_TransactionOutPoint::ref(&umkk_TransactionInput::get(input).prevout);
}

void umkk_transaction_input_destroy(umkk_TransactionInput* input)
{
    delete input;
}

umkk_TransactionOutPoint* umkk_transaction_out_point_copy(const umkk_TransactionOutPoint* out_point)
{
    return umkk_TransactionOutPoint::copy(out_point);
}

uint32_t umkk_transaction_out_point_get_index(const umkk_TransactionOutPoint* out_point)
{
    return umkk_TransactionOutPoint::get(out_point).n;
}

const umkk_Txid* umkk_transaction_out_point_get_txid(const umkk_TransactionOutPoint* out_point)
{
    return umkk_Txid::ref(&umkk_TransactionOutPoint::get(out_point).hash);
}

void umkk_transaction_out_point_destroy(umkk_TransactionOutPoint* out_point)
{
    delete out_point;
}

umkk_Txid* umkk_txid_copy(const umkk_Txid* txid)
{
    return umkk_Txid::copy(txid);
}

void umkk_txid_to_bytes(const umkk_Txid* txid, unsigned char output[32])
{
    std::memcpy(output, umkk_Txid::get(txid).begin(), 32);
}

int umkk_txid_equals(const umkk_Txid* txid1, const umkk_Txid* txid2)
{
    return umkk_Txid::get(txid1) == umkk_Txid::get(txid2);
}

void umkk_txid_destroy(umkk_Txid* txid)
{
    delete txid;
}

void umkk_logging_set_options(const umkk_LoggingOptions options)
{
    LOCK(cs_main);
    LogInstance().m_log_timestamps = options.log_timestamps;
    LogInstance().m_log_time_micros = options.log_time_micros;
    LogInstance().m_log_threadnames = options.log_threadnames;
    LogInstance().m_log_sourcelocations = options.log_sourcelocations;
    LogInstance().m_always_print_category_level = options.always_print_category_levels;
}

void umkk_logging_set_level_category(umkk_LogCategory category, umkk_LogLevel level)
{
    LOCK(cs_main);
    if (category == umkk_LogCategory_ALL) {
        LogInstance().SetLogLevel(get_bclog_level(level));
    }

    LogInstance().AddCategoryLogLevel(get_bclog_flag(category), get_bclog_level(level));
}

void umkk_logging_enable_category(umkk_LogCategory category)
{
    LogInstance().EnableCategory(get_bclog_flag(category));
}

void umkk_logging_disable_category(umkk_LogCategory category)
{
    LogInstance().DisableCategory(get_bclog_flag(category));
}

void umkk_logging_disable()
{
    LogInstance().DisableLogging();
}

umkk_LoggingConnection* umkk_logging_connection_create(umkk_LogCallback callback, void* user_data, umkk_DestroyCallback user_data_destroy_callback)
{
    try {
        return umkk_LoggingConnection::create(callback, user_data, user_data_destroy_callback);
    } catch (const std::exception&) {
        return nullptr;
    }
}

void umkk_logging_connection_destroy(umkk_LoggingConnection* connection)
{
    delete connection;
}

umkk_ChainParameters* umkk_chain_parameters_create(const umkk_ChainType chain_type)
{
    switch (chain_type) {
    case umkk_ChainType_MAINNET: {
        return umkk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::Main().release()));
    }
    case umkk_ChainType_TESTNET: {
        return umkk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::TestNet().release()));
    }
    case umkk_ChainType_TESTNET_4: {
        return umkk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::TestNet4().release()));
    }
    case umkk_ChainType_SIGNET: {
        return umkk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::SigNet({}).release()));
    }
    case umkk_ChainType_REGTEST: {
        return umkk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::RegTest({}).release()));
    }
    }
    assert(false);
}

umkk_ChainParameters* umkk_chain_parameters_copy(const umkk_ChainParameters* chain_parameters)
{
    return umkk_ChainParameters::copy(chain_parameters);
}

void umkk_chain_parameters_destroy(umkk_ChainParameters* chain_parameters)
{
    delete chain_parameters;
}

umkk_ContextOptions* umkk_context_options_create()
{
    return umkk_ContextOptions::create();
}

void umkk_context_options_set_chainparams(umkk_ContextOptions* options, const umkk_ChainParameters* chain_parameters)
{
    // Copy the chainparams, so the caller can free it again
    LOCK(umkk_ContextOptions::get(options).m_mutex);
    umkk_ContextOptions::get(options).m_chainparams = std::make_unique<const CChainParams>(umkk_ChainParameters::get(chain_parameters));
}

void umkk_context_options_set_notifications(umkk_ContextOptions* options, umkk_NotificationInterfaceCallbacks notifications)
{
    // The KernelNotifications are copy-initialized, so the caller can free them again.
    LOCK(umkk_ContextOptions::get(options).m_mutex);
    umkk_ContextOptions::get(options).m_notifications = std::make_shared<KernelNotifications>(notifications);
}

void umkk_context_options_set_validation_interface(umkk_ContextOptions* options, umkk_ValidationInterfaceCallbacks vi_cbs)
{
    LOCK(umkk_ContextOptions::get(options).m_mutex);
    umkk_ContextOptions::get(options).m_validation_interface = std::make_shared<KernelValidationInterface>(vi_cbs);
}

void umkk_context_options_destroy(umkk_ContextOptions* options)
{
    delete options;
}

umkk_Context* umkk_context_create(const umkk_ContextOptions* options)
{
    bool sane{true};
    const ContextOptions* opts = options ? &umkk_ContextOptions::get(options) : nullptr;
    auto context{std::make_shared<const Context>(opts, sane)};
    if (!sane) {
        LogError("Kernel context sanity check failed.");
        return nullptr;
    }
    return umkk_Context::create(context);
}

umkk_Context* umkk_context_copy(const umkk_Context* context)
{
    return umkk_Context::copy(context);
}

int umkk_context_interrupt(umkk_Context* context)
{
    return (*umkk_Context::get(context)->m_interrupt)() ? 0 : -1;
}

void umkk_context_destroy(umkk_Context* context)
{
    delete context;
}

const umkk_BlockTreeEntry* umkk_block_tree_entry_get_previous(const umkk_BlockTreeEntry* entry)
{
    if (!umkk_BlockTreeEntry::get(entry).pprev) {
        LogInfo("Genesis block has no previous.");
        return nullptr;
    }

    return umkk_BlockTreeEntry::ref(umkk_BlockTreeEntry::get(entry).pprev);
}

umkk_ValidationMode umkk_block_validation_state_get_validation_mode(const umkk_BlockValidationState* block_validation_state_)
{
    auto& block_validation_state = umkk_BlockValidationState::get(block_validation_state_);
    if (block_validation_state.IsValid()) return umkk_ValidationMode_VALID;
    if (block_validation_state.IsInvalid()) return umkk_ValidationMode_INVALID;
    return umkk_ValidationMode_INTERNAL_ERROR;
}

umkk_BlockValidationResult umkk_block_validation_state_get_block_validation_result(const umkk_BlockValidationState* block_validation_state_)
{
    auto& block_validation_state = umkk_BlockValidationState::get(block_validation_state_);
    switch (block_validation_state.GetResult()) {
    case BlockValidationResult::BLOCK_RESULT_UNSET:
        return umkk_BlockValidationResult_UNSET;
    case BlockValidationResult::BLOCK_CONSENSUS:
        return umkk_BlockValidationResult_CONSENSUS;
    case BlockValidationResult::BLOCK_CACHED_INVALID:
        return umkk_BlockValidationResult_CACHED_INVALID;
    case BlockValidationResult::BLOCK_INVALID_HEADER:
        return umkk_BlockValidationResult_INVALID_HEADER;
    case BlockValidationResult::BLOCK_MUTATED:
        return umkk_BlockValidationResult_MUTATED;
    case BlockValidationResult::BLOCK_MISSING_PREV:
        return umkk_BlockValidationResult_MISSING_PREV;
    case BlockValidationResult::BLOCK_INVALID_PREV:
        return umkk_BlockValidationResult_INVALID_PREV;
    case BlockValidationResult::BLOCK_TIME_FUTURE:
        return umkk_BlockValidationResult_TIME_FUTURE;
    case BlockValidationResult::BLOCK_HEADER_LOW_WORK:
        return umkk_BlockValidationResult_HEADER_LOW_WORK;
    } // no default case, so the compiler can warn about missing cases
    assert(false);
}

umkk_ChainstateManagerOptions* umkk_chainstate_manager_options_create(const umkk_Context* context, const char* data_dir, size_t data_dir_len, const char* blocks_dir, size_t blocks_dir_len)
{
    try {
        fs::path abs_data_dir{fs::absolute(fs::PathFromString({data_dir, data_dir_len}))};
        fs::create_directories(abs_data_dir);
        fs::path abs_blocks_dir{fs::absolute(fs::PathFromString({blocks_dir, blocks_dir_len}))};
        fs::create_directories(abs_blocks_dir);
        return umkk_ChainstateManagerOptions::create(umkk_Context::get(context), abs_data_dir, abs_blocks_dir);
    } catch (const std::exception& e) {
        LogError("Failed to create chainstate manager options: %s", e.what());
        return nullptr;
    }
}

void umkk_chainstate_manager_options_set_worker_threads_num(umkk_ChainstateManagerOptions* opts, int worker_threads)
{
    LOCK(umkk_ChainstateManagerOptions::get(opts).m_mutex);
    umkk_ChainstateManagerOptions::get(opts).m_chainman_options.worker_threads_num = worker_threads;
}

void umkk_chainstate_manager_options_destroy(umkk_ChainstateManagerOptions* options)
{
    delete options;
}

int umkk_chainstate_manager_options_set_wipe_dbs(umkk_ChainstateManagerOptions* chainman_opts, int wipe_block_tree_db, int wipe_chainstate_db)
{
    if (wipe_block_tree_db == 1 && wipe_chainstate_db != 1) {
        LogError("Wiping the block tree db without also wiping the chainstate db is currently unsupported.");
        return -1;
    }
    auto& opts{umkk_ChainstateManagerOptions::get(chainman_opts)};
    LOCK(opts.m_mutex);
    opts.m_blockman_options.block_tree_db_params.wipe_data = wipe_block_tree_db == 1;
    opts.m_chainstate_load_options.wipe_chainstate_db = wipe_chainstate_db == 1;
    return 0;
}

void umkk_chainstate_manager_options_update_block_tree_db_in_memory(
    umkk_ChainstateManagerOptions* chainman_opts,
    int block_tree_db_in_memory)
{
    auto& opts{umkk_ChainstateManagerOptions::get(chainman_opts)};
    LOCK(opts.m_mutex);
    opts.m_blockman_options.block_tree_db_params.memory_only = block_tree_db_in_memory == 1;
}

void umkk_chainstate_manager_options_update_chainstate_db_in_memory(
    umkk_ChainstateManagerOptions* chainman_opts,
    int chainstate_db_in_memory)
{
    auto& opts{umkk_ChainstateManagerOptions::get(chainman_opts)};
    LOCK(opts.m_mutex);
    opts.m_chainstate_load_options.coins_db_in_memory = chainstate_db_in_memory == 1;
}

umkk_ChainstateManager* umkk_chainstate_manager_create(
    const umkk_ChainstateManagerOptions* chainman_opts)
{
    auto& opts{umkk_ChainstateManagerOptions::get(chainman_opts)};
    std::unique_ptr<ChainstateManager> chainman;
    try {
        LOCK(opts.m_mutex);
        chainman = std::make_unique<ChainstateManager>(*opts.m_context->m_interrupt, opts.m_chainman_options, opts.m_blockman_options);
    } catch (const std::exception& e) {
        LogError("Failed to create chainstate manager: %s", e.what());
        return nullptr;
    }

    try {
        const auto chainstate_load_opts{WITH_LOCK(opts.m_mutex, return opts.m_chainstate_load_options)};

        kernel::CacheSizes cache_sizes{DEFAULT_KERNEL_CACHE};
        auto [status, chainstate_err]{node::LoadChainstate(*chainman, cache_sizes, chainstate_load_opts)};
        if (status != node::ChainstateLoadStatus::SUCCESS) {
            LogError("Failed to load chain state from your data directory: %s", chainstate_err.original);
            return nullptr;
        }
        std::tie(status, chainstate_err) = node::VerifyLoadedChainstate(*chainman, chainstate_load_opts);
        if (status != node::ChainstateLoadStatus::SUCCESS) {
            LogError("Failed to verify loaded chain state from your datadir: %s", chainstate_err.original);
            return nullptr;
        }

        for (Chainstate* chainstate : WITH_LOCK(chainman->GetMutex(), return chainman->GetAll())) {
            BlockValidationState state;
            if (!chainstate->ActivateBestChain(state, nullptr)) {
                LogError("Failed to connect best block: %s", state.ToString());
                return nullptr;
            }
        }
    } catch (const std::exception& e) {
        LogError("Failed to load chainstate: %s", e.what());
        return nullptr;
    }

    return umkk_ChainstateManager::create(std::move(chainman), opts.m_context);
}

const umkk_BlockTreeEntry* umkk_chainstate_manager_get_block_tree_entry_by_hash(const umkk_ChainstateManager* chainman, const umkk_BlockHash* block_hash)
{
    auto block_index = WITH_LOCK(umkk_ChainstateManager::get(chainman).m_chainman->GetMutex(),
                                 return umkk_ChainstateManager::get(chainman).m_chainman->m_blockman.LookupBlockIndex(umkk_BlockHash::get(block_hash)));
    if (!block_index) {
        LogDebug(BCLog::KERNEL, "A block with the given hash is not indexed.");
        return nullptr;
    }
    return umkk_BlockTreeEntry::ref(block_index);
}

void umkk_chainstate_manager_destroy(umkk_ChainstateManager* chainman)
{
    {
        LOCK(umkk_ChainstateManager::get(chainman).m_chainman->GetMutex());
        for (Chainstate* chainstate : umkk_ChainstateManager::get(chainman).m_chainman->GetAll()) {
            if (chainstate->CanFlushToDisk()) {
                chainstate->ForceFlushStateToDisk();
                chainstate->ResetCoinsViews();
            }
        }
    }

    delete chainman;
}

int umkk_chainstate_manager_import_blocks(umkk_ChainstateManager* chainman, const char** block_file_paths_data, size_t* block_file_paths_lens, size_t block_file_paths_data_len)
{
    try {
        std::vector<fs::path> import_files;
        import_files.reserve(block_file_paths_data_len);
        for (uint32_t i = 0; i < block_file_paths_data_len; i++) {
            if (block_file_paths_data[i] != nullptr) {
                import_files.emplace_back(std::string{block_file_paths_data[i], block_file_paths_lens[i]}.c_str());
            }
        }
        node::ImportBlocks(*umkk_ChainstateManager::get(chainman).m_chainman, import_files);
    } catch (const std::exception& e) {
        LogError("Failed to import blocks: %s", e.what());
        return -1;
    }
    return 0;
}

umkk_Block* umkk_block_create(const void* raw_block, size_t raw_block_length)
{
    if (raw_block == nullptr && raw_block_length != 0) {
        return nullptr;
    }
    auto block{std::make_shared<CBlock>()};

    DataStream stream{std::span{reinterpret_cast<const std::byte*>(raw_block), raw_block_length}};

    try {
        stream >> TX_WITH_WITNESS(*block);
    } catch (...) {
        LogDebug(BCLog::KERNEL, "Block decode failed.");
        return nullptr;
    }

    return umkk_Block::create(block);
}

umkk_Block* umkk_block_copy(const umkk_Block* block)
{
    return umkk_Block::copy(block);
}

size_t umkk_block_count_transactions(const umkk_Block* block)
{
    return umkk_Block::get(block)->vtx.size();
}

const umkk_Transaction* umkk_block_get_transaction_at(const umkk_Block* block, size_t index)
{
    assert(index < umkk_Block::get(block)->vtx.size());
    return umkk_Transaction::ref(&umkk_Block::get(block)->vtx[index]);
}

int umkk_block_to_bytes(const umkk_Block* block, umkk_WriteBytes writer, void* user_data)
{
    try {
        WriterStream ws{writer, user_data};
        ws << TX_WITH_WITNESS(*umkk_Block::get(block));
        return 0;
    } catch (...) {
        return -1;
    }
}

umkk_BlockHash* umkk_block_get_hash(const umkk_Block* block)
{
    return umkk_BlockHash::create(umkk_Block::get(block)->GetHash());
}

void umkk_block_destroy(umkk_Block* block)
{
    delete block;
}

umkk_Block* umkk_block_read(const umkk_ChainstateManager* chainman, const umkk_BlockTreeEntry* entry)
{
    auto block{std::make_shared<CBlock>()};
    if (!umkk_ChainstateManager::get(chainman).m_chainman->m_blockman.ReadBlock(*block, umkk_BlockTreeEntry::get(entry))) {
        LogError("Failed to read block.");
        return nullptr;
    }
    return umkk_Block::create(block);
}

int32_t umkk_block_tree_entry_get_height(const umkk_BlockTreeEntry* entry)
{
    return umkk_BlockTreeEntry::get(entry).nHeight;
}

const umkk_BlockHash* umkk_block_tree_entry_get_block_hash(const umkk_BlockTreeEntry* entry)
{
    return umkk_BlockHash::ref(umkk_BlockTreeEntry::get(entry).phashBlock);
}

umkk_BlockHash* umkk_block_hash_create(const unsigned char block_hash[32])
{
    return umkk_BlockHash::create(std::span<const unsigned char>{block_hash, 32});
}

umkk_BlockHash* umkk_block_hash_copy(const umkk_BlockHash* block_hash)
{
    return umkk_BlockHash::copy(block_hash);
}

void umkk_block_hash_to_bytes(const umkk_BlockHash* block_hash, unsigned char output[32])
{
    std::memcpy(output, umkk_BlockHash::get(block_hash).begin(), 32);
}

int umkk_block_hash_equals(const umkk_BlockHash* hash1, const umkk_BlockHash* hash2)
{
    return umkk_BlockHash::get(hash1) == umkk_BlockHash::get(hash2);
}

void umkk_block_hash_destroy(umkk_BlockHash* hash)
{
    delete hash;
}

umkk_BlockSpentOutputs* umkk_block_spent_outputs_read(const umkk_ChainstateManager* chainman, const umkk_BlockTreeEntry* entry)
{
    auto block_undo{std::make_shared<CBlockUndo>()};
    if (umkk_BlockTreeEntry::get(entry).nHeight < 1) {
        LogDebug(BCLog::KERNEL, "The genesis block does not have any spent outputs.");
        return umkk_BlockSpentOutputs::create(block_undo);
    }
    if (!umkk_ChainstateManager::get(chainman).m_chainman->m_blockman.ReadBlockUndo(*block_undo, umkk_BlockTreeEntry::get(entry))) {
        LogError("Failed to read block spent outputs data.");
        return nullptr;
    }
    return umkk_BlockSpentOutputs::create(block_undo);
}

umkk_BlockSpentOutputs* umkk_block_spent_outputs_copy(const umkk_BlockSpentOutputs* block_spent_outputs)
{
    return umkk_BlockSpentOutputs::copy(block_spent_outputs);
}

size_t umkk_block_spent_outputs_count(const umkk_BlockSpentOutputs* block_spent_outputs)
{
    return umkk_BlockSpentOutputs::get(block_spent_outputs)->vtxundo.size();
}

const umkk_TransactionSpentOutputs* umkk_block_spent_outputs_get_transaction_spent_outputs_at(const umkk_BlockSpentOutputs* block_spent_outputs, size_t transaction_index)
{
    assert(transaction_index < umkk_BlockSpentOutputs::get(block_spent_outputs)->vtxundo.size());
    const auto* tx_undo{&umkk_BlockSpentOutputs::get(block_spent_outputs)->vtxundo.at(transaction_index)};
    return umkk_TransactionSpentOutputs::ref(tx_undo);
}

void umkk_block_spent_outputs_destroy(umkk_BlockSpentOutputs* block_spent_outputs)
{
    delete block_spent_outputs;
}

umkk_TransactionSpentOutputs* umkk_transaction_spent_outputs_copy(const umkk_TransactionSpentOutputs* transaction_spent_outputs)
{
    return umkk_TransactionSpentOutputs::copy(transaction_spent_outputs);
}

size_t umkk_transaction_spent_outputs_count(const umkk_TransactionSpentOutputs* transaction_spent_outputs)
{
    return umkk_TransactionSpentOutputs::get(transaction_spent_outputs).vprevout.size();
}

void umkk_transaction_spent_outputs_destroy(umkk_TransactionSpentOutputs* transaction_spent_outputs)
{
    delete transaction_spent_outputs;
}

const umkk_Coin* umkk_transaction_spent_outputs_get_coin_at(const umkk_TransactionSpentOutputs* transaction_spent_outputs, size_t coin_index)
{
    assert(coin_index < umkk_TransactionSpentOutputs::get(transaction_spent_outputs).vprevout.size());
    const Coin* coin{&umkk_TransactionSpentOutputs::get(transaction_spent_outputs).vprevout.at(coin_index)};
    return umkk_Coin::ref(coin);
}

umkk_Coin* umkk_coin_copy(const umkk_Coin* coin)
{
    return umkk_Coin::copy(coin);
}

uint32_t umkk_coin_confirmation_height(const umkk_Coin* coin)
{
    return umkk_Coin::get(coin).nHeight;
}

int umkk_coin_is_coinbase(const umkk_Coin* coin)
{
    return umkk_Coin::get(coin).IsCoinBase() ? 1 : 0;
}

const umkk_TransactionOutput* umkk_coin_get_output(const umkk_Coin* coin)
{
    return umkk_TransactionOutput::ref(&umkk_Coin::get(coin).out);
}

void umkk_coin_destroy(umkk_Coin* coin)
{
    delete coin;
}

int umkk_chainstate_manager_process_block(
    umkk_ChainstateManager* chainman,
    const umkk_Block* block,
    int* _new_block)
{
    bool new_block;
    auto result = umkk_ChainstateManager::get(chainman).m_chainman->ProcessNewBlock(umkk_Block::get(block), /*force_processing=*/true, /*min_pow_checked=*/true, /*new_block=*/&new_block);
    if (_new_block) {
        *_new_block = new_block ? 1 : 0;
    }
    return result ? 0 : -1;
}

const umkk_Chain* umkk_chainstate_manager_get_active_chain(const umkk_ChainstateManager* chainman)
{
    return umkk_Chain::ref(&WITH_LOCK(umkk_ChainstateManager::get(chainman).m_chainman->GetMutex(), return umkk_ChainstateManager::get(chainman).m_chainman->ActiveChain()));
}

int umkk_chain_get_height(const umkk_Chain* chain)
{
    LOCK(::cs_main);
    return umkk_Chain::get(chain).Height();
}

const umkk_BlockTreeEntry* umkk_chain_get_by_height(const umkk_Chain* chain, int height)
{
    LOCK(::cs_main);
    return umkk_BlockTreeEntry::ref(umkk_Chain::get(chain)[height]);
}

int umkk_chain_contains(const umkk_Chain* chain, const umkk_BlockTreeEntry* entry)
{
    LOCK(::cs_main);
    return umkk_Chain::get(chain).Contains(&umkk_BlockTreeEntry::get(entry)) ? 1 : 0;
}
