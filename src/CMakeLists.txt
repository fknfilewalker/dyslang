add_library(${DYSLANG_LIBRARY} STATIC 
    ${DYSLANG_INCLUDE_DIR}/dyslang/dyslang.h
    ${DYSLANG_INCLUDE_DIR}/dyslang/slangc.h slangc.cpp 
    ${DYSLANG_INCLUDE_DIR}/dyslang/plugin.h plugin.cpp 
    # ${DYSLANG_INCLUDE_DIR}/dyslang/properties.h 
    ${DYSLANG_INCLUDE_DIR}/dyslang/platform.h)

target_include_directories(${DYSLANG_LIBRARY} PUBLIC "${DYSLANG_INCLUDE_DIR}")
set_target_properties(${DYSLANG_LIBRARY} PROPERTIES FOLDER ${DYSLANG_FOLDER})

add_dependencies(${DYSLANG_LIBRARY} slang)
target_link_libraries(${DYSLANG_LIBRARY} PUBLIC slang)
