list(APPEND BIN_SOURCES
        main.cpp
)

foreach(BIN_SOURCE IN LISTS BIN_SOURCES)
    #    message(STATUS "Setting compile options for ${BIN_SOURCE} to ${SOURCE_FILE_COMPILE_OPTIONS}")
    set_source_files_properties(${BIN_SOURCE} PROPERTIES COMPILE_OPTIONS "${SOURCE_FILE_COMPILE_OPTIONS}")
endforeach()

target_sources(${RENORMALIZE_COMBINE_TARGET} PRIVATE ${BIN_SOURCES})
target_include_directories(${RENORMALIZE_COMBINE_TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${ARGS_INCLUDE_DIR})

if (RENORMALIZE_BUILD_TESTS)
    add_test(NAME renormalize_combine_test COMMAND ${RENORMALIZE_COMBINE_TARGET} --help)
endif()