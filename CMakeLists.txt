cmake_minimum_required(VERSION 3.28.0)
cmake_policy(SET CMP0069 NEW) 
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
###########################################################################
# Disable insource build

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR
		"CMake generation for this project is not allowed within the source directory!"
		"\n Remove the CMakeCache.txt file and try again from another folder, e.g.:"
		"\n "
		"\n rm CMakeCache.txt"
		"\n cd .."
		"\n mkdir cmake-make"
		"\n cd cmake-make"
		"\n cmake ../project"
	)
endif()

# PROJECT
project(dyslang DESCRIPTION "dyslang" LANGUAGES CXX C)

set(DYSLANG_LIBRARY dyslang)

set(DYSLANG_FOLDER "dyslang")
set(DYSLANG_TEST_FOLDER "dyslang_tests")

set(DYSLANG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(DYSLANG_INCLUDE_DIR "${DYSLANG_SOURCE_DIR}/include")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(SLANG_IS_TOPLEVEL_PROJECT TRUE)
else()
    set(SLANG_IS_TOPLEVEL_PROJECT FALSE)
endif()

set(DYSLANG_SLANG_VERSION "2025.23.2")
if(WIN32)
    set(DYSLANG_SLANG_OS "windows-x86_64")
elseif(APPLE)
    set(DYSLANG_SLANG_OS "macos-aarch64")
elseif(UNIX)
    set(DYSLANG_SLANG_OS "linux-x86_64")
endif()
include(FetchContent)    

# COMPILER
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Qpar")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
else()
endif()

if(WIN32)
	add_definitions(-DNOMINMAX)
endif()

set(DYSLANG_SLANG_FROM_SUBMODULE FALSE)
if(NOT DYSLANG_SLANG_FROM_SUBMODULE)
    FetchContent_Populate(slang_zip URL https://github.com/shader-slang/slang/releases/download/v${DYSLANG_SLANG_VERSION}/slang-${DYSLANG_SLANG_VERSION}-${DYSLANG_SLANG_OS}.zip QUIET)
    message(STATUS "Slang dir: ${slang_zip_SOURCE_DIR}")
    add_library(slang INTERFACE)
    add_library(slang::slang-compiler SHARED IMPORTED GLOBAL)
    add_library(slang::slang-glslang MODULE IMPORTED GLOBAL)
    target_include_directories(slang 
        INTERFACE "${slang_zip_SOURCE_DIR}/include"
    )
    if(WIN32)
        set_target_properties(slang::slang-compiler PROPERTIES
            IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/bin/slang-compiler.dll
            IMPORTED_IMPLIB ${slang_zip_SOURCE_DIR}/lib/slang-compiler.lib
        )
        set_target_properties(slang::slang-glslang PROPERTIES
            IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/bin/slang-glslang.dll
        )
        target_link_libraries(slang INTERFACE slang::slang-compiler)
    elseif(APPLE)
        set_target_properties(slang::slang-compiler PROPERTIES
            IMPORTED_LOCATION "${slang_zip_SOURCE_DIR}/lib/libslang-compiler.dylib"
        )
        set_target_properties(slang::slang-glslang PROPERTIES
            IMPORTED_LOCATION "${slang_zip_SOURCE_DIR}/lib/libslang-glslang-${DYSLANG_SLANG_VERSION}.dylib"
        )
        target_link_libraries(slang INTERFACE slang::slang-compiler)
    elseif(UNIX)
        set_target_properties(slang::slang-compiler PROPERTIES
            IMPORTED_LOCATION "${slang_zip_SOURCE_DIR}/lib/libslang-compiler.so.0.${DYSLANG_SLANG_VERSION}"
        )
        set_target_properties(slang::slang-glslang PROPERTIES
            IMPORTED_LOCATION "${slang_zip_SOURCE_DIR}/lib/libslang-glslang-${DYSLANG_SLANG_VERSION}.so"
        )
        target_link_libraries(slang INTERFACE slang::slang-compiler)
    endif()
else()
    set(SLANG_ENABLE_GFX OFF)
    set(SLANG_ENABLE_SLANGD OFF)
    set(SLANG_ENABLE_SLANGC OFF)
    set(SLANG_ENABLE_SLANGRT OFF)
    set(SLANG_ENABLE_CUDA OFF)
    set(SLANG_ENABLE_TESTS OFF)
    set(SLANG_ENABLE_EXAMPLES OFF)
    set(SLANG_ENABLE_SLANG_RHI OFF)
    set(SLANG_ENABLE_REPLAYER OFF)
    FetchContent_Declare(slang GIT_REPOSITORY https://github.com/shader-slang/slang)
    FetchContent_MakeAvailable(slang)
    target_include_directories(slang INTERFACE ${slang_SOURCE_DIR}/include)
endif()
add_library(slang::slang ALIAS slang)

add_subdirectory(plugin_compiler)
add_subdirectory(src)

# if(SLANG_IS_TOPLEVEL_PROJECT)
# 	add_subdirectory(tests)
# endif()

