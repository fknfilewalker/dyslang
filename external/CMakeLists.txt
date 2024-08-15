set(SLANG_VERSION "2024.9.2")

include(FetchContent)    
if(WIN32)
    set(SLANG_OS "windows-x86_64")
elseif(APPLE)
    set(SLANG_OS "macos-aarch64")
elseif(UNIX)
    set(SLANG_OS "linux-x86_64")
endif()
FetchContent_Populate(slang_zip URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-${SLANG_OS}.zip QUIET)
message(STATUS "Slang dir: ${slang_zip_SOURCE_DIR}")
add_library(slang SHARED IMPORTED GLOBAL)
add_library(slang::slang ALIAS slang)
if(WIN32)
    set_target_properties(slang PROPERTIES
        IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/bin/slang.dll
        IMPORTED_IMPLIB ${slang_zip_SOURCE_DIR}/lib/slang.lib
        INTERFACE_INCLUDE_DIRECTORIES ${slang_zip_SOURCE_DIR}/include
    )
elseif(APPLE)
    set_target_properties(slang PROPERTIES
        IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/lib/libslang.dylib
        INTERFACE_INCLUDE_DIRECTORIES ${slang_zip_SOURCE_DIR}/include
    )
elseif(UNIX)
    set_target_properties(slang PROPERTIES
        IMPORTED_LOCATION ${slang_zip_SOURCE_DIR}/lib/libslang.so
        INTERFACE_INCLUDE_DIRECTORIES ${slang_zip_SOURCE_DIR}/include
    )
endif()
