add_library(${DYSLANG_COMPILER} STATIC slangc.cpp)
add_library(tamashii::${DYSLANG_COMPILER} ALIAS ${DYSLANG_COMPILER})

target_include_directories(${DYSLANG_COMPILER} PUBLIC "${DYSLANG_INCLUDE_DIR}")
set_target_properties(${DYSLANG_COMPILER} PROPERTIES FOLDER ${DYSLANG_COMPILER_FOLDER})

add_dependencies(${DYSLANG_COMPILER} slang)
target_link_libraries(${DYSLANG_COMPILER} PUBLIC slang)
