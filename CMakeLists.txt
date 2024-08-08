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
project(Dyslang DESCRIPTION "Dyslang" LANGUAGES CXX C)

set(DYSLANG_PLUGIN_COMPILER dyslang_plugin_compiler)
set(DYSLANG_COMPILER dyslang_compiler)

set(DYSLANG_EXTERNAL_FOLDER "External")
set(DYSLANG_COMPILER_FOLDER "Compilers")
set(DYSLANG_PLUGIN_FOLDER "Plugins")
set(DYSLANG_TEST_FOLDER "Test")

set(DYSLANG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(DYSLANG_INCLUDE_DIR "${DYSLANG_SOURCE_DIR}/include")
set(DYSLANG_EXTERNAL_DIR "${DYSLANG_SOURCE_DIR}/external")

# COMPILER
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Qpar")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
else()
endif()

if(WIN32)
	add_definitions(
		-DNOMINMAX
		-D_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
		-D_CRT_SECURE_NO_WARNINGS
		-D_CRT_NONSTDC_NO_DEPRECATE
		-D_SCL_SECURE_NO_WARNINGS
	)
endif()


add_subdirectory(external)
add_subdirectory(plugin_compiler)
add_subdirectory(src)
add_subdirectory(tests)


