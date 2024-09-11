# Copyright (c) 2024-present The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

add_executable(test_umkoin-qt
  apptests.cpp
  optiontests.cpp
  rpcnestedtests.cpp
  test_main.cpp
  uritests.cpp
  util.cpp
  ../../init/umkoin-qt.cpp
)

target_link_libraries(test_umkoin-qt
  core_interface
  umkoinqt
  test_util
  umkoin_node
  Boost::headers
  Qt5::Test
)

import_plugins(test_umkoin-qt)

if(ENABLE_WALLET)
  target_sources(test_umkoin-qt
    PRIVATE
      addressbooktests.cpp
      wallettests.cpp
      ../../wallet/test/wallet_test_fixture.cpp
  )
endif()

if(NOT QT_IS_STATIC)
  add_custom_command(
    TARGET test_umkoin-qt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_PROPERTY:Qt5::QMinimalIntegrationPlugin,LOCATION_$<UPPER_CASE:$<CONFIG>>> $<TARGET_FILE_DIR:test_umkoin-qt>/plugins/platforms
    VERBATIM
  )
endif()

add_test(NAME test_umkoin-qt
  COMMAND test_umkoin-qt
)
if(WIN32 AND VCPKG_TARGET_TRIPLET)
  # On Windows, vcpkg configures Qt with `-opengl dynamic`, which makes
  # the "minimal" platform plugin unusable due to internal Qt bugs.
  set_tests_properties(test_umkoin-qt PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=windows"
  )
endif()

install(TARGETS test_umkoin-qt
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
