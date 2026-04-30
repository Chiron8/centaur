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
)

target_compile_options(centaur PRIVATE
  -Wall -Wextra -Wpedantic
  $<$<CONFIG:Release>:-O2>
)

option(CENTAUR_NATIVE "Enable -march=native" OFF)
if(CENTAUR_NATIVE)
  target_compile_options(centaur PRIVATE -march=native)
endif()

install(TARGETS centaur
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY packages/
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/centaur/packages/scripts
)

install(DIRECTORY doc/
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/centaur/doc
)

# Consider removing /tmp handling; shown here only if you truly need it:
install(CODE [[
  file(MAKE_DIRECTORY "$ENV{DESTDIR}${CMAKE_INSTALL_FULL_SYSCONFDIR}/centaur/packages/installed")
  file(MAKE_DIRECTORY "$ENV{DESTDIR}${CMAKE_INSTALL_FULL_SYSCONFDIR}/centaur/packages/uninstall")
]])
