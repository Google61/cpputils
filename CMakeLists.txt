cmake_minimum_required(VERSION 3.10)
project(cpputils)
set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(OPT_STL_DEBUGGING "Build with STL debugging"  OFF)
option(OPT_PROF "Build for profiling"  OFF)
option(OPT_COV "Build for code coverage"  OFF)
option(OPT_LIBCXX "Build with libcxx"  OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DARWIN TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(FREEBSD TRUE)
endif()

# checking if we are called in the correct way:
#  with a -B argument.  and without a cache file in the source directory.
if (CMAKE_CACHEFILE_DIR STREQUAL "${CMAKE_SOURCE_DIR}")
    message(FATAL_ERROR "\nUnexpected CMakeCache.txt file in the source directory. Please remove it.")
    return()
endif()

if (EXISTS ${CMAKE_BINARY_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "\nRun cmake with an explicit -B buildpath")
    return()
endif()

if (OPT_LIBCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()
if (OPT_STL_DEBUGGING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LIBCPP_DEBUG_LEVEL=1")
endif()
if (OPT_PROF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg ")
endif()
if (OPT_COV)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(INFO "gcc code coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -ftest-coverage -fprofile-arcs ")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -ftest-coverage -fprofile-arcs ")
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(INFO "llvm code coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping -fdebug-info-for-profiling")
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -inline-threshold=100000")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    else()
        message(INFO "don't know how to add code coverage for ${CMAKE_CXX_COMPILER_ID }")
    endif()

endif()
if(MSVC)
    # /MP = multithreaded build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    # /utf-8 = utf8 source and execution
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    add_definitions(-DNOMINMAX -DNOGDI)
endif()



add_library(cpputils INTERFACE)
target_include_directories(cpputils INTERFACE .)

include(FetchContent)
FetchContent_Declare(fmt
      GIT_REPOSITORY https://github.com/fmtlib/fmt.git
      GIT_TAG master
      )
FetchContent_MakeAvailable(fmt)

file(GLOB UnittestSrc tests/*.cpp)
if (WIN32)
    # skippoing this test on windows.
    list(REMOVE_ITEM UnittestSrc ${CMAKE_SOURCE_DIR}/tests/test-fhandle.cpp)
endif()

# disable work-in-progress
list(REMOVE_ITEM UnittestSrc ${CMAKE_SOURCE_DIR}/tests/test-makeasn1.cpp)

add_executable(unittests ${UnittestSrc})
target_link_libraries(unittests cpputils)
target_compile_definitions(unittests PRIVATE WITH_DOCTEST)

add_executable(fmt_string-boost     fmtbench/fmt_string-boost.cpp)
target_link_libraries(fmt_string-boost cpputils)
add_executable(fmt_string-fmt       fmtbench/fmt_string-fmt.cpp)
target_link_libraries(fmt_string-fmt cpputils fmt::fmt)
add_executable(fmt_string-fmt2      fmtbench/fmt_string-fmt2.cpp)
target_link_libraries(fmt_string-fmt2 cpputils fmt::fmt)
add_executable(fmt_string-formatter fmtbench/fmt_string-formatter.cpp)
target_link_libraries(fmt_string-formatter cpputils)
add_executable(fmt_string-printf    fmtbench/fmt_string-printf.cpp)
#add_executable(fmt_string-std    fmtbench/fmt_string-std.cpp)


add_executable(fmt_print-boost     fmtbench/fmt_print-boost.cpp)
target_link_libraries(fmt_print-boost cpputils)
add_executable(fmt_print-fmt       fmtbench/fmt_print-fmt.cpp)
target_link_libraries(fmt_print-fmt cpputils fmt::fmt)
add_executable(fmt_print-fmt2      fmtbench/fmt_print-fmt2.cpp)
target_link_libraries(fmt_print-fmt2 cpputils fmt::fmt)
add_executable(fmt_print-formatter fmtbench/fmt_print-formatter.cpp)
target_link_libraries(fmt_print-formatter cpputils)
add_executable(fmt_print-printf    fmtbench/fmt_print-printf.cpp)
#add_executable(fmt_print-std    fmtbench/fmt_print-std.cpp)
