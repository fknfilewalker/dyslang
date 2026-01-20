set(SOURCES "main.cpp")

if(WIN32)
   add_executable(dyslang_plugin_compiler ${SOURCES})

   add_custom_command(TARGET dyslang_plugin_compiler POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:dyslang_plugin_compiler> $<TARGET_RUNTIME_DLLS:dyslang_plugin_compiler>
      COMMAND_EXPAND_LISTS
   )
elseif(APPLE)
   add_executable(dyslang_plugin_compiler MACOSX_BUNDLE ${SOURCES})
elseif(UNIX)
   add_executable(dyslang_plugin_compiler ${SOURCES})
   add_custom_command(TARGET dyslang_plugin_compiler POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:dyslang_plugin_compiler> $<TARGET_RUNTIME_DLLS:dyslang_plugin_compiler>
      COMMAND_EXPAND_LISTS
   )
endif()

set_target_properties(dyslang_plugin_compiler PROPERTIES FOLDER ${DYSLANG_FOLDER})
target_link_libraries(dyslang_plugin_compiler PRIVATE slang::slang)
add_dependencies(dyslang_plugin_compiler slang::slang)

define_property(GLOBAL PROPERTY DYSLANG_PLUGIN_LIST
   BRIEF_DOCS "Global list of plugins"
   FULL_DOCS "Global list of plugins"
)
set_property(GLOBAL PROPERTY DYSLANG_PLUGIN_LIST "")

# generates a dynamic lib that contains the host code and the slang module
function(dyslang_add_plugin FILEPATH VARIANTS)
   set( _VARIANTS ${VARIANTS} ${ARGN} )
   message("Adding plugin: ${FILEPATH} ${_VARIANTS}")
   get_filename_component(DIR ${FILEPATH} DIRECTORY)
   get_filename_component(FILENAME ${FILEPATH} NAME_WE)

   set(INPUT_SLANG_FILE "${CMAKE_CURRENT_LIST_DIR}/${FILEPATH}")
   set(DYNAMIC_LIB_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
   # set(SLANG_MODULE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.slang-module")
   
   add_custom_command(OUTPUT ${DYNAMIC_LIB_OUTPUT} #${SLANG_MODULE_OUTPUT}
      COMMAND dyslang_plugin_compiler ARGS ${INPUT_SLANG_FILE} "${_VARIANTS}"
      MAIN_DEPENDENCY ${INPUT_SLANG_FILE}
      DEPENDS ${INPUT_SLANG_FILE} dyslang_plugin_compiler
      COMMENT "Generating ${FILENAME} plugin"
      VERBATIM)
   add_custom_target(${FILENAME} ALL DEPENDS ${DYNAMIC_LIB_OUTPUT} SOURCES ${INPUT_SLANG_FILE}) #${SLANG_MODULE_OUTPUT})
   # set_target_properties(${FILENAME} PROPERTIES FOLDER "${DYSLANG_PLUGIN_FOLDER}/${DIR}")

   # append to the list of plugins
   set_property(GLOBAL APPEND PROPERTY DYSLANG_PLUGIN_LIST ${DYNAMIC_LIB_OUTPUT})
   # set_property(GLOBAL APPEND PROPERTY DYSLANG_PLUGIN_LIST ${SLANG_MODULE_OUTPUT})
endfunction()

function(dyslang_copy_plugins TARGET PLUGINS RELATIVE_PATH)
   add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${TARGET}>/${RELATIVE_PATH}
      COMMAND_EXPAND_LISTS
   )

   get_property(PLUGIN_LIST GLOBAL PROPERTY DYSLANG_PLUGIN_LIST)
   # message(STATUS "Plugins: ${PLUGIN_LIST}")

   # copy the plugins to the target directory
   foreach(PLUGIN ${PLUGIN_LIST})
      get_filename_component(FILENAME ${PLUGIN} NAME)
      get_filename_component(FILENAME_WE ${PLUGIN} NAME_WE)
      # if filename is in the PLUGINS list (copy it)
      list(FIND PLUGINS ${FILENAME_WE} INDEX)
      if(INDEX EQUAL -1)
         continue()
      endif()
      add_custom_command(TARGET ${TARGET} POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PLUGIN} $<TARGET_FILE_DIR:${TARGET}>/${RELATIVE_PATH}/${FILENAME}
         COMMAND_EXPAND_LISTS
         VERBATIM
      )
      # message(STATUS "Copying plugin: ${PLUGIN}")
   endforeach()
endfunction()

# USAGE:

# define variants like this
# variant_name:<template_type_0,template_type_1,...,template_type_N>
# set(EXAMPLE_VARIANTS 
#     "float_rgb:<float,Spectrum<float,3>>"
#     "double_rgb:<double,Spectrum<double,3>>"
# )
# Saved as -> float_rgb:float,Spectrum<float,3>;double_rgb:double,Spectrum<double,3>

# non template variant
# set(EXAMPLE_VARIANTS_NON_TEMPLATE 
#     "float_rgb:"
# )
# dyslang_add_plugin(emitters/point.slang ${EXAMPLE_VARIANTS})
