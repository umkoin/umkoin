// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UMKOIN_SCRIPT_UMKOINCONSENSUS_H
#define UMKOIN_SCRIPT_UMKOINCONSENSUS_H

#include <stdint.h>

#if defined(BUILD_UMKOIN_INTERNAL) && defined(HAVE_CONFIG_H)
#include <config/umkoin-config.h>
  #if defined(_WIN32)
    #if defined(HAVE_DLLEXPORT_ATTRIBUTE)
      #define EXPORT_SYMBOL __declspec(dllexport)
    #else
      #define EXPORT_SYMBOL
    #endif
  #elif defined(HAVE_DEFAULT_VISIBILITY_ATTRIBUTE)
    #define EXPORT_SYMBOL __attribute__ ((visibility ("default")))
  #endif
#elif defined(MSC_VER) && !defined(STATIC_LIBUMKOINCONSENSUS)
  #define EXPORT_SYMBOL __declspec(dllimport)
#endif

#ifndef EXPORT_SYMBOL
  #define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define UMKOINCONSENSUS_API_VER 2

typedef enum umkoinconsensus_error_t
{
    umkoinconsensus_ERR_OK = 0,
    umkoinconsensus_ERR_TX_INDEX,
    umkoinconsensus_ERR_TX_SIZE_MISMATCH,
    umkoinconsensus_ERR_TX_DESERIALIZE,
    umkoinconsensus_ERR_AMOUNT_REQUIRED,
    umkoinconsensus_ERR_INVALID_FLAGS,
    umkoinconsensus_ERR_SPENT_OUTPUTS_REQUIRED,
    umkoinconsensus_ERR_SPENT_OUTPUTS_MISMATCH
} umkoinconsensus_error;

/** Script verification flags */
enum
{
    umkoinconsensus_SCRIPT_FLAGS_VERIFY_NONE                = 0,
    umkoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH                = (1U << 0), // evaluate P2SH (BIP16) subscripts
    umkoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG              = (1U << 2), // enforce strict DER (BIP66) compliance
    umkoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY           = (1U << 4), // enforce NULLDUMMY (BIP147)
    umkoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9), // enable CHECKLOCKTIMEVERIFY (BIP65)
    umkoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10), // enable CHECKSEQUENCEVERIFY (BIP112)
    umkoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS             = (1U << 11), // enable WITNESS (BIP141)
    umkoinconsensus_SCRIPT_FLAGS_VERIFY_TAPROOT             = (1U << 17), // enable TAPROOT (BIPs 341 & 342)
    umkoinconsensus_SCRIPT_FLAGS_VERIFY_ALL                 = umkoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH | umkoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
                                                               umkoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY | umkoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY |
                                                               umkoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY | umkoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS |
                                                               umkoinconsensus_SCRIPT_FLAGS_VERIFY_TAPROOT
};

typedef struct {
    const unsigned char *scriptPubKey;
    unsigned int scriptPubKeySize;
    int64_t value;
} UTXO;

/// Returns 1 if the input nIn of the serialized transaction pointed to by
/// txTo correctly spends the scriptPubKey pointed to by scriptPubKey under
/// the additional constraints specified by flags.
/// If not nullptr, err will contain an error/success code for the operation
EXPORT_SYMBOL int umkoinconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                                 const unsigned char *txTo        , unsigned int txToLen,
                                                 unsigned int nIn, unsigned int flags, umkoinconsensus_error* err);

EXPORT_SYMBOL int umkoinconsensus_verify_script_with_amount(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, umkoinconsensus_error* err);

EXPORT_SYMBOL int umkoinconsensus_verify_script_with_spent_outputs(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    const UTXO *spentOutputs, unsigned int spentOutputsLen,
                                    unsigned int nIn, unsigned int flags, umkoinconsensus_error* err);

EXPORT_SYMBOL unsigned int umkoinconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif // UMKOIN_SCRIPT_UMKOINCONSENSUS_H
