// Copyright (c) 2023 Umkoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "logprintf.h"

#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

class UmkoinModule final : public clang::tidy::ClangTidyModule
{
public:
    void addCheckFactories(clang::tidy::ClangTidyCheckFactories& CheckFactories) override
    {
        CheckFactories.registerCheck<umkoin::LogPrintfCheck>("umkoin-unterminated-logprintf");
    }
};

static clang::tidy::ClangTidyModuleRegistry::Add<UmkoinModule>
    X("umkoin-module", "Adds umkoin checks.");

volatile int UmkoinModuleAnchorSource = 0;
