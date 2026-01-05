// Copyright (c) 2024-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UMKOIN_KERNEL_UMKOINKERNEL_H
#define UMKOIN_KERNEL_UMKOINKERNEL_H

#ifndef __cplusplus
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif // __cplusplus

#ifndef UMKOINKERNEL_API
    #ifdef UMKOINKERNEL_BUILD
        #if defined(_WIN32)
            #define UMKOINKERNEL_API __declspec(dllexport)
        #else
            #define UMKOINKERNEL_API __attribute__((visibility("default")))
        #endif
    #else
        #if defined(_WIN32) && !defined(UMKOINKERNEL_STATIC)
            #define UMKOINKERNEL_API __declspec(dllimport)
        #else
            #define UMKOINKERNEL_API
        #endif
    #endif
#endif

/* Warning attributes */
#if defined(__GNUC__)
    #define UMKOINKERNEL_WARN_UNUSED_RESULT __attribute__((__warn_unused_result__))
#else
    #define UMKOINKERNEL_WARN_UNUSED_RESULT
#endif

/**
 * UMKOINKERNEL_ARG_NONNULL is a compiler attribute used to indicate that
 * certain pointer arguments to a function are not expected to be null.
 *
 * Callers must not pass a null pointer for arguments marked with this attribute,
 * as doing so may result in undefined behavior. This attribute should only be
 * used for arguments where a null pointer is unambiguously a programmer error,
 * such as for opaque handles, and not for pointers to raw input data that might
 * validly be null (e.g., from an empty std::span or std::string).
 */
#if !defined(UMKOINKERNEL_BUILD) && defined(__GNUC__)
    #define UMKOINKERNEL_ARG_NONNULL(...) __attribute__((__nonnull__(__VA_ARGS__)))
#else
    #define UMKOINKERNEL_ARG_NONNULL(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @page remarks Remarks
 *
 * @section purpose Purpose
 *
 * This header currently exposes an API for interacting with parts of Umkoin
 * Core's consensus code. Users can validate blocks, iterate the block index,
 * read block and undo data from disk, and validate scripts. The header is
 * unversioned and not stable yet. Users should expect breaking changes. It is
 * also not yet included in releases of Umkoin Core.
 *
 * @section context Context
 *
 * The library provides a built-in static constant kernel context. This static
 * context offers only limited functionality. It detects and self-checks the
 * correct sha256 implementation, initializes the random number generator and
 * self-checks the secp256k1 static context. It is used internally for
 * otherwise "context-free" operations. This means that the user is not
 * required to initialize their own context before using the library.
 *
 * The user should create their own context for passing it to state-rich validation
 * functions and holding callbacks for kernel events.
 *
 * @section error Error handling
 *
 * Functions communicate an error through their return types, usually returning
 * a nullptr, 0, or false if an error is encountered. Additionally, verification
 * functions, e.g. for scripts, may communicate more detailed error information
 * through status code out parameters.
 *
 * Fine-grained validation information is communicated through the validation
 * interface.
 *
 * The kernel notifications issue callbacks for errors. These are usually
 * indicative of a system error. If such an error is issued, it is recommended
 * to halt and tear down the existing kernel objects. Remediating the error may
 * require system intervention by the user.
 *
 * @section pointer Pointer and argument conventions
 *
 * The user is responsible for de-allocating the memory owned by pointers
 * returned by functions. Typically pointers returned by *_create(...) functions
 * can be de-allocated by corresponding *_destroy(...) functions.
 *
 * A function that takes pointer arguments makes no assumptions on their
 * lifetime. Once the function returns the user can safely de-allocate the
 * passed in arguments.
 *
 * Const pointers represent views, and do not transfer ownership. Lifetime
 * guarantees of these objects are described in the respective documentation.
 * Ownership of these resources may be taken by copying. They are typically
 * used for iteration with minimal overhead and require some care by the
 * programmer that their lifetime is not extended beyond that of the original
 * object.
 *
 * Array lengths follow the pointer argument they describe.
 */

/**
 * Opaque data structure for holding a transaction.
 */
typedef struct umkk_Transaction umkk_Transaction;

/**
 * Opaque data structure for holding a script pubkey.
 */
typedef struct umkk_ScriptPubkey umkk_ScriptPubkey;

/**
 * Opaque data structure for holding a transaction output.
 */
typedef struct umkk_TransactionOutput umkk_TransactionOutput;

/**
 * Opaque data structure for holding a logging connection.
 *
 * The logging connection can be used to manually stop logging.
 *
 * Messages that were logged before a connection is created are buffered in a
 * 1MB buffer. Logging can alternatively be permanently disabled by calling
 * @ref umkk_logging_disable. Functions changing the logging settings are
 * global and change the settings for all existing umkk_LoggingConnection
 * instances.
 */
typedef struct umkk_LoggingConnection umkk_LoggingConnection;

/**
 * Opaque data structure for holding the chain parameters.
 *
 * These are eventually placed into a kernel context through the kernel context
 * options. The parameters describe the properties of a chain, and may be
 * instantiated for either mainnet, testnet, signet, or regtest.
 */
typedef struct umkk_ChainParameters umkk_ChainParameters;

/**
 * Opaque data structure for holding options for creating a new kernel context.
 *
 * Once a kernel context has been created from these options, they may be
 * destroyed. The options hold the notification and validation interface
 * callbacks as well as the selected chain type until they are passed to the
 * context. If no options are configured, the context will be instantiated with
 * no callbacks and for mainnet. Their content and scope can be expanded over
 * time.
 */
typedef struct umkk_ContextOptions umkk_ContextOptions;

/**
 * Opaque data structure for holding a kernel context.
 *
 * The kernel context is used to initialize internal state and hold the chain
 * parameters and callbacks for handling error and validation events. Once
 * other validation objects are instantiated from it, the context is kept in
 * memory for the duration of their lifetimes.
 *
 * The processing of validation events is done through an internal task runner
 * owned by the context. It passes events through the registered validation
 * interface callbacks.
 *
 * A constructed context can be safely used from multiple threads.
 */
typedef struct umkk_Context umkk_Context;

/**
 * Opaque data structure for holding a block tree entry.
 *
 * This is a pointer to an element in the block index currently in memory of
 * the chainstate manager. It is valid for the lifetime of the chainstate
 * manager it was retrieved from. The entry is part of a tree-like structure
 * that is maintained internally. Every entry, besides the genesis, points to a
 * single parent. Multiple entries may share a parent, thus forming a tree.
 * Each entry corresponds to a single block and may be used to retrieve its
 * data and validation status.
 */
typedef struct umkk_BlockTreeEntry umkk_BlockTreeEntry;

/**
 * Opaque data structure for holding options for creating a new chainstate
 * manager.
 *
 * The chainstate manager options are used to set some parameters for the
 * chainstate manager.
 */
typedef struct umkk_ChainstateManagerOptions umkk_ChainstateManagerOptions;

/**
 * Opaque data structure for holding a chainstate manager.
 *
 * The chainstate manager is the central object for doing validation tasks as
 * well as retrieving data from the chain. Internally it is a complex data
 * structure with diverse functionality.
 *
 * Its functionality will be more and more exposed in the future.
 */
typedef struct umkk_ChainstateManager umkk_ChainstateManager;

/**
 * Opaque data structure for holding a block.
 */
typedef struct umkk_Block umkk_Block;

/**
 * Opaque data structure for holding the state of a block during validation.
 *
 * Contains information indicating whether validation was successful, and if not
 * which step during block validation failed.
 */
typedef struct umkk_BlockValidationState umkk_BlockValidationState;

/**
 * Opaque data structure for holding the currently known best-chain associated
 * with a chainstate.
 */
typedef struct umkk_Chain umkk_Chain;

/**
 * Opaque data structure for holding a block's spent outputs.
 *
 * Contains all the previous outputs consumed by all transactions in a specific
 * block. Internally it holds a nested vector. The top level vector has an
 * entry for each transaction in a block (in order of the actual transactions
 * of the block and without the coinbase transaction). This is exposed through
 * @ref umkk_TransactionSpentOutputs. Each umkk_TransactionSpentOutputs is in
 * turn a vector of all the previous outputs of a transaction (in order of
 * their corresponding inputs).
 */
typedef struct umkk_BlockSpentOutputs umkk_BlockSpentOutputs;

/**
 * Opaque data structure for holding a transaction's spent outputs.
 *
 * Holds the coins consumed by a certain transaction. Retrieved through the
 * @ref umkk_BlockSpentOutputs. The coins are in the same order as the
 * transaction's inputs consuming them.
 */
typedef struct umkk_TransactionSpentOutputs umkk_TransactionSpentOutputs;

/**
 * Opaque data structure for holding a coin.
 *
 * Holds information on the @ref umkk_TransactionOutput held within,
 * including the height it was spent at and whether it is a coinbase output.
 */
typedef struct umkk_Coin umkk_Coin;

/**
 * Opaque data structure for holding a block hash.
 *
 * This is a type-safe identifier for a block.
 */
typedef struct umkk_BlockHash umkk_BlockHash;

/**
 * Opaque data structure for holding a transaction input.
 *
 * Holds information on the @ref umkk_TransactionOutPoint held within.
 */
typedef struct umkk_TransactionInput umkk_TransactionInput;

/**
 * Opaque data structure for holding a transaction out point.
 *
 * Holds the txid and output index it is pointing to.
 */
typedef struct umkk_TransactionOutPoint umkk_TransactionOutPoint;

/**
 * Opaque data structure for holding precomputed transaction data.
 *
 * Reusable when verifying multiple inputs of the same transaction.
 * This avoids recomputing transaction hashes for each input.
 *
 * Required when verifying a taproot input.
 */
typedef struct umkk_PrecomputedTransactionData umkk_PrecomputedTransactionData;

typedef struct umkk_Txid umkk_Txid;

/** Current sync state passed to tip changed callbacks. */
typedef uint8_t umkk_SynchronizationState;
#define umkk_SynchronizationState_INIT_REINDEX ((umkk_SynchronizationState)(0))
#define umkk_SynchronizationState_INIT_DOWNLOAD ((umkk_SynchronizationState)(1))
#define umkk_SynchronizationState_POST_INIT ((umkk_SynchronizationState)(2))

/** Possible warning types issued by validation. */
typedef uint8_t umkk_Warning;
#define umkk_Warning_UNKNOWN_NEW_RULES_ACTIVATED ((umkk_Warning)(0))
#define umkk_Warning_LARGE_WORK_INVALID_CHAIN ((umkk_Warning)(1))

/** Callback function types */

/**
 * Function signature for the global logging callback. All umkoin kernel
 * internal logs will pass through this callback.
 */
typedef void (*umkk_LogCallback)(void* user_data, const char* message, size_t message_len);

/**
 * Function signature for freeing user data.
 */
typedef void (*umkk_DestroyCallback)(void* user_data);

/**
 * Function signatures for the kernel notifications.
 */
typedef void (*umkk_NotifyBlockTip)(void* user_data, umkk_SynchronizationState state, const umkk_BlockTreeEntry* entry, double verification_progress);
typedef void (*umkk_NotifyHeaderTip)(void* user_data, umkk_SynchronizationState state, int64_t height, int64_t timestamp, int presync);
typedef void (*umkk_NotifyProgress)(void* user_data, const char* title, size_t title_len, int progress_percent, int resume_possible);
typedef void (*umkk_NotifyWarningSet)(void* user_data, umkk_Warning warning, const char* message, size_t message_len);
typedef void (*umkk_NotifyWarningUnset)(void* user_data, umkk_Warning warning);
typedef void (*umkk_NotifyFlushError)(void* user_data, const char* message, size_t message_len);
typedef void (*umkk_NotifyFatalError)(void* user_data, const char* message, size_t message_len);

/**
 * Function signatures for the validation interface.
 */
typedef void (*umkk_ValidationInterfaceBlockChecked)(void* user_data, umkk_Block* block, const umkk_BlockValidationState* state);
typedef void (*umkk_ValidationInterfacePoWValidBlock)(void* user_data, umkk_Block* block, const umkk_BlockTreeEntry* entry);
typedef void (*umkk_ValidationInterfaceBlockConnected)(void* user_data, umkk_Block* block, const umkk_BlockTreeEntry* entry);
typedef void (*umkk_ValidationInterfaceBlockDisconnected)(void* user_data, umkk_Block* block, const umkk_BlockTreeEntry* entry);

/**
 * Function signature for serializing data.
 */
typedef int (*umkk_WriteBytes)(const void* bytes, size_t size, void* userdata);

/**
 * Whether a validated data structure is valid, invalid, or an error was
 * encountered during processing.
 */
typedef uint8_t umkk_ValidationMode;
#define umkk_ValidationMode_VALID ((umkk_ValidationMode)(0))
#define umkk_ValidationMode_INVALID ((umkk_ValidationMode)(1))
#define umkk_ValidationMode_INTERNAL_ERROR ((umkk_ValidationMode)(2))

/**
 * A granular "reason" why a block was invalid.
 */
typedef uint32_t umkk_BlockValidationResult;
#define umkk_BlockValidationResult_UNSET ((umkk_BlockValidationResult)(0))           //!< initial value. Block has not yet been rejected
#define umkk_BlockValidationResult_CONSENSUS ((umkk_BlockValidationResult)(1))       //!< invalid by consensus rules (excluding any below reasons)
#define umkk_BlockValidationResult_CACHED_INVALID ((umkk_BlockValidationResult)(2))  //!< this block was cached as being invalid and we didn't store the reason why
#define umkk_BlockValidationResult_INVALID_HEADER ((umkk_BlockValidationResult)(3))  //!< invalid proof of work or time too old
#define umkk_BlockValidationResult_MUTATED ((umkk_BlockValidationResult)(4))         //!< the block's data didn't match the data committed to by the PoW
#define umkk_BlockValidationResult_MISSING_PREV ((umkk_BlockValidationResult)(5))    //!< We don't have the previous block the checked one is built on
#define umkk_BlockValidationResult_INVALID_PREV ((umkk_BlockValidationResult)(6))    //!< A block this one builds on is invalid
#define umkk_BlockValidationResult_TIME_FUTURE ((umkk_BlockValidationResult)(7))     //!< block timestamp was > 2 hours in the future (or our clock is bad)
#define umkk_BlockValidationResult_HEADER_LOW_WORK ((umkk_BlockValidationResult)(8)) //!< the block header may be on a too-little-work chain

/**
 * Holds the validation interface callbacks. The user data pointer may be used
 * to point to user-defined structures to make processing the validation
 * callbacks easier. Note that these callbacks block any further validation
 * execution when they are called.
 */
typedef struct {
    void* user_data;                                              //!< Holds a user-defined opaque structure that is passed to the validation
                                                                  //!< interface callbacks. If user_data_destroy is also defined ownership of the
                                                                  //!< user_data is passed to the created context options and subsequently context.
    umkk_DestroyCallback user_data_destroy;                       //!< Frees the provided user data structure.
    umkk_ValidationInterfaceBlockChecked block_checked;           //!< Called when a new block has been fully validated. Contains the
                                                                  //!< result of its validation.
    umkk_ValidationInterfacePoWValidBlock pow_valid_block;        //!< Called when a new block extends the header chain and has a valid transaction
                                                                  //!< and segwit merkle root.
    umkk_ValidationInterfaceBlockConnected block_connected;       //!< Called when a block is valid and has now been connected to the best chain.
    umkk_ValidationInterfaceBlockDisconnected block_disconnected; //!< Called during a re-org when a block has been removed from the best chain.
} umkk_ValidationInterfaceCallbacks;

/**
 * A struct for holding the kernel notification callbacks. The user data
 * pointer may be used to point to user-defined structures to make processing
 * the notifications easier.
 *
 * If user_data_destroy is provided, the kernel will automatically call this
 * callback to clean up user_data when the notification interface object is destroyed.
 * If user_data_destroy is NULL, it is the user's responsibility to ensure that
 * the user_data outlives the kernel objects. Notifications can
 * occur even as kernel objects are deleted, so care has to be taken to ensure
 * safe unwinding.
 */
typedef struct {
    void* user_data;                        //!< Holds a user-defined opaque structure that is passed to the notification callbacks.
                                            //!< If user_data_destroy is also defined ownership of the user_data is passed to the
                                            //!< created context options and subsequently context.
    umkk_DestroyCallback user_data_destroy; //!< Frees the provided user data structure.
    umkk_NotifyBlockTip block_tip;          //!< The chain's tip was updated to the provided block entry.
    umkk_NotifyHeaderTip header_tip;        //!< A new best block header was added.
    umkk_NotifyProgress progress;           //!< Reports on current block synchronization progress.
    umkk_NotifyWarningSet warning_set;      //!< A warning issued by the kernel library during validation.
    umkk_NotifyWarningUnset warning_unset;  //!< A previous condition leading to the issuance of a warning is no longer given.
    umkk_NotifyFlushError flush_error;      //!< An error encountered when flushing data to disk.
    umkk_NotifyFatalError fatal_error;      //!< An unrecoverable system error encountered by the library.
} umkk_NotificationInterfaceCallbacks;

/**
 * A collection of logging categories that may be encountered by kernel code.
 */
typedef uint8_t umkk_LogCategory;
#define umkk_LogCategory_ALL ((umkk_LogCategory)(0))
#define umkk_LogCategory_BENCH ((umkk_LogCategory)(1))
#define umkk_LogCategory_BLOCKSTORAGE ((umkk_LogCategory)(2))
#define umkk_LogCategory_COINDB ((umkk_LogCategory)(3))
#define umkk_LogCategory_LEVELDB ((umkk_LogCategory)(4))
#define umkk_LogCategory_MEMPOOL ((umkk_LogCategory)(5))
#define umkk_LogCategory_PRUNE ((umkk_LogCategory)(6))
#define umkk_LogCategory_RAND ((umkk_LogCategory)(7))
#define umkk_LogCategory_REINDEX ((umkk_LogCategory)(8))
#define umkk_LogCategory_VALIDATION ((umkk_LogCategory)(9))
#define umkk_LogCategory_KERNEL ((umkk_LogCategory)(10))

/**
 * The level at which logs should be produced.
 */
typedef uint8_t umkk_LogLevel;
#define umkk_LogLevel_TRACE ((umkk_LogLevel)(0))
#define umkk_LogLevel_DEBUG ((umkk_LogLevel)(1))
#define umkk_LogLevel_INFO ((umkk_LogLevel)(2))

/**
 * Options controlling the format of log messages.
 *
 * Set fields as non-zero to indicate true.
 */
typedef struct {
    int log_timestamps;               //!< Prepend a timestamp to log messages.
    int log_time_micros;              //!< Log timestamps in microsecond precision.
    int log_threadnames;              //!< Prepend the name of the thread to log messages.
    int log_sourcelocations;          //!< Prepend the source location to log messages.
    int always_print_category_levels; //!< Prepend the log category and level to log messages.
} umkk_LoggingOptions;

/**
 * A collection of status codes that may be issued by the script verify function.
 */
typedef uint8_t umkk_ScriptVerifyStatus;
#define umkk_ScriptVerifyStatus_OK ((umkk_ScriptVerifyStatus)(0))
#define umkk_ScriptVerifyStatus_ERROR_INVALID_FLAGS_COMBINATION ((umkk_ScriptVerifyStatus)(1)) //!< The flags were combined in an invalid way.
#define umkk_ScriptVerifyStatus_ERROR_SPENT_OUTPUTS_REQUIRED ((umkk_ScriptVerifyStatus)(2))    //!< The taproot flag was set, so valid spent_outputs have to be provided.

/**
 * Script verification flags that may be composed with each other.
 */
typedef uint32_t umkk_ScriptVerificationFlags;
#define umkk_ScriptVerificationFlags_NONE ((umkk_ScriptVerificationFlags)(0))
#define umkk_ScriptVerificationFlags_P2SH ((umkk_ScriptVerificationFlags)(1U << 0))                 //!< evaluate P2SH (BIP16) subscripts
#define umkk_ScriptVerificationFlags_DERSIG ((umkk_ScriptVerificationFlags)(1U << 2))               //!< enforce strict DER (BIP66) compliance
#define umkk_ScriptVerificationFlags_NULLDUMMY ((umkk_ScriptVerificationFlags)(1U << 4))            //!< enforce NULLDUMMY (BIP147)
#define umkk_ScriptVerificationFlags_CHECKLOCKTIMEVERIFY ((umkk_ScriptVerificationFlags)(1U << 9))  //!< enable CHECKLOCKTIMEVERIFY (BIP65)
#define umkk_ScriptVerificationFlags_CHECKSEQUENCEVERIFY ((umkk_ScriptVerificationFlags)(1U << 10)) //!< enable CHECKSEQUENCEVERIFY (BIP112)
#define umkk_ScriptVerificationFlags_WITNESS ((umkk_ScriptVerificationFlags)(1U << 11))             //!< enable WITNESS (BIP141)
#define umkk_ScriptVerificationFlags_TAPROOT ((umkk_ScriptVerificationFlags)(1U << 17))             //!< enable TAPROOT (BIPs 341 & 342)
#define umkk_ScriptVerificationFlags_ALL ((umkk_ScriptVerificationFlags)(umkk_ScriptVerificationFlags_P2SH |                \
                                                                         umkk_ScriptVerificationFlags_DERSIG |              \
                                                                         umkk_ScriptVerificationFlags_NULLDUMMY |           \
                                                                         umkk_ScriptVerificationFlags_CHECKLOCKTIMEVERIFY | \
                                                                         umkk_ScriptVerificationFlags_CHECKSEQUENCEVERIFY | \
                                                                         umkk_ScriptVerificationFlags_WITNESS |             \
                                                                         umkk_ScriptVerificationFlags_TAPROOT))

typedef uint8_t umkk_ChainType;
#define umkk_ChainType_MAINNET ((umkk_ChainType)(0))
#define umkk_ChainType_TESTNET ((umkk_ChainType)(1))
#define umkk_ChainType_TESTNET_4 ((umkk_ChainType)(2))
#define umkk_ChainType_SIGNET ((umkk_ChainType)(3))
#define umkk_ChainType_REGTEST ((umkk_ChainType)(4))

/** @name Transaction
 * Functions for working with transactions.
 */
///@{

/**
 * @brief Create a new transaction from the serialized data.
 *
 * @param[in] raw_transaction     Serialized transaction.
 * @param[in] raw_transaction_len Length of the serialized transaction.
 * @return                        The transaction, or null on error.
 */
UMKOINKERNEL_API umkk_Transaction* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_create(
    const void* raw_transaction, size_t raw_transaction_len);

/**
 * @brief Copy a transaction. Transactions are reference counted, so this just
 * increments the reference count.
 *
 * @param[in] transaction Non-null.
 * @return                The copied transaction.
 */
UMKOINKERNEL_API umkk_Transaction* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_copy(
    const umkk_Transaction* transaction) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Serializes the transaction through the passed in callback to bytes.
 * This is consensus serialization that is also used for the P2P network.
 *
 * @param[in] transaction Non-null.
 * @param[in] writer      Non-null, callback to a write bytes function.
 * @param[in] user_data   Holds a user-defined opaque structure that will be
 *                        passed back through the writer callback.
 * @return                0 on success.
 */
UMKOINKERNEL_API int umkk_transaction_to_bytes(
    const umkk_Transaction* transaction,
    umkk_WriteBytes writer,
    void* user_data) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Get the number of outputs of a transaction.
 *
 * @param[in] transaction Non-null.
 * @return                The number of outputs.
 */
UMKOINKERNEL_API size_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_count_outputs(
    const umkk_Transaction* transaction) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the transaction outputs at the provided index. The returned
 * transaction output is not owned and depends on the lifetime of the
 * transaction.
 *
 * @param[in] transaction  Non-null.
 * @param[in] output_index The index of the transaction output to be retrieved.
 * @return                 The transaction output
 */
UMKOINKERNEL_API const umkk_TransactionOutput* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_get_output_at(
    const umkk_Transaction* transaction, size_t output_index) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the transaction input at the provided index. The returned
 * transaction input is not owned and depends on the lifetime of the
 * transaction.
 *
 * @param[in] transaction Non-null.
 * @param[in] input_index The index of the transaction input to be retrieved.
 * @return                 The transaction input
 */
UMKOINKERNEL_API const umkk_TransactionInput* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_get_input_at(
    const umkk_Transaction* transaction, size_t input_index) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the number of inputs of a transaction.
 *
 * @param[in] transaction Non-null.
 * @return                The number of inputs.
 */
UMKOINKERNEL_API size_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_count_inputs(
    const umkk_Transaction* transaction) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the txid of a transaction. The returned txid is not owned and
 * depends on the lifetime of the transaction.
 *
 * @param[in] transaction Non-null.
 * @return                The txid.
 */
UMKOINKERNEL_API const umkk_Txid* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_get_txid(
    const umkk_Transaction* transaction) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction.
 */
UMKOINKERNEL_API void umkk_transaction_destroy(umkk_Transaction* transaction);

///@}

/** @name PrecomputedTransactionData
 * Functions for working with precomputed transaction data.
 */
///@{

/**
 * @brief Create precomputed transaction data for script verification.
 *
 * @param[in] tx_to             Non-null.
 * @param[in] spent_outputs     Nullable for non-taproot verification. Points to an array of
 *                              outputs spent by the transaction.
 * @param[in] spent_outputs_len Length of the spent_outputs array.
 * @return                      The precomputed data, or null on error.
 */
UMKOINKERNEL_API umkk_PrecomputedTransactionData* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_precomputed_transaction_data_create(
    const umkk_Transaction* tx_to,
    const umkk_TransactionOutput** spent_outputs, size_t spent_outputs_len) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Copy precomputed transaction data.
 *
 * @param[in] precomputed_txdata Non-null.
 * @return                       The copied precomputed transaction data.
 */
UMKOINKERNEL_API umkk_PrecomputedTransactionData* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_precomputed_transaction_data_copy(
    const umkk_PrecomputedTransactionData* precomputed_txdata) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the precomputed transaction data.
 */
UMKOINKERNEL_API void umkk_precomputed_transaction_data_destroy(umkk_PrecomputedTransactionData* precomputed_txdata);

///@}

/** @name ScriptPubkey
 * Functions for working with script pubkeys.
 */
///@{

/**
 * @brief Create a script pubkey from serialized data.
 * @param[in] script_pubkey     Serialized script pubkey.
 * @param[in] script_pubkey_len Length of the script pubkey data.
 * @return                      The script pubkey.
 */
UMKOINKERNEL_API umkk_ScriptPubkey* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_script_pubkey_create(
    const void* script_pubkey, size_t script_pubkey_len);

/**
 * @brief Copy a script pubkey.
 *
 * @param[in] script_pubkey Non-null.
 * @return                  The copied script pubkey.
 */
UMKOINKERNEL_API umkk_ScriptPubkey* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_script_pubkey_copy(
    const umkk_ScriptPubkey* script_pubkey) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Verify if the input at input_index of tx_to spends the script pubkey
 * under the constraints specified by flags. If the
 * `umkk_ScriptVerificationFlags_WITNESS` flag is set in the flags bitfield, the
 * amount parameter is used. If the taproot flag is set, the precomputed data
 * must contain the spent outputs.
 *
 * @param[in] script_pubkey      Non-null, script pubkey to be spent.
 * @param[in] amount             Amount of the script pubkey's associated output. May be zero if
 *                               the witness flag is not set.
 * @param[in] tx_to              Non-null, transaction spending the script_pubkey.
 * @param[in] precomputed_txdata Nullable if the taproot flag is not set. Otherwise, precomputed data
 *                               for tx_to with the spent outputs must be provided.
 * @param[in] input_index        Index of the input in tx_to spending the script_pubkey.
 * @param[in] flags              Bitfield of umkk_ScriptVerificationFlags controlling validation constraints.
 * @param[out] status            Nullable, will be set to an error code if the operation fails, or OK otherwise.
 * @return                       1 if the script is valid, 0 otherwise.
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_script_pubkey_verify(
    const umkk_ScriptPubkey* script_pubkey,
    int64_t amount,
    const umkk_Transaction* tx_to,
    const umkk_PrecomputedTransactionData* precomputed_txdata,
    unsigned int input_index,
    umkk_ScriptVerificationFlags flags,
    umkk_ScriptVerifyStatus* status) UMKOINKERNEL_ARG_NONNULL(1, 3);

/**
 * @brief Serializes the script pubkey through the passed in callback to bytes.
 *
 * @param[in] script_pubkey Non-null.
 * @param[in] writer        Non-null, callback to a write bytes function.
 * @param[in] user_data     Holds a user-defined opaque structure that will be
 *                          passed back through the writer callback.
 * @return                  0 on success.
 */
UMKOINKERNEL_API int umkk_script_pubkey_to_bytes(
    const umkk_ScriptPubkey* script_pubkey,
    umkk_WriteBytes writer,
    void* user_data) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the script pubkey.
 */
UMKOINKERNEL_API void umkk_script_pubkey_destroy(umkk_ScriptPubkey* script_pubkey);

///@}

/** @name TransactionOutput
 * Functions for working with transaction outputs.
 */
///@{

/**
 * @brief Create a transaction output from a script pubkey and an amount.
 *
 * @param[in] script_pubkey Non-null.
 * @param[in] amount        The amount associated with the script pubkey for this output.
 * @return                  The transaction output.
 */
UMKOINKERNEL_API umkk_TransactionOutput* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_output_create(
    const umkk_ScriptPubkey* script_pubkey,
    int64_t amount) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the script pubkey of the output. The returned
 * script pubkey is not owned and depends on the lifetime of the
 * transaction output.
 *
 * @param[in] transaction_output Non-null.
 * @return                       The script pubkey.
 */
UMKOINKERNEL_API const umkk_ScriptPubkey* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_output_get_script_pubkey(
    const umkk_TransactionOutput* transaction_output) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the amount in the output.
 *
 * @param[in] transaction_output Non-null.
 * @return                       The amount.
 */
UMKOINKERNEL_API int64_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_output_get_amount(
    const umkk_TransactionOutput* transaction_output) UMKOINKERNEL_ARG_NONNULL(1);

/**
 *  @brief Copy a transaction output.
 *
 *  @param[in] transaction_output Non-null.
 *  @return                       The copied transaction output.
 */
UMKOINKERNEL_API umkk_TransactionOutput* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_output_copy(
    const umkk_TransactionOutput* transaction_output) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction output.
 */
UMKOINKERNEL_API void umkk_transaction_output_destroy(umkk_TransactionOutput* transaction_output);

///@}

/** @name Logging
 * Logging-related functions.
 */
///@{

/**
 * @brief This disables the global internal logger. No log messages will be
 * buffered internally anymore once this is called and the buffer is cleared.
 * This function should only be called once and is not thread or re-entry safe.
 * Log messages will be buffered until this function is called, or a logging
 * connection is created. This must not be called while a logging connection
 * already exists.
 */
UMKOINKERNEL_API void umkk_logging_disable();

/**
 * @brief Set some options for the global internal logger. This changes global
 * settings and will override settings for all existing @ref
 * umkk_LoggingConnection instances.
 *
 * @param[in] options Sets formatting options of the log messages.
 */
UMKOINKERNEL_API void umkk_logging_set_options(const umkk_LoggingOptions options);

/**
 * @brief Set the log level of the global internal logger. This does not
 * enable the selected categories. Use @ref umkk_logging_enable_category to
 * start logging from a specific, or all categories. This changes a global
 * setting and will override settings for all existing
 * @ref umkk_LoggingConnection instances.
 *
 * @param[in] category If umkk_LogCategory_ALL is chosen, sets both the global fallback log level
 *                     used by all categories that don't have a specific level set, and also
 *                     sets the log level for messages logged with the umkk_LogCategory_ALL category itself.
 *                     For any other category, sets a category-specific log level that overrides
 *                     the global fallback for that category only.

 * @param[in] level    Log level at which the log category is set.
 */
UMKOINKERNEL_API void umkk_logging_set_level_category(umkk_LogCategory category, umkk_LogLevel level);

/**
 * @brief Enable a specific log category for the global internal logger. This
 * changes a global setting and will override settings for all existing @ref
 * umkk_LoggingConnection instances.
 *
 * @param[in] category If umkk_LogCategory_ALL is chosen, all categories will be enabled.
 */
UMKOINKERNEL_API void umkk_logging_enable_category(umkk_LogCategory category);

/**
 * @brief Disable a specific log category for the global internal logger. This
 * changes a global setting and will override settings for all existing @ref
 * umkk_LoggingConnection instances.
 *
 * @param[in] category If umkk_LogCategory_ALL is chosen, all categories will be disabled.
 */
UMKOINKERNEL_API void umkk_logging_disable_category(umkk_LogCategory category);

/**
 * @brief Start logging messages through the provided callback. Log messages
 * produced before this function is first called are buffered and on calling this
 * function are logged immediately.
 *
 * @param[in] log_callback               Non-null, function through which messages will be logged.
 * @param[in] user_data                  Nullable, holds a user-defined opaque structure. Is passed back
 *                                       to the user through the callback. If the user_data_destroy_callback
 *                                       is also defined it is assumed that ownership of the user_data is passed
 *                                       to the created logging connection.
 * @param[in] user_data_destroy_callback Nullable, function for freeing the user data.
 * @return                               A new kernel logging connection, or null on error.
 */
UMKOINKERNEL_API umkk_LoggingConnection* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_logging_connection_create(
    umkk_LogCallback log_callback,
    void* user_data,
    umkk_DestroyCallback user_data_destroy_callback) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Stop logging and destroy the logging connection.
 */
UMKOINKERNEL_API void umkk_logging_connection_destroy(umkk_LoggingConnection* logging_connection);

///@}

/** @name ChainParameters
 * Functions for working with chain parameters.
 */
///@{

/**
 * @brief Creates a chain parameters struct with default parameters based on the
 * passed in chain type.
 *
 * @param[in] chain_type Controls the chain parameters type created.
 * @return               An allocated chain parameters opaque struct.
 */
UMKOINKERNEL_API umkk_ChainParameters* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chain_parameters_create(
    const umkk_ChainType chain_type);

/**
 * Copy the chain parameters.
 */
UMKOINKERNEL_API umkk_ChainParameters* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chain_parameters_copy(
    const umkk_ChainParameters* chain_parameters) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the chain parameters.
 */
UMKOINKERNEL_API void umkk_chain_parameters_destroy(umkk_ChainParameters* chain_parameters);

///@}

/** @name ContextOptions
 * Functions for working with context options.
 */
///@{

/**
 * Creates an empty context options.
 */
UMKOINKERNEL_API umkk_ContextOptions* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_context_options_create();

/**
 * @brief Sets the chain params for the context options. The context created
 * with the options will be configured for these chain parameters.
 *
 * @param[in] context_options  Non-null, previously created by @ref umkk_context_options_create.
 * @param[in] chain_parameters Is set to the context options.
 */
UMKOINKERNEL_API void umkk_context_options_set_chainparams(
    umkk_ContextOptions* context_options,
    const umkk_ChainParameters* chain_parameters) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Set the kernel notifications for the context options. The context
 * created with the options will be configured with these notifications.
 *
 * @param[in] context_options Non-null, previously created by @ref umkk_context_options_create.
 * @param[in] notifications   Is set to the context options.
 */
UMKOINKERNEL_API void umkk_context_options_set_notifications(
    umkk_ContextOptions* context_options,
    umkk_NotificationInterfaceCallbacks notifications) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Set the validation interface callbacks for the context options. The
 * context created with the options will be configured for these validation
 * interface callbacks. The callbacks will then be triggered from validation
 * events issued by the chainstate manager created from the same context.
 *
 * @param[in] context_options                Non-null, previously created with umkk_context_options_create.
 * @param[in] validation_interface_callbacks The callbacks used for passing validation information to the
 *                                           user.
 */
UMKOINKERNEL_API void umkk_context_options_set_validation_interface(
    umkk_ContextOptions* context_options,
    umkk_ValidationInterfaceCallbacks validation_interface_callbacks) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the context options.
 */
UMKOINKERNEL_API void umkk_context_options_destroy(umkk_ContextOptions* context_options);

///@}

/** @name Context
 * Functions for working with contexts.
 */
///@{

/**
 * @brief Create a new kernel context. If the options have not been previously
 * set, their corresponding fields will be initialized to default values; the
 * context will assume mainnet chain parameters and won't attempt to call the
 * kernel notification callbacks.
 *
 * @param[in] context_options Nullable, created by @ref umkk_context_options_create.
 * @return                    The allocated context, or null on error.
 */
UMKOINKERNEL_API umkk_Context* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_context_create(
    const umkk_ContextOptions* context_options);

/**
 * Copy the context.
 */
UMKOINKERNEL_API umkk_Context* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_context_copy(
    const umkk_Context* context) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Interrupt can be used to halt long-running validation functions like
 * when reindexing, importing or processing blocks.
 *
 * @param[in] context  Non-null.
 * @return             0 if the interrupt was successful, non-zero otherwise.
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_context_interrupt(
    umkk_Context* context) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the context.
 */
UMKOINKERNEL_API void umkk_context_destroy(umkk_Context* context);

///@}

/** @name BlockTreeEntry
 * Functions for working with block tree entries.
 */
///@{

/**
 * @brief Returns the previous block tree entry in the tree, or null if the current
 * block tree entry is the genesis block.
 *
 * @param[in] block_tree_entry Non-null.
 * @return                     The previous block tree entry, or null on error or if the current block tree entry is the genesis block.
 */
UMKOINKERNEL_API const umkk_BlockTreeEntry* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_tree_entry_get_previous(
    const umkk_BlockTreeEntry* block_tree_entry) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Return the height of a certain block tree entry.
 *
 * @param[in] block_tree_entry Non-null.
 * @return                     The block height.
 */
UMKOINKERNEL_API int32_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_tree_entry_get_height(
    const umkk_BlockTreeEntry* block_tree_entry) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Return the block hash associated with a block tree entry.
 *
 * @param[in] block_tree_entry Non-null.
 * @return                     The block hash.
 */
UMKOINKERNEL_API const umkk_BlockHash* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_tree_entry_get_block_hash(
    const umkk_BlockTreeEntry* block_tree_entry) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Check if two block tree entries are equal. Two block tree entries are equal when they
 * point to the same block.
 *
 * @param[in] entry1 Non-null.
 * @param[in] entry2 Non-null.
 * @return           1 if the block tree entries are equal, 0 otherwise.
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_tree_entry_equals(
    const umkk_BlockTreeEntry* entry1, const umkk_BlockTreeEntry* entry2) UMKOINKERNEL_ARG_NONNULL(1, 2);

///@}

/** @name ChainstateManagerOptions
 * Functions for working with chainstate manager options.
 */
///@{

/**
 * @brief Create options for the chainstate manager.
 *
 * @param[in] context          Non-null, the created options and through it the chainstate manager will
 *                             associate with this kernel context for the duration of their lifetimes.
 * @param[in] data_directory   Non-null, non-empty path string of the directory containing the
 *                             chainstate data. If the directory does not exist yet, it will be
 *                             created.
 * @param[in] blocks_directory Non-null, non-empty path string of the directory containing the block
 *                             data. If the directory does not exist yet, it will be created.
 * @return                     The allocated chainstate manager options, or null on error.
 */
UMKOINKERNEL_API umkk_ChainstateManagerOptions* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chainstate_manager_options_create(
    const umkk_Context* context,
    const char* data_directory,
    size_t data_directory_len,
    const char* blocks_directory,
    size_t blocks_directory_len) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Set the number of available worker threads used during validation.
 *
 * @param[in] chainstate_manager_options Non-null, options to be set.
 * @param[in] worker_threads             The number of worker threads that should be spawned in the thread pool
 *                                       used for validation. When set to 0 no parallel verification is done.
 *                                       The value range is clamped internally between 0 and 15.
 */
UMKOINKERNEL_API void umkk_chainstate_manager_options_set_worker_threads_num(
    umkk_ChainstateManagerOptions* chainstate_manager_options,
    int worker_threads) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Sets wipe db in the options. In combination with calling
 * @ref umkk_chainstate_manager_import_blocks this triggers either a full reindex,
 * or a reindex of just the chainstate database.
 *
 * @param[in] chainstate_manager_options Non-null, created by @ref umkk_chainstate_manager_options_create.
 * @param[in] wipe_block_tree_db         Set wipe block tree db. Should only be 1 if wipe_chainstate_db is 1 too.
 * @param[in] wipe_chainstate_db         Set wipe chainstate db.
 * @return                               0 if the set was successful, non-zero if the set failed.
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chainstate_manager_options_set_wipe_dbs(
    umkk_ChainstateManagerOptions* chainstate_manager_options,
    int wipe_block_tree_db,
    int wipe_chainstate_db) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Sets block tree db in memory in the options.
 *
 * @param[in] chainstate_manager_options   Non-null, created by @ref umkk_chainstate_manager_options_create.
 * @param[in] block_tree_db_in_memory      Set block tree db in memory.
 */
UMKOINKERNEL_API void umkk_chainstate_manager_options_update_block_tree_db_in_memory(
    umkk_ChainstateManagerOptions* chainstate_manager_options,
    int block_tree_db_in_memory) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Sets chainstate db in memory in the options.
 *
 * @param[in] chainstate_manager_options Non-null, created by @ref umkk_chainstate_manager_options_create.
 * @param[in] chainstate_db_in_memory    Set chainstate db in memory.
 */
UMKOINKERNEL_API void umkk_chainstate_manager_options_update_chainstate_db_in_memory(
    umkk_ChainstateManagerOptions* chainstate_manager_options,
    int chainstate_db_in_memory) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the chainstate manager options.
 */
UMKOINKERNEL_API void umkk_chainstate_manager_options_destroy(umkk_ChainstateManagerOptions* chainstate_manager_options);

///@}

/** @name ChainstateManager
 * Functions for chainstate management.
 */
///@{

/**
 * @brief Create a chainstate manager. This is the main object for many
 * validation tasks as well as for retrieving data from the chain and
 * interacting with its chainstate and indexes.
 *
 * @param[in] chainstate_manager_options Non-null, created by @ref umkk_chainstate_manager_options_create.
 * @return                               The allocated chainstate manager, or null on error.
 */
UMKOINKERNEL_API umkk_ChainstateManager* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chainstate_manager_create(
    const umkk_ChainstateManagerOptions* chainstate_manager_options) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Triggers the start of a reindex if the wipe options were previously
 * set for the chainstate manager. Can also import an array of existing block
 * files selected by the user.
 *
 * @param[in] chainstate_manager        Non-null.
 * @param[in] block_file_paths_data     Nullable, array of block files described by their full filesystem paths.
 * @param[in] block_file_paths_lens     Nullable, array containing the lengths of each of the paths.
 * @param[in] block_file_paths_data_len Length of the block_file_paths_data and block_file_paths_len arrays.
 * @return                              0 if the import blocks call was completed successfully, non-zero otherwise.
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chainstate_manager_import_blocks(
    umkk_ChainstateManager* chainstate_manager,
    const char** block_file_paths_data, size_t* block_file_paths_lens,
    size_t block_file_paths_data_len) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Process and validate the passed in block with the chainstate
 * manager. Processing first does checks on the block, and if these passed,
 * saves it to disk. It then validates the block against the utxo set. If it is
 * valid, the chain is extended with it. The return value is not indicative of
 * the block's validity. Detailed information on the validity of the block can
 * be retrieved by registering the `block_checked` callback in the validation
 * interface.
 *
 * @param[in] chainstate_manager Non-null.
 * @param[in] block              Non-null, block to be validated.
 *
 * @param[out] new_block         Nullable, will be set to 1 if this block was not processed before. Note that this means it
 *                               might also not be 1 if processing was attempted before, but the block was found invalid
 *                               before its data was persisted.
 * @return                       0 if processing the block was successful. Will also return 0 for valid, but duplicate blocks.
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chainstate_manager_process_block(
    umkk_ChainstateManager* chainstate_manager,
    const umkk_Block* block,
    int* new_block) UMKOINKERNEL_ARG_NONNULL(1, 2, 3);

/**
 * @brief Returns the best known currently active chain. Its lifetime is
 * dependent on the chainstate manager. It can be thought of as a view on a
 * vector of block tree entries that form the best chain. The returned chain
 * reference always points to the currently active best chain. However, state
 * transitions within the chainstate manager (e.g., processing blocks) will
 * update the chain's contents. Data retrieved from this chain is only
 * consistent up to the point when new data is processed in the chainstate
 * manager. It is the user's responsibility to guard against these
 * inconsistencies.
 *
 * @param[in] chainstate_manager Non-null.
 * @return                       The chain.
 */
UMKOINKERNEL_API const umkk_Chain* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chainstate_manager_get_active_chain(
    const umkk_ChainstateManager* chainstate_manager) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Retrieve a block tree entry by its block hash.
 *
 * @param[in] chainstate_manager Non-null.
 * @param[in] block_hash         Non-null.
 * @return                       The block tree entry of the block with the passed in hash, or null if
 *                               the block hash is not found.
 */
UMKOINKERNEL_API const umkk_BlockTreeEntry* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chainstate_manager_get_block_tree_entry_by_hash(
    const umkk_ChainstateManager* chainstate_manager,
    const umkk_BlockHash* block_hash) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the chainstate manager.
 */
UMKOINKERNEL_API void umkk_chainstate_manager_destroy(umkk_ChainstateManager* chainstate_manager);

///@}

/** @name Block
 * Functions for working with blocks.
 */
///@{

/**
 * @brief Reads the block the passed in block tree entry points to from disk and
 * returns it.
 *
 * @param[in] chainstate_manager Non-null.
 * @param[in] block_tree_entry   Non-null.
 * @return                       The read out block, or null on error.
 */
UMKOINKERNEL_API umkk_Block* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_read(
    const umkk_ChainstateManager* chainstate_manager,
    const umkk_BlockTreeEntry* block_tree_entry) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Parse a serialized raw block into a new block object.
 *
 * @param[in] raw_block     Serialized block.
 * @param[in] raw_block_len Length of the serialized block.
 * @return                  The allocated block, or null on error.
 */
UMKOINKERNEL_API umkk_Block* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_create(
    const void* raw_block, size_t raw_block_len);

/**
 * @brief Copy a block. Blocks are reference counted, so this just increments
 * the reference count.
 *
 * @param[in] block Non-null.
 * @return          The copied block.
 */
UMKOINKERNEL_API umkk_Block* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_copy(
    const umkk_Block* block) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Count the number of transactions contained in a block.
 *
 * @param[in] block Non-null.
 * @return          The number of transactions in the block.
 */
UMKOINKERNEL_API size_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_count_transactions(
    const umkk_Block* block) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the transaction at the provided index. The returned transaction
 * is not owned and depends on the lifetime of the block.
 *
 * @param[in] block             Non-null.
 * @param[in] transaction_index The index of the transaction to be retrieved.
 * @return                      The transaction.
 */
UMKOINKERNEL_API const umkk_Transaction* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_get_transaction_at(
    const umkk_Block* block, size_t transaction_index) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Calculate and return the hash of a block.
 *
 * @param[in] block Non-null.
 * @return    The block hash.
 */
UMKOINKERNEL_API umkk_BlockHash* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_get_hash(
    const umkk_Block* block) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Serializes the block through the passed in callback to bytes.
 * This is consensus serialization that is also used for the P2P network.
 *
 * @param[in] block     Non-null.
 * @param[in] writer    Non-null, callback to a write bytes function.
 * @param[in] user_data Holds a user-defined opaque structure that will be
 *                      passed back through the writer callback.
 * @return              0 on success.
 */
UMKOINKERNEL_API int umkk_block_to_bytes(
    const umkk_Block* block,
    umkk_WriteBytes writer,
    void* user_data) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the block.
 */
UMKOINKERNEL_API void umkk_block_destroy(umkk_Block* block);

///@}

/** @name BlockValidationState
 * Functions for working with block validation states.
 */
///@{

/**
 * Returns the validation mode from an opaque block validation state pointer.
 */
UMKOINKERNEL_API umkk_ValidationMode umkk_block_validation_state_get_validation_mode(
    const umkk_BlockValidationState* block_validation_state) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Returns the validation result from an opaque block validation state pointer.
 */
UMKOINKERNEL_API umkk_BlockValidationResult umkk_block_validation_state_get_block_validation_result(
    const umkk_BlockValidationState* block_validation_state) UMKOINKERNEL_ARG_NONNULL(1);

///@}

/** @name Chain
 * Functions for working with the chain
 */
///@{

/**
 * @brief Return the height of the tip of the chain.
 *
 * @param[in] chain Non-null.
 * @return          The current height.
 */
UMKOINKERNEL_API int32_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chain_get_height(
    const umkk_Chain* chain) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Retrieve a block tree entry by its height in the currently active chain.
 * Once retrieved there is no guarantee that it remains in the active chain.
 *
 * @param[in] chain        Non-null.
 * @param[in] block_height Height in the chain of the to be retrieved block tree entry.
 * @return                 The block tree entry at a certain height in the currently active chain, or null
 *                         if the height is out of bounds.
 */
UMKOINKERNEL_API const umkk_BlockTreeEntry* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chain_get_by_height(
    const umkk_Chain* chain,
    int block_height) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Return true if the passed in chain contains the block tree entry.
 *
 * @param[in] chain            Non-null.
 * @param[in] block_tree_entry Non-null.
 * @return                     1 if the block_tree_entry is in the chain, 0 otherwise.
 *
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_chain_contains(
    const umkk_Chain* chain,
    const umkk_BlockTreeEntry* block_tree_entry) UMKOINKERNEL_ARG_NONNULL(1, 2);

///@}

/** @name BlockSpentOutputs
 * Functions for working with block spent outputs.
 */
///@{

/**
 * @brief Reads the block spent coins data the passed in block tree entry points to from
 * disk and returns it.
 *
 * @param[in] chainstate_manager Non-null.
 * @param[in] block_tree_entry   Non-null.
 * @return                       The read out block spent outputs, or null on error.
 */
UMKOINKERNEL_API umkk_BlockSpentOutputs* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_spent_outputs_read(
    const umkk_ChainstateManager* chainstate_manager,
    const umkk_BlockTreeEntry* block_tree_entry) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Copy a block's spent outputs.
 *
 * @param[in] block_spent_outputs Non-null.
 * @return                        The copied block spent outputs.
 */
UMKOINKERNEL_API umkk_BlockSpentOutputs* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_spent_outputs_copy(
    const umkk_BlockSpentOutputs* block_spent_outputs) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns the number of transaction spent outputs whose data is contained in
 * block spent outputs.
 *
 * @param[in] block_spent_outputs Non-null.
 * @return                        The number of transaction spent outputs data in the block spent outputs.
 */
UMKOINKERNEL_API size_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_spent_outputs_count(
    const umkk_BlockSpentOutputs* block_spent_outputs) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns a transaction spent outputs contained in the block spent
 * outputs at a certain index. The returned pointer is unowned and only valid
 * for the lifetime of block_spent_outputs.
 *
 * @param[in] block_spent_outputs             Non-null.
 * @param[in] transaction_spent_outputs_index The index of the transaction spent outputs within the block spent outputs.
 * @return                                    A transaction spent outputs pointer.
 */
UMKOINKERNEL_API const umkk_TransactionSpentOutputs* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_spent_outputs_get_transaction_spent_outputs_at(
    const umkk_BlockSpentOutputs* block_spent_outputs,
    size_t transaction_spent_outputs_index) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the block spent outputs.
 */
UMKOINKERNEL_API void umkk_block_spent_outputs_destroy(umkk_BlockSpentOutputs* block_spent_outputs);

///@}

/** @name TransactionSpentOutputs
 * Functions for working with the spent coins of a transaction
 */
///@{

/**
 * @brief Copy a transaction's spent outputs.
 *
 * @param[in] transaction_spent_outputs Non-null.
 * @return                              The copied transaction spent outputs.
 */
UMKOINKERNEL_API umkk_TransactionSpentOutputs* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_spent_outputs_copy(
    const umkk_TransactionSpentOutputs* transaction_spent_outputs) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns the number of previous transaction outputs contained in the
 * transaction spent outputs data.
 *
 * @param[in] transaction_spent_outputs Non-null
 * @return                              The number of spent transaction outputs for the transaction.
 */
UMKOINKERNEL_API size_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_spent_outputs_count(
    const umkk_TransactionSpentOutputs* transaction_spent_outputs) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns a coin contained in the transaction spent outputs at a
 * certain index. The returned pointer is unowned and only valid for the
 * lifetime of transaction_spent_outputs.
 *
 * @param[in] transaction_spent_outputs Non-null.
 * @param[in] coin_index                The index of the to be retrieved coin within the
 *                                      transaction spent outputs.
 * @return                              A coin pointer.
 */
UMKOINKERNEL_API const umkk_Coin* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_spent_outputs_get_coin_at(
    const umkk_TransactionSpentOutputs* transaction_spent_outputs,
    size_t coin_index) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction spent outputs.
 */
UMKOINKERNEL_API void umkk_transaction_spent_outputs_destroy(umkk_TransactionSpentOutputs* transaction_spent_outputs);

///@}

/** @name Transaction Input
 * Functions for working with transaction inputs.
 */
///@{

/**
 * @brief Copy a transaction input.
 *
 * @param[in] transaction_input Non-null.
 * @return                      The copied transaction input.
 */
UMKOINKERNEL_API umkk_TransactionInput* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_input_copy(
    const umkk_TransactionInput* transaction_input) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the transaction out point. The returned transaction out point is
 * not owned and depends on the lifetime of the transaction.
 *
 * @param[in] transaction_input Non-null.
 * @return                      The transaction out point.
 */
UMKOINKERNEL_API const umkk_TransactionOutPoint* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_input_get_out_point(
    const umkk_TransactionInput* transaction_input) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction input.
 */
UMKOINKERNEL_API void umkk_transaction_input_destroy(umkk_TransactionInput* transaction_input);

///@}

/** @name Transaction Out Point
 * Functions for working with transaction out points.
 */
///@{

/**
 * @brief Copy a transaction out point.
 *
 * @param[in] transaction_out_point Non-null.
 * @return                          The copied transaction out point.
 */
UMKOINKERNEL_API umkk_TransactionOutPoint* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_out_point_copy(
    const umkk_TransactionOutPoint* transaction_out_point) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the output position from the transaction out point.
 *
 * @param[in] transaction_out_point Non-null.
 * @return                          The output index.
 */
UMKOINKERNEL_API uint32_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_out_point_get_index(
    const umkk_TransactionOutPoint* transaction_out_point) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the txid from the transaction out point. The returned txid is
 * not owned and depends on the lifetime of the transaction out point.
 *
 * @param[in] transaction_out_point Non-null.
 * @return                          The txid.
 */
UMKOINKERNEL_API const umkk_Txid* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_transaction_out_point_get_txid(
    const umkk_TransactionOutPoint* transaction_out_point) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction out point.
 */
UMKOINKERNEL_API void umkk_transaction_out_point_destroy(umkk_TransactionOutPoint* transaction_out_point);

///@}

/** @name Txid
 * Functions for working with txids.
 */
///@{

/**
 * @brief Copy a txid.
 *
 * @param[in] txid Non-null.
 * @return         The copied txid.
 */
UMKOINKERNEL_API umkk_Txid* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_txid_copy(
    const umkk_Txid* txid) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Check if two txids are equal.
 *
 * @param[in] txid1 Non-null.
 * @param[in] txid2 Non-null.
 * @return          0 if the txid is not equal.
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_txid_equals(
    const umkk_Txid* txid1, const umkk_Txid* txid2) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Serializes the txid to bytes.
 *
 * @param[in] txid    Non-null.
 * @param[out] output The serialized txid.
 */
UMKOINKERNEL_API void umkk_txid_to_bytes(
    const umkk_Txid* txid, unsigned char output[32]) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the txid.
 */
UMKOINKERNEL_API void umkk_txid_destroy(umkk_Txid* txid);

///@}

///@}

/** @name Coin
 * Functions for working with coins.
 */
///@{

/**
 * @brief Copy a coin.
 *
 * @param[in] coin Non-null.
 * @return         The copied coin.
 */
UMKOINKERNEL_API umkk_Coin* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_coin_copy(
    const umkk_Coin* coin) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns the block height where the transaction that
 * created this coin was included in.
 *
 * @param[in] coin Non-null.
 * @return         The block height of the coin.
 */
UMKOINKERNEL_API uint32_t UMKOINKERNEL_WARN_UNUSED_RESULT umkk_coin_confirmation_height(
    const umkk_Coin* coin) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns whether the containing transaction was a coinbase.
 *
 * @param[in] coin Non-null.
 * @return         1 if the coin is a coinbase coin, 0 otherwise.
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_coin_is_coinbase(
    const umkk_Coin* coin) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Return the transaction output of a coin. The returned pointer is
 * unowned and only valid for the lifetime of the coin.
 *
 * @param[in] coin Non-null.
 * @return         A transaction output pointer.
 */
UMKOINKERNEL_API const umkk_TransactionOutput* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_coin_get_output(
    const umkk_Coin* coin) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * Destroy the coin.
 */
UMKOINKERNEL_API void umkk_coin_destroy(umkk_Coin* coin);

///@}

/** @name BlockHash
 * Functions for working with block hashes.
 */
///@{

/**
 * @brief Create a block hash from its raw data.
 */
UMKOINKERNEL_API umkk_BlockHash* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_hash_create(
    const unsigned char block_hash[32]) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Check if two block hashes are equal.
 *
 * @param[in] hash1 Non-null.
 * @param[in] hash2 Non-null.
 * @return          0 if the block hashes are not equal.
 */
UMKOINKERNEL_API int UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_hash_equals(
    const umkk_BlockHash* hash1, const umkk_BlockHash* hash2) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Copy a block hash.
 *
 * @param[in] block_hash Non-null.
 * @return               The copied block hash.
 */
UMKOINKERNEL_API umkk_BlockHash* UMKOINKERNEL_WARN_UNUSED_RESULT umkk_block_hash_copy(
    const umkk_BlockHash* block_hash) UMKOINKERNEL_ARG_NONNULL(1);

/**
 * @brief Serializes the block hash to bytes.
 *
 * @param[in] block_hash     Non-null.
 * @param[in] output         The serialized block hash.
 */
UMKOINKERNEL_API void umkk_block_hash_to_bytes(
    const umkk_BlockHash* block_hash, unsigned char output[32]) UMKOINKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the block hash.
 */
UMKOINKERNEL_API void umkk_block_hash_destroy(umkk_BlockHash* block_hash);

///@}

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // UMKOIN_KERNEL_UMKOINKERNEL_H
