list(APPEND LIB_SOURCES
        renormalize.cpp
        merge.cpp
)

foreach(LIB_SOURCE IN LISTS LIB_SOURCES)
    #    message(STATUS "Setting compile options for ${CXX_SOURCE} to ${SOURCE_FILE_COMPILE_OPTIONS}")
    set_source_files_properties(${LIB_SOURCE} PROPERTIES COMPILE_OPTIONS "${SOURCE_FILE_COMPILE_OPTIONS}")
endforeach()

foreach(LIB_TARGET IN LISTS RENORMALIZE_LIB_TARGETS)
    target_sources(${LIB_TARGET} PRIVATE ${LIB_SOURCES})
    #target_include_directories(${CXX_TARGET} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
    target_include_directories(
            ${LIB_TARGET}
            INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
            $<INSTALL_INTERFACE:>  # <prefix>
    )
endforeach()