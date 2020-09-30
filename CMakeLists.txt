cmake_minimum_required(VERSION 3.9)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(DISTRAC_PROJECT OFF)
else()
    set(DISTRAC_PROJECT ON)
endif()

if(DISTRAC_PROJECT)
    project(distrac
        LANGUAGES C CXX
        VERSION 0.1.0)

    set(BUILD_NUMBER "0" CACHE STRING "Build-Number")
    set(BUILD_URL "" CACHE STRING "Unique URL to identify this build.")

    set(VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}+${BUILD_NUMBER})

    option(DEBUG_ENABLE_ADDRESS_SANITIZER "enable address sanitizer (GCC)" OFF)
    option(DEBUG_ENABLE_CLANG_TIDY "enable clang-tidy check of source code" OFF)
    option(DEBUG_ENABLE_CLANG_TIDY_WITH_FIX "allow clang-tidy to modify source files and automatically fix issues. Very likely leads to uncompilable code, use selectively." OFF)

    option(DISTRAC_ENABLE_TESTS "enable building and integrating unit tests" ON)
endif()

# Libraries
if(NOT Threads_FOUND)
    find_package(Threads)
endif()

# Generate Export Header
include(GenerateExportHeader)

if(DISTRAC_PROJECT)
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_STATIC_RUNTIME OFF)
    add_definitions(-DBOOST_ALL_DYN_LINK)

    set(Boost_USE_MULTITHREADED ON)
    find_package(Boost COMPONENTS program_options filesystem system iostreams coroutine context REQUIRED)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if ( CMAKE_COMPILER_IS_GNUCC )
            set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
        endif()
        if ( MSVC )
            set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
        endif()
    endif()

    find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable"
        )
    if(NOT CLANG_TIDY_EXE)
        message(STATUS "clang-tidy not found.")
    else()
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE} - Enable checking with -DDEBUG_ENABLE_CLANG_TIDY=ON in CMake. Warning: Compilation Overhead!")

        set(CLANG_TIDY_ARG_FIX "")
        if(DEBUG_ENABLE_CLANG_TIDY_WITH_FIX)
            set(CLANG_TIDY_ARG_FIX "-fix")
        endif()

        set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" ${CLANG_TIDY_ARG_FIX} "-checks=*,-clang-analyzer-alpha.*,-llvm-namespace-comment,-google-readability-namespace-comments,-fuchsia*,-modernize-use-trailing-return-type,-hicpp-no-array-decay,-cppcoreguidelines-*,-modernize-use-equals-default,-*braces-around-statements,-hicpp-avoid-goto,-modernize-concat-nested-namespaces,-readability-implicit-bool-conversion")
    endif()

    find_program(CCACHE_PROGRAM ccache)
    # Speed up compilations using ccache.
    if(CCACHE_PROGRAM)
        message(STATUS "Found ccache at ${CCACHE_PROGRAM} - using it to speed up compilations.")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    endif()
endif()

add_subdirectory(third_party)

# Add Catch2 unit testing framework.
if(DISTRAC_ENABLE_TESTS)
    cmake_policy(SET CMP0079 NEW)
    include(CTest)
    include(${CMAKE_CURRENT_SOURCE_DIR}/third_party/catch/Catch2-2.13.0/contrib/Catch.cmake)
    enable_testing()
endif()

if(DISTRAC_PROJECT)
    # Address Sanitizer for Debug Builds
    if(DEBUG_ENABLE_ADDRESS_SANITIZER)
        set(CMAKE_BUILD_TYPE Debug)
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address  -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=undefined -fno-sanitize=vptr")
        set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address  -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=undefined -fno-sanitize=vptr")
    endif()

    if(DISTRAC_ENABLE_TESTS)
        set(distrac_test_libs  "" CACHE INTERNAL "Libraries for Tests for Distrac")
    endif()
endif()

add_subdirectory(src)

add_library(distrac STATIC ${DISTRAC_SRCS})
target_include_directories(distrac PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_property(TARGET distrac PROPERTY POSITION_INDEPENDENT_CODE ON)
generate_export_header(distrac)
target_include_directories(distrac PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_library(distrac-analysis SHARED ${DISTRAC_ANALYSIS_SRCS})
target_link_libraries(distrac-analysis PUBLIC distrac Boost::iostreams)
set_property(TARGET distrac-analysis PROPERTY CXX_STANDARD 17)
generate_export_header(distrac-analysis)
target_include_directories(distrac-analysis PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_library(distrac-codegen SHARED ${DISTRAC_CODEGEN_SRCS})
target_link_libraries(distrac-codegen PUBLIC distrac)
set_property(TARGET distrac-codegen PROPERTY CXX_STANDARD 17)
generate_export_header(distrac-codegen)
target_include_directories(distrac-codegen PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(distrac-bin ${DISTRAC_BIN_SRCS})
set_target_properties(distrac-bin
    PROPERTIES OUTPUT_NAME distrac)
target_link_libraries(distrac-bin Boost::program_options Boost::filesystem Threads::Threads distrac-analysis)

add_executable(distrac-codegen-bin ${DISTRAC_CODEGEN_BIN_SRCS})
set_property(TARGET distrac-codegen-bin PROPERTY CXX_STANDARD 17)
set_target_properties(distrac-codegen-bin
    PROPERTIES OUTPUT_NAME distrac-codegen)
target_link_libraries(distrac-codegen-bin Boost::program_options Boost::filesystem Threads::Threads distrac-codegen distrac-analysis)

if(DISTRAC_PROJECT)
    if(DISTRAC_ENABLE_TESTS)
        add_library(distrac_lib_test SHARED ${DISTRAC_TEST_SRCS})
        set_property(TARGET distrac_lib_test PROPERTY CXX_STANDARD 17)
        target_link_libraries(distrac_lib_test PUBLIC distrac Catch2::Catch2)

        set(distrac_test_libs ${distrac_test_libs} distrac_lib_test CACHE INTERNAL "Libraries for Tests")

        add_subdirectory(test)
    endif()
endif()
