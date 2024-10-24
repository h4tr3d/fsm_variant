cmake_minimum_required(VERSION 3.16)

project(jtag LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(jtag main.cpp)
target_include_directories(jtag PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../..)

include(GNUInstallDirs)
install(TARGETS jtag
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
