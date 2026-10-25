cmake_minimum_required(VERSION 3.16)
project(centaur C)

include(GNUInstallDirs)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(centaur
  src/main.c
  src/install.c
  src/license.c
  src/list.c
  src/parse.c
  src/uninstall.c
  src/dependencies.c
  src/clean.c
  src/curl_hash.c
  src/sha256.c
  src/sync.c
  src/update.c
)

find_program(BWRAP_EXECUTABLE NAMES bwrap)
if (NOT BWRAP_EXECUTABLE)
    message(FATAL_ERROR "Bubblewrap (bwrap) is required to build and run centaur. It was not found.")
else()
    message(STATUS "Found bwrap: ${BWRAP_EXECUTABLE}")
endif()

find_package(CURL REQUIRED)

find_package(LibArchive REQUIRED)

target_link_libraries(centaur PRIVATE CURL::libcurl)
target_link_libraries(centaur PRIVATE LibArchive::LibArchive)

target_compile_definitions(centaur PRIVATE
    BWRAP_PATH="${BWRAP_EXECUTABLE}"
)

target_compile_options(centaur PRIVATE
  -Wall -Wextra -Wpedantic
)

option(CENTAUR_NATIVE "Enable -march=native" OFF)
if(CENTAUR_NATIVE)
  target_compile_options(centaur PRIVATE -march=native)
endif()

option(CENTAUR_O2 "Enable -O2" OFF)
if (CENTAUR_O2)
    target_compile_options(centaur PRIVATE -O2)
endif()

# Install stuff

install(TARGETS centaur
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY packages/
  DESTINATION /var/db/centaur/packages/scripts
)

install(DIRECTORY doc/
  DESTINATION /var/db/centaur/doc
)

install(CODE [[
  file(MAKE_DIRECTORY "/var/db/centaur/packages/installed")
  file(MAKE_DIRECTORY "/var/db/centaur/packages/manifests")
  file(MAKE_DIRECTORY "/var/db/centaur/tmp")
]])
