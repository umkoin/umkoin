# Copyright (c) 2023-present The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  enable_language(OBJCXX)
  set(CMAKE_OBJCXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_OBJCXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_OBJCXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_OBJCXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
  string(APPEND CMAKE_OBJCXX_COMPILE_OBJECT " ${APPEND_CPPFLAGS} ${APPEND_CXXFLAGS}")
endif()

get_target_property(qt_lib_type Qt6::Core TYPE)

function(import_plugins target)
  if(qt_lib_type STREQUAL "STATIC_LIBRARY")
    set(plugins Qt6::QMinimalIntegrationPlugin)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      list(APPEND plugins Qt6::QXcbIntegrationPlugin)
    elseif(WIN32)
      list(APPEND plugins Qt6::QWindowsIntegrationPlugin Qt6::QModernWindowsStylePlugin)
    elseif(APPLE)
      list(APPEND plugins Qt6::QCocoaIntegrationPlugin Qt6::QMacStylePlugin)
    endif()
    qt6_import_plugins(${target}
      INCLUDE ${plugins}
      EXCLUDE_BY_TYPE
        accessiblebridge
        platforms
        platforms_darwin
        xcbglintegrations
        platformthemes
        platforminputcontexts
        generic
        iconengines
        imageformats
        egldeviceintegrations
        styles
        networkaccess
        networkinformation
        tls
    )
  endif()
endfunction()

# For Qt-specific commands and variables, please consult:
#  - https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html
#  - https://doc.qt.io/qt-5/cmake-manual.html

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS "-p${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS forms)

configure_file(umkoin_locale.qrc umkoin_locale.qrc USE_SOURCE_PERMISSIONS COPYONLY)

# The umkoinqt sources have to include headers in
# order to parse them to collect translatable strings.
add_library(umkoinqt STATIC EXCLUDE_FROM_ALL
  bantablemodel.cpp
  bantablemodel.h
  umkoin.cpp
  umkoin.h
  umkoinaddressvalidator.cpp
  umkoinaddressvalidator.h
  umkoinamountfield.cpp
  umkoinamountfield.h
  umkoingui.cpp
  umkoingui.h
  umkoinunits.cpp
  umkoinunits.h
  clientmodel.cpp
  clientmodel.h
  csvmodelwriter.cpp
  csvmodelwriter.h
  guiutil.cpp
  guiutil.h
  initexecutor.cpp
  initexecutor.h
  intro.cpp
  intro.h
  $<$<PLATFORM_ID:Darwin>:macdockiconhandler.h>
  $<$<PLATFORM_ID:Darwin>:macdockiconhandler.mm>
  $<$<PLATFORM_ID:Darwin>:macnotificationhandler.h>
  $<$<PLATFORM_ID:Darwin>:macnotificationhandler.mm>
  $<$<PLATFORM_ID:Darwin>:macos_appnap.h>
  $<$<PLATFORM_ID:Darwin>:macos_appnap.mm>
  modaloverlay.cpp
  modaloverlay.h
  networkstyle.cpp
  networkstyle.h
  notificator.cpp
  notificator.h
  optionsdialog.cpp
  optionsdialog.h
  optionsmodel.cpp
  optionsmodel.h
  peertablemodel.cpp
  peertablemodel.h
  peertablesortproxy.cpp
  peertablesortproxy.h
  platformstyle.cpp
  platformstyle.h
  qvalidatedlineedit.cpp
  qvalidatedlineedit.h
  qvaluecombobox.cpp
  qvaluecombobox.h
  rpcconsole.cpp
  rpcconsole.h
  splashscreen.cpp
  splashscreen.h
  trafficgraphwidget.cpp
  trafficgraphwidget.h
  utilitydialog.cpp
  utilitydialog.h
  $<$<PLATFORM_ID:Windows>:winshutdownmonitor.cpp>
  $<$<PLATFORM_ID:Windows>:winshutdownmonitor.h>
  umkoin.qrc
  ${CMAKE_CURRENT_BINARY_DIR}/umkoin_locale.qrc
)
target_compile_definitions(umkoinqt
  PUBLIC
    QT_NO_KEYWORDS
    QT_USE_QSTRINGBUILDER
)
target_include_directories(umkoinqt
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)
set_property(SOURCE macnotificationhandler.mm
  # Ignore warnings "'NSUserNotificationCenter' is deprecated: first deprecated in macOS 11.0".
  APPEND PROPERTY COMPILE_OPTIONS -Wno-deprecated-declarations
)
target_link_libraries(umkoinqt
  PUBLIC
    Qt6::Widgets
  PRIVATE
    core_interface
    umkoin_cli
    leveldb
    Boost::headers
    $<TARGET_NAME_IF_EXISTS:QRencode::QRencode>
    $<$<PLATFORM_ID:Darwin>:-framework\ AppKit>
    $<$<CXX_COMPILER_ID:MSVC>:shlwapi>
)

if(ENABLE_WALLET)
  target_sources(umkoinqt
    PRIVATE
      addressbookpage.cpp
      addressbookpage.h
      addresstablemodel.cpp
      addresstablemodel.h
      askpassphrasedialog.cpp
      askpassphrasedialog.h
      coincontroldialog.cpp
      coincontroldialog.h
      coincontroltreewidget.cpp
      coincontroltreewidget.h
      createwalletdialog.cpp
      createwalletdialog.h
      editaddressdialog.cpp
      editaddressdialog.h
      openuridialog.cpp
      openuridialog.h
      overviewpage.cpp
      overviewpage.h
      paymentserver.cpp
      paymentserver.h
      psbtoperationsdialog.cpp
      psbtoperationsdialog.h
      qrimagewidget.cpp
      qrimagewidget.h
      receivecoinsdialog.cpp
      receivecoinsdialog.h
      receiverequestdialog.cpp
      receiverequestdialog.h
      recentrequeststablemodel.cpp
      recentrequeststablemodel.h
      sendcoinsdialog.cpp
      sendcoinsdialog.h
      sendcoinsentry.cpp
      sendcoinsentry.h
      signverifymessagedialog.cpp
      signverifymessagedialog.h
      transactiondesc.cpp
      transactiondesc.h
      transactiondescdialog.cpp
      transactiondescdialog.h
      transactionfilterproxy.cpp
      transactionfilterproxy.h
      transactionoverviewwidget.cpp
      transactionoverviewwidget.h
      transactionrecord.cpp
      transactionrecord.h
      transactiontablemodel.cpp
      transactiontablemodel.h
      transactionview.cpp
      transactionview.h
      walletcontroller.cpp
      walletcontroller.h
      walletframe.cpp
      walletframe.h
      walletmodel.cpp
      walletmodel.h
      walletmodeltransaction.cpp
      walletmodeltransaction.h
      walletview.cpp
      walletview.h
  )
  target_link_libraries(umkoinqt
    PRIVATE
      umkoin_wallet
      Qt6::Network
  )
endif()

if(WITH_DBUS)
  target_link_libraries(umkoinqt PRIVATE Qt6::DBus)
endif()

if(qt_lib_type STREQUAL "STATIC_LIBRARY")
  # We want to define static plugins to link ourselves, thus preventing
  # automatic linking against a "sane" set of default static plugins.
  qt6_import_plugins(umkoinqt
    EXCLUDE_BY_TYPE
      accessiblebridge
      platforms
      platforms_darwin
      xcbglintegrations
      platformthemes
      platforminputcontexts
      generic
      iconengines
      imageformats
      egldeviceintegrations
      styles
      networkaccess
      networkinformation
      tls
  )
endif()

file(GLOB ts_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} locale/*.ts)
set_source_files_properties(${ts_files} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/locale)
qt6_add_lrelease(umkoinqt
  TS_FILES ${ts_files}
  OPTIONS -silent
)

add_executable(umkoin-qt
  main.cpp
  ../init/umkoin-qt.cpp
)

add_windows_resources(umkoin-qt res/umkoin-qt-res.rc)
add_windows_application_manifest(umkoin-qt)

target_link_libraries(umkoin-qt
  core_interface
  umkoinqt
  umkoin_node
)

import_plugins(umkoin-qt)
install_binary_component(umkoin-qt HAS_MANPAGE)
if(WIN32)
  set_target_properties(umkoin-qt PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(ENABLE_IPC)
  add_executable(umkoin-gui
    main.cpp
    ../init/umkoin-gui.cpp
  )
  target_link_libraries(umkoin-gui
    core_interface
    umkoinqt
    umkoin_node
    umkoin_ipc
  )
  import_plugins(umkoin-gui)
  install_binary_component(umkoin-gui)
  if(WIN32)
    set_target_properties(umkoin-gui PROPERTIES WIN32_EXECUTABLE TRUE)
  endif()
endif()

if(BUILD_GUI_TESTS)
  add_subdirectory(test)
endif()


# Gets sources to be parsed to gather translatable strings.
function(get_translatable_sources var)
  set(result)
  set(targets)
  foreach(dir IN ITEMS ${ARGN})
    get_directory_property(dir_targets DIRECTORY ${PROJECT_SOURCE_DIR}/${dir} BUILDSYSTEM_TARGETS)
    list(APPEND targets ${dir_targets})
  endforeach()
  foreach(target IN LISTS targets)
    get_target_property(target_sources ${target} SOURCES)
    if(target_sources)
      foreach(source IN LISTS target_sources)
        # Get an expression from the generator expression, if any.
        if(source MATCHES ":([^>]+)>$")
          set(source ${CMAKE_MATCH_1})
        endif()
        cmake_path(GET source EXTENSION LAST_ONLY ext)
        if(ext STREQUAL ".qrc")
          continue()
        endif()
        if(NOT IS_ABSOLUTE source)
          get_target_property(target_source_dir ${target} SOURCE_DIR)
          cmake_path(APPEND target_source_dir ${source} OUTPUT_VARIABLE source)
        endif()
        get_property(is_generated
          SOURCE  ${source} TARGET_DIRECTORY ${target}
          PROPERTY GENERATED
        )
        if(NOT is_generated)
          list(APPEND result ${source})
        endif()
      endforeach()
    endif()
  endforeach()
  set(${var} ${result} PARENT_SCOPE)
endfunction()

find_program(XGETTEXT_EXECUTABLE xgettext)
find_program(SED_EXECUTABLE sed)
if(NOT XGETTEXT_EXECUTABLE)
  add_custom_target(translate
    COMMAND ${CMAKE_COMMAND} -E echo "Error: GNU gettext-tools not found"
  )
elseif(NOT SED_EXECUTABLE)
  add_custom_target(translate
    COMMAND ${CMAKE_COMMAND} -E echo "Error: GNU sed not found"
  )
else()
  set(translatable_sources_directories src src/qt src/util)
  if(ENABLE_WALLET)
    list(APPEND translatable_sources_directories src/wallet)
  endif()
  get_translatable_sources(translatable_sources ${translatable_sources_directories})
  get_translatable_sources(qt_translatable_sources src/qt)
  file(GLOB ui_files ${CMAKE_CURRENT_SOURCE_DIR}/forms/*.ui)
  add_custom_target(translate
    COMMAND ${CMAKE_COMMAND} -E env XGETTEXT=${XGETTEXT_EXECUTABLE} COPYRIGHT_HOLDERS=${COPYRIGHT_HOLDERS} ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/share/qt/extract_strings_qt.py ${translatable_sources}
    COMMAND Qt6::lupdate -no-obsolete -I ${PROJECT_SOURCE_DIR}/src -locations relative ${CMAKE_CURRENT_SOURCE_DIR}/umkoinstrings.cpp ${ui_files} ${qt_translatable_sources} -ts ${CMAKE_CURRENT_SOURCE_DIR}/locale/umkoin_en.ts
    COMMAND Qt6::lconvert -drop-translations -o ${CMAKE_CURRENT_SOURCE_DIR}/locale/umkoin_en.xlf -i ${CMAKE_CURRENT_SOURCE_DIR}/locale/umkoin_en.ts
    COMMAND ${SED_EXECUTABLE} -i.old -e "s|source-language=\"en\" target-language=\"en\"|source-language=\"en\"|" -e "/<target xml:space=\"preserve\"><\\/target>/d" ${CMAKE_CURRENT_SOURCE_DIR}/locale/umkoin_en.xlf
    COMMAND ${CMAKE_COMMAND} -E rm ${CMAKE_CURRENT_SOURCE_DIR}/locale/umkoin_en.xlf.old
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src
    VERBATIM
  )
endif()
