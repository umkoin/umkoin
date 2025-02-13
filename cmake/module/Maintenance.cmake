# Copyright (c) 2023-present The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

include_guard(GLOBAL)

function(setup_split_debug_script)
  if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(OBJCOPY ${CMAKE_OBJCOPY})
    set(STRIP ${CMAKE_STRIP})
    configure_file(
      contrib/devtools/split-debug.sh.in split-debug.sh
      FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
                       GROUP_READ GROUP_EXECUTE
                       WORLD_READ
      @ONLY
    )
  endif()
endfunction()

function(add_maintenance_targets)
  if(NOT PYTHON_COMMAND)
    return()
  endif()

  foreach(target IN ITEMS umkoind umkoin-qt umkoin-cli umkoin-tx umkoin-util umkoin-wallet test_umkoin bench_umkoin)
    if(TARGET ${target})
      list(APPEND executables $<TARGET_FILE:${target}>)
    endif()
  endforeach()

  add_custom_target(check-symbols
    COMMAND ${CMAKE_COMMAND} -E echo "Running symbol and dynamic library checks..."
    COMMAND ${PYTHON_COMMAND} ${PROJECT_SOURCE_DIR}/contrib/devtools/symbol-check.py ${executables}
    VERBATIM
  )

  if(ENABLE_HARDENING)
    add_custom_target(check-security
      COMMAND ${CMAKE_COMMAND} -E echo "Checking binary security..."
      COMMAND ${PYTHON_COMMAND} ${PROJECT_SOURCE_DIR}/contrib/devtools/security-check.py ${executables}
      VERBATIM
    )
  else()
    add_custom_target(check-security)
  endif()
endfunction()

function(add_windows_deploy_target)
  if(MINGW AND TARGET umkoin-qt AND TARGET umkoind AND TARGET umkoin-cli AND TARGET umkoin-tx AND TARGET umkoin-wallet AND TARGET umkoin-util AND TARGET test_umkoin)
    # TODO: Consider replacing this code with the CPack NSIS Generator.
    #       See https://cmake.org/cmake/help/latest/cpack_gen/nsis.html
    include(GenerateSetupNsi)
    generate_setup_nsi()
    add_custom_command(
      OUTPUT ${PROJECT_BINARY_DIR}/umkoin-win64-setup.exe
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/release
      COMMAND ${CMAKE_STRIP} $<TARGET_FILE:umkoin-qt> -o ${PROJECT_BINARY_DIR}/release/$<TARGET_FILE_NAME:umkoin-qt>
      COMMAND ${CMAKE_STRIP} $<TARGET_FILE:umkoind> -o ${PROJECT_BINARY_DIR}/release/$<TARGET_FILE_NAME:umkoind>
      COMMAND ${CMAKE_STRIP} $<TARGET_FILE:umkoin-cli> -o ${PROJECT_BINARY_DIR}/release/$<TARGET_FILE_NAME:umkoin-cli>
      COMMAND ${CMAKE_STRIP} $<TARGET_FILE:umkoin-tx> -o ${PROJECT_BINARY_DIR}/release/$<TARGET_FILE_NAME:umkoin-tx>
      COMMAND ${CMAKE_STRIP} $<TARGET_FILE:umkoin-wallet> -o ${PROJECT_BINARY_DIR}/release/$<TARGET_FILE_NAME:umkoin-wallet>
      COMMAND ${CMAKE_STRIP} $<TARGET_FILE:umkoin-util> -o ${PROJECT_BINARY_DIR}/release/$<TARGET_FILE_NAME:umkoin-util>
      COMMAND ${CMAKE_STRIP} $<TARGET_FILE:test_umkoin> -o ${PROJECT_BINARY_DIR}/release/$<TARGET_FILE_NAME:test_umkoin>
      COMMAND makensis -V2 ${PROJECT_BINARY_DIR}/umkoin-win64-setup.nsi
      VERBATIM
    )
    add_custom_target(deploy DEPENDS ${PROJECT_BINARY_DIR}/umkoin-win64-setup.exe)
  endif()
endfunction()

function(add_macos_deploy_target)
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND TARGET umkoin-qt)
    set(macos_app "Umkoin-Qt.app")
    # Populate Contents subdirectory.
    configure_file(${PROJECT_SOURCE_DIR}/share/qt/Info.plist.in ${macos_app}/Contents/Info.plist NO_SOURCE_PERMISSIONS)
    file(CONFIGURE OUTPUT ${macos_app}/Contents/PkgInfo CONTENT "APPL????")
    # Populate Contents/Resources subdirectory.
    file(CONFIGURE OUTPUT ${macos_app}/Contents/Resources/empty.lproj CONTENT "")
    configure_file(${PROJECT_SOURCE_DIR}/src/qt/res/icons/umkoin.icns ${macos_app}/Contents/Resources/umkoin.icns NO_SOURCE_PERMISSIONS COPYONLY)
    file(CONFIGURE OUTPUT ${macos_app}/Contents/Resources/Base.lproj/InfoPlist.strings
      CONTENT "{ CFBundleDisplayName = \"@CLIENT_NAME@\"; CFBundleName = \"@CLIENT_NAME@\"; }"
    )

    add_custom_command(
      OUTPUT ${PROJECT_BINARY_DIR}/${macos_app}/Contents/MacOS/Umkoin-Qt
      COMMAND ${CMAKE_COMMAND} --install ${PROJECT_BINARY_DIR} --config $<CONFIG> --component GUI --prefix ${macos_app}/Contents/MacOS --strip
      COMMAND ${CMAKE_COMMAND} -E rename ${macos_app}/Contents/MacOS/bin/$<TARGET_FILE_NAME:umkoin-qt> ${macos_app}/Contents/MacOS/Umkoin-Qt
      COMMAND ${CMAKE_COMMAND} -E rm -rf ${macos_app}/Contents/MacOS/bin
      VERBATIM
    )

    string(REPLACE " " "-" osx_volname ${CLIENT_NAME})
    if(CMAKE_HOST_APPLE)
      add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/${osx_volname}.zip
        COMMAND ${PYTHON_COMMAND} ${PROJECT_SOURCE_DIR}/contrib/macdeploy/macdeployqtplus ${macos_app} ${osx_volname} -translations-dir=${QT_TRANSLATIONS_DIR} -zip
        DEPENDS ${PROJECT_BINARY_DIR}/${macos_app}/Contents/MacOS/Umkoin-Qt
        VERBATIM
      )
      add_custom_target(deploydir
        DEPENDS ${PROJECT_BINARY_DIR}/${osx_volname}.zip
      )
      add_custom_target(deploy
        DEPENDS ${PROJECT_BINARY_DIR}/${osx_volname}.zip
      )
    else()
      add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/dist/${macos_app}/Contents/MacOS/Umkoin-Qt
        COMMAND OBJDUMP=${CMAKE_OBJDUMP} ${PYTHON_COMMAND} ${PROJECT_SOURCE_DIR}/contrib/macdeploy/macdeployqtplus ${macos_app} ${osx_volname} -translations-dir=${QT_TRANSLATIONS_DIR}
        DEPENDS ${PROJECT_BINARY_DIR}/${macos_app}/Contents/MacOS/Umkoin-Qt
        VERBATIM
      )
      add_custom_target(deploydir
        DEPENDS ${PROJECT_BINARY_DIR}/dist/${macos_app}/Contents/MacOS/Umkoin-Qt
      )

      find_program(ZIP_COMMAND zip REQUIRED)
      add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/dist/${osx_volname}.zip
        WORKING_DIRECTORY dist
        COMMAND ${PROJECT_SOURCE_DIR}/cmake/script/macos_zip.sh ${ZIP_COMMAND} ${osx_volname}.zip
        VERBATIM
      )
      add_custom_target(deploy
        DEPENDS ${PROJECT_BINARY_DIR}/dist/${osx_volname}.zip
      )
    endif()
    add_dependencies(deploydir umkoin-qt)
    add_dependencies(deploy deploydir)
  endif()
endfunction()
