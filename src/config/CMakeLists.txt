list(APPEND BIN_SOURCES
        main.cpp
        helper.cpp
)

foreach(BIN_SOURCE IN LISTS BIN_SOURCES)
    #    message(STATUS "Setting compile options for ${BIN_SOURCE} to ${SOURCE_FILE_COMPILE_OPTIONS}")
    set_source_files_properties(${BIN_SOURCE} PROPERTIES COMPILE_OPTIONS "${SOURCE_FILE_COMPILE_OPTIONS}")
endforeach()

target_sources(${RENORMALIZE_CONFIG_TARGET} PRIVATE ${BIN_SOURCES})
target_include_directories(${RENORMALIZE_CONFIG_TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_BINARY_DIR} ${ARGS_INCLUDE_DIR})

if (RENORMALIZE_BUILD_TESTS)
    add_test(NAME renormalize_config_help_test COMMAND ${RENORMALIZE_CONFIG_BINARY} --help)
    add_test(NAME renormalize_config_show_test COMMAND ${RENORMALIZE_CONFIG_BINARY} --show libdir)
endif()