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
  src/curl.c
  src/sha256.c
)

find_package(CURL REQUIRED)
target_link_libraries(centaur PRIVATE CURL::libcurl)

target_compile_options(centaur PRIVATE
  -Wall -Wextra -Wpedantic
  $<$<CONFIG:Release>:-O2>
)

option(CENTAUR_NATIVE "Enable -march=native" OFF)
if(CENTAUR_NATIVE)
  target_compile_options(centaur PRIVATE -march=native)
endif()

# Install stuff

install(TARGETS centaur
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY packages/
  DESTINATION /etc/centaur/packages/scripts
)

install(DIRECTORY doc/
  DESTINATION /etc/centaur/doc
)

file(WRITE "${CMAKE_BINARY_DIR}/centaur-tmpfiles.conf"
"d /tmp/centaur 0755 root root -\n")

install(FILES "${CMAKE_BINARY_DIR}/centaur-tmpfiles.conf"
        DESTINATION /usr/lib/tmpfiles.d
)

install(CODE [[
  file(MAKE_DIRECTORY "/etc/centaur/packages/installed")
  file(MAKE_DIRECTORY "/etc/centaur/packages/uninstall")
  file(MAKE_DIRECTORY "/etc/centaur/packages/world")
  file(MAKE_DIRECTORY "/etc/centaur/tmp")
]])
