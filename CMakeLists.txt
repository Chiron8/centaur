cmake_minimum_required(VERSION 3.16)
set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix" FORCE)

project(centaur C)
include(GNUInstallDirs)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(centaur main.c)

target_compile_options(centaur PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

install(TARGETS centaur DESTINATION /usr/bin/)
install(DIRECTORY install_scripts/ DESTINATION /etc/centaur/packages/scripts)
install(DIRECTORY doc/ DESTINATION /etc/centaur/doc)
install(DIRECTORY DESTINATION /etc/centaur/packages/installed)
install(DIRECTORY DESTINATION /etc/centaur/packages/uninstall)
install(DIRECTORY DESTINATION /tmp/centaur)
