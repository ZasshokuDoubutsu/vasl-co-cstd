cmake_minimum_required(VERSION 3.15)

project(vasl_co C)

option(VASL_CO_BUILD_EXAMPLES "Build the example programs" ON)

add_library(vasl_co STATIC
    vasl_co.c
)

target_include_directories(vasl_co
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_compile_features(vasl_co PUBLIC c_std_99)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(vasl_co PRIVATE -Wall -Wextra)
endif()

if(VASL_CO_BUILD_EXAMPLES)
    add_executable(vasl_co_blink examples/blink.c)
    target_link_libraries(vasl_co_blink PRIVATE vasl_co)

    add_executable(vasl_co_concurrency examples/concurrency.c)
    target_link_libraries(vasl_co_concurrency PRIVATE vasl_co)

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(vasl_co_blink PRIVATE -Wall -Wextra)
        target_compile_options(vasl_co_concurrency PRIVATE -Wall -Wextra)
    endif()
endif()
