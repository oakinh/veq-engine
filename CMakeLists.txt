cmake_minimum_required(VERSION 3.22.1)

project(veq VERSION 0.1.0 LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(VEQ_BUILD_TESTS              "Build unit tests" ON)
option(VEQ_BUILD_BENCHMARKS         "Build benchmarks" OFF)
option(VEQ_BUILD_SCRATCH            "Build scratch executable" ON)
option(VEQ_WARNINGS_AS_ERRORS       "Treat warnings as errors for VEQ targets" ON)

include(CTest)

# ------------------------------------------------------------------------------
# Project options
# ------------------------------------------------------------------------------
add_library(veq_options INTERFACE)
target_compile_features(veq_options INTERFACE cxx_std_23)

if(MSVC)
    target_compile_options(veq_options INTERFACE /W4)
    if(VEQ_WARNINGS_AS_ERRORS)
        target_compile_options(veq_options INTERFACE /WX)
    endif()
else()
    target_compile_options(veq_options INTERFACE -Wall -Wextra -Wpedantic)
    if(VEQ_WARNINGS_AS_ERRORS)
        target_compile_options(veq_options INTERFACE -Werror)
    endif()
endif()

# ------------------------------------------------------------------------------
# Core library
# ------------------------------------------------------------------------------
add_library(veq STATIC)
add_library(veq::veq ALIAS veq)

target_link_libraries(veq PUBLIC veq_options)

target_include_directories(veq
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_sources(veq
        PRIVATE
        src/storage/column.cpp
        src/storage/table.cpp
        src/storage/batch.cpp
        src/execution/scan.cpp
        src/execution/filter.cpp
        src/execution/projection.cpp

        PUBLIC
        include/veq/storage/column.hpp
        include/veq/storage/table/table.hpp
        include/veq/storage/table/schema.hpp
        include/veq/storage/batch.hpp
        include/veq/execution/scan.hpp
        include/veq/execution/filter.hpp
        include/veq/execution/projection.hpp
)

# ------------------------------------------------------------------------------
# clang-tidy
# ------------------------------------------------------------------------------
function(veq_apply_tidy target_name)
    if(CMAKE_CXX_CLANG_TIDY)
        set_property(TARGET ${target_name} PROPERTY CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
    endif()
endfunction()

veq_apply_tidy(veq)

# ------------------------------------------------------------------------------
# Scratch executable
# ------------------------------------------------------------------------------
if(VEQ_BUILD_SCRATCH)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Skipping veq_scratch in Release build.")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/scratch/debug.cpp")
        add_executable(veq_scratch scratch/debug.cpp)
        target_link_libraries(veq_scratch PRIVATE veq::veq)
        veq_apply_tidy(veq_scratch)
    else()
        message(STATUS "VEQ_BUILD_SCRATCH=ON but scratch/debug.cpp not found; skipping.")
    endif()
endif()

# ------------------------------------------------------------------------------
# GoogleTest
# ------------------------------------------------------------------------------
if(VEQ_BUILD_TESTS AND BUILD_TESTING)
    include(FetchContent)

    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK   OFF CACHE BOOL "" FORCE)

    set(_OLD_TIDY "${CMAKE_CXX_CLANG_TIDY}")
    set(CMAKE_CXX_CLANG_TIDY "")

    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)

    set(CMAKE_CXX_CLANG_TIDY "${_OLD_TIDY}")

    include(GoogleTest)

    add_executable(veq_tests
#            tests/column_test.cpp
            tests/table_test.cpp
#            tests/batch_test.cpp
            tests/scan_test.cpp
            tests/filter_test.cpp
            tests/projection_test.cpp
    )

    target_link_libraries(veq_tests
            PRIVATE
            veq::veq
            GTest::gtest_main
    )

    target_include_directories(veq_tests
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    veq_apply_tidy(veq_tests)
    gtest_discover_tests(veq_tests DISCOVERY_TIMEOUT 30)
endif()

# ------------------------------------------------------------------------------
# Benchmarks
# ------------------------------------------------------------------------------
if(VEQ_BUILD_BENCHMARKS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt")
        add_subdirectory(benchmarks)
    else()
        message(STATUS "VEQ_BUILD_BENCHMARKS=ON but benchmarks/ missing; skipping.")
    endif()
endif()

# ------------------------------------------------------------------------------
# clang-format
# ------------------------------------------------------------------------------
find_program(CLANG_FORMAT_EXE NAMES clang-format-19 clang-format)

if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE VEQ_FORMAT_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/**/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/**/*.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/**/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/scratch/**/*.cpp"
    )

    add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${VEQ_FORMAT_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-format on VEQ sources"
    )

    add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${VEQ_FORMAT_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Checking clang-format compliance"
    )
else()
    message(STATUS "clang-format not found; format targets unavailable.")
endif()