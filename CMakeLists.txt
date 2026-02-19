include(GNUInstallDirs)
cmake_minimum_required(VERSION 3.16)

project(centaur C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(centaur main.c)

target_compile_options(centaur PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

install(TARGETS centaur DESTINATION bin)
install(DIRECTORY install_scripts/ DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/centaur/packages/scripts/)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/centaur/packages/installed)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/centaur/packages/uninstall)
