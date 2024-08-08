set(SOURCES "main.cpp")

if(WIN32)
   add_executable(${DYSLANG_PLUGIN_COMPILER} WIN32 ${SOURCES})

   add_custom_command(TARGET ${DYSLANG_PLUGIN_COMPILER} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${DYSLANG_PLUGIN_COMPILER}> $<TARGET_RUNTIME_DLLS:${DYSLANG_PLUGIN_COMPILER}>
      COMMAND_EXPAND_LISTS
   )
elseif(APPLE)
   add_executable(${DYSLANG_PLUGIN_COMPILER} MACOSX_BUNDLE ${SOURCES})
elseif(UNIX)
   add_executable(${DYSLANG_PLUGIN_COMPILER} ${SOURCES})
endif()

set_target_properties(${DYSLANG_PLUGIN_COMPILER} PROPERTIES FOLDER ${DYSLANG_COMPILER_FOLDER})
target_link_libraries(${DYSLANG_PLUGIN_COMPILER} PRIVATE slang::slang)
add_dependencies(${DYSLANG_PLUGIN_COMPILER} slang::slang)

define_property(GLOBAL PROPERTY DYSLANG_PLUGIN_LIST
   BRIEF_DOCS "Global list of plugins"
   FULL_DOCS "Global list of plugins"
)
set_property(GLOBAL PROPERTY DYSLANG_PLUGIN_LIST "")

# use like this:
# generates a dynamic lib that contains the host code and the slang module
# also generates a slang-module file for the linter
function(dyslang_add_plugin FILEPATH)
   message("Adding plugin: ${FILEPATH}")
   get_filename_component(DIR ${FILEPATH} DIRECTORY)
   get_filename_component(FILENAME ${FILEPATH} NAME_WE)

   # set(INPUT_MODULE_FILE "${CMAKE_CURRENT_LIST_DIR}/${FILEPATH}")
   # add_library(${FILENAME} SHARED ${INPUT_MODULE_FILE})
   # add_library(DYSLANG_plugin::${FILENAME} ALIAS ${FILENAME})
   # set_target_properties(${FILENAME} PROPERTIES LINKER_LANGUAGE CXX)
   # target_link_libraries(${FILENAME} PRIVATE ${DYSLANG_CORE})
   # add_dependencies(${FILENAME} ${DYSLANG_CORE})
   # target_compile_definitions(${FILENAME} PUBLIC 
   #    _SLANG_MODULE_IR="hello"
   # )
   # set_target_properties(${FILENAME} PROPERTIES FOLDER "${DYSLANG_PLUGIN_FOLDER}/${DIR}")


   set(INPUT_SLANG_FILE "${CMAKE_CURRENT_LIST_DIR}/${FILEPATH}")
   set(DYNAMIC_LIB_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
   set(SLANG_MODULE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.slang-module")
   
   add_custom_command(OUTPUT ${DYNAMIC_LIB_OUTPUT} ${SLANG_MODULE_OUTPUT}
                     COMMAND ${DYSLANG_PLUGIN_COMPILER} ARGS ${INPUT_SLANG_FILE} ${OUT_DIR}
                     MAIN_DEPENDENCY ${INPUT_SLANG_FILE}
                     DEPENDS ${DYSLANG_PLUGIN_COMPILER}
                     COMMENT "Generating ${FILENAME} plugin"
                     VERBATIM)
   add_custom_target(${FILENAME} ALL DEPENDS ${DYNAMIC_LIB_OUTPUT} ${SLANG_MODULE_OUTPUT})
   set_target_properties(${FILENAME} PROPERTIES FOLDER "${DYSLANG_PLUGIN_FOLDER}/${DIR}")
   add_dependencies(${DYSLANG_CORE} ${FILENAME})

   # append to the list of plugins
   set_property(GLOBAL APPEND PROPERTY DYSLANG_PLUGIN_LIST ${DYNAMIC_LIB_OUTPUT})
   set_property(GLOBAL APPEND PROPERTY DYSLANG_PLUGIN_LIST ${SLANG_MODULE_OUTPUT})
endfunction()

function(dyslang_copy_plugins TARGET)
   set(PLUGIN_DIR_NAME "plugins")
   add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${TARGET}>/${PLUGIN_DIR_NAME}
      COMMAND_EXPAND_LISTS
   )

   get_property(PLUGIN_LIST GLOBAL PROPERTY DYSLANG_PLUGIN_LIST)
   message(STATUS "Plugins: ${PLUGIN_LIST}")

   # copy the plugins to the target directory
   foreach(PLUGIN ${PLUGIN_LIST})
      get_filename_component(FILENAME ${PLUGIN} NAME)
      add_custom_command(TARGET ${TARGET} POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN} $<TARGET_FILE_DIR:${TARGET}>/${PLUGIN_DIR_NAME}/${FILENAME}
         COMMAND_EXPAND_LISTS
      )
      # message(STATUS "Copying plugin: ${PLUGIN}")
   endforeach()
endfunction()

