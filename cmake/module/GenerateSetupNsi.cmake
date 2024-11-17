# Copyright (c) 2023-present The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

function(generate_setup_nsi)
  set(abs_top_srcdir ${PROJECT_SOURCE_DIR})
  set(abs_top_builddir ${PROJECT_BINARY_DIR})
  set(PACKAGE_URL ${PROJECT_HOMEPAGE_URL})
  set(PACKAGE_TARNAME "umkoin")
  set(UMKOIN_GUI_NAME "umkoin-qt")
  set(UMKOIN_DAEMON_NAME "umkoind")
  set(UMKOIN_CLI_NAME "umkoin-cli")
  set(UMKOIN_TX_NAME "umkoin-tx")
  set(UMKOIN_WALLET_TOOL_NAME "umkoin-wallet")
  set(UMKOIN_TEST_NAME "test_umkoin")
  set(EXEEXT ${CMAKE_EXECUTABLE_SUFFIX})
  configure_file(${PROJECT_SOURCE_DIR}/share/setup.nsi.in ${PROJECT_BINARY_DIR}/umkoin-win64-setup.nsi @ONLY)
endfunction()
