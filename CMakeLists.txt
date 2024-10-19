cmake_minimum_required(VERSION 3.16)

project(FsmTest_variant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FsmTest_variant main.cpp
    fsm_variant.hpp)

include(GNUInstallDirs)
install(TARGETS FsmTest_variant
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
