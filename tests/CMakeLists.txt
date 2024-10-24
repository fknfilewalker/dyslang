function(add_target target)
    cmake_parse_arguments(PARSE_ARGV 1 TARGET "" "" "DEPS;SOURCES")
    if(WIN32)
        add_executable(${target} ${TARGET_SOURCES})
        set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${target}> $<TARGET_RUNTIME_DLLS:${target}>
            COMMAND_EXPAND_LISTS
        )
    elseif(APPLE)
        add_executable(${target} MACOSX_BUNDLE ${TARGET_SOURCES})
        set_target_properties(${target} PROPERTIES XCODE_GENERATE_SCHEME TRUE CMAKE_XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    elseif(UNIX)
        add_executable(${target} ${TARGET_SOURCES})
    endif()
    set_target_properties(${target} PROPERTIES FOLDER ${DYSLANG_TEST_FOLDER})
    if(TARGET_DEPS)
        target_link_libraries(${target} PRIVATE ${TARGET_DEPS})
        add_dependencies(${target} ${TARGET_DEPS})
    endif()


    install(TARGETS ${target} CONFIGURATIONS Debug DESTINATION "debug")
    install(TARGETS ${target} CONFIGURATIONS Release DESTINATION "release")
endfunction(add_target target)

add_target(slang_interface_type_conformance DEPS ${DYSLANG_LIBRARY} SOURCES "slang_interface_type_conformance/main.cpp" "slang_interface_type_conformance/interface_from_buffer.slang")

set(EMITTER_VARIANTS 
    "float_rgb:<float>"
    "double_rgb:<double>"
)
dyslang_add_plugin(load_plugins/point.slang ${EMITTER_VARIANTS})
dyslang_add_plugin(load_plugins/spot.slang ${EMITTER_VARIANTS})
set_target_properties(point spot PROPERTIES FOLDER "${DYSLANG_TEST_FOLDER}/plugins")
add_target(load_plugins_one DEPS ${DYSLANG_LIBRARY}
    SOURCES "load_plugins/one_plugin.cpp" "load_plugins/emitter.slang" "load_plugins/point.slang" "load_plugins/interface_test.slang"
)
add_target(load_plugins_two DEPS ${DYSLANG_LIBRARY}
    SOURCES "load_plugins/two_plugins.cpp" "load_plugins/emitter.slang" "load_plugins/point.slang" "load_plugins/spot.slang" "load_plugins/interface_test.slang"
)
dyslang_copy_plugins(load_plugins_one plugins)
dyslang_copy_plugins(load_plugins_two plugins)
