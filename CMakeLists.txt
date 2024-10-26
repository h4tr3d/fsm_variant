cmake_minimum_required(VERSION 3.16)

project(FsmTest_variant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Warnings
set (VFSM_WARNING_OPTIONS
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
       -Wall -Wextra -Werror=return-type
       -Wwrite-strings -Wduplicated-cond -Wduplicated-branches -Wlogical-op
       -Wrestrict -Wnull-dereference -Wold-style-cast -Wuseless-cast
       -Wdouble-promotion -Wshadow>
     $<$<CXX_COMPILER_ID:MSVC>:
       /W4>)

add_executable(FsmTest_variant main.cpp
    vfsm/vfsm.hpp)
target_compile_options(FsmTest_variant PRIVATE ${VFSM_WARNING_OPTIONS})

include(GNUInstallDirs)
install(TARGETS FsmTest_variant
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
add_subdirectory(samples/jtag)
