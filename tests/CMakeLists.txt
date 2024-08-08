function(add_target target)
    cmake_parse_arguments(PARSE_ARGV 1 TARGET "" "" "DEPS;SOURCES")
    if(WIN32)
        add_executable(${target} WIN32 ${TARGET_SOURCES})
        set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${target}> $<TARGET_RUNTIME_DLLS:${target}>
            COMMAND_EXPAND_LISTS
        )
    elseif(APPLE)
        add_executable(${target} MACOSX_BUNDLE ${TARGET_SOURCES})
        set_target_properties(${target} PROPERTIES XCODE_GENERATE_SCHEME TRUE CMAKE_XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    elseif(UNIX)
    endif()
    set_target_properties(${target} PROPERTIES FOLDER ${DYSLANG_TEST_FOLDER})
    if(TARGET_DEPS)
        target_link_libraries(${target} PRIVATE ${TARGET_DEPS})
        add_dependencies(${target} ${TARGET_DEPS})
    endif()


    install(TARGETS ${target} CONFIGURATIONS Debug DESTINATION "debug")
    install(TARGETS ${target} CONFIGURATIONS Release DESTINATION "release")
endfunction(add_target target)

add_target(slang_interface_type_conformance DEPS ${DYSLANG_COMPILER} SOURCES "slang_interface_type_conformance/main.cpp" "slang_interface_type_conformance/generic-interface-conformance.slang")