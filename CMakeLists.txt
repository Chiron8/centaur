cmake_minimum_required(VERSION 3.16)
set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix" FORCE)

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
    src/dependencies.c)

target_compile_options(centaur PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

install(TARGETS centaur DESTINATION /usr/bin/)
install(DIRECTORY packages/ DESTINATION /etc/centaur/packages/scripts)
install(DIRECTORY doc/ DESTINATION /etc/centaur/doc)
install(DIRECTORY DESTINATION /etc/centaur/packages/installed)
install(DIRECTORY DESTINATION /etc/centaur/packages/uninstall)
install(DIRECTORY DESTINATION /tmp/centaur)
