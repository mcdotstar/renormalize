cmake_minimum_required(VERSION 3.30)
cmake_policy(SET CMP0076 NEW) # Ensure target_sources converts relative paths
cmake_policy(SET CMP0017 NEW) # Prefer cmake's own files for include/find_package before CMAKE_MODULE_PATH

set(CMAKE_MACOSX_RPATH OFF)
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(RENORMALIZE_BUILD_ON_CONDA OFF CACHE BOOL "Set to ON to build for conda")
set(RENORMALIZE_USE_CONAN ON CACHE BOOL "Use Conan to manage dependencies") # Set to OFF to use system libraries
set(RENORMALIZE_BUILD_TESTS ON CACHE BOOL "Build test binary")

# Read the version of renormalize
include(check)
checkSetup(RENORMALIZE) # generates version.hpp and adds the generated directory as a target include directory
# So that we can print it to the console along with the specified build type
message(STATUS "Build Renormalize v${RENORMALIZE_VERSION} with type ${CMAKE_BUILD_TYPE}")
project(Renormalize LANGUAGES C CXX VERSION "${RENORMALIZE_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MCPL REQUIRED)

set(RENORMALIZE_LIBRARY_TARGET renormalize)
set(RENORMALIZE_COMBINE_TARGET renormalize-combine)
set(RENORMALIZE_CONFIG_TARGET renormalize-config)

set(RENORMALIZE_LIB_DESTINATION lib)
set(RENORMALIZE_BIN_DESTINATION bin)
set(RENORMALIZE_INCLUDE_DESTINATION include)
set(RENORMALIZE_TESTER_TARGET renormalize_tester)

include (GNUInstallDirs)
set(Renormalize_BINDIR "${CMAKE_INSTALL_BINDIR}")  # e.g. <prefix>/bin
set(Renormalize_LIBDIR "${CMAKE_INSTALL_LIBDIR}")  # e.g. <prefix>/lib
set(Renormalize_INCDIR "${CMAKE_INSTALL_INCLUDEDIR}")  # e.g. <prefix>/include
set(Renormalize_DATADIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}") # e.g. <prefix>/share/Renormalize>
if (NOT Renormalize_CMAKEDIR)
    set(Renormalize_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}") #e.g. <prefix>/lib/cmake/Renormalize>
endif()

#Get a few relative paths, mostly for expansion in various installed files (we
#use PROJECT_BINARY_DIR as prefix here, but it should not matter which as long
#as it is an absolute path):
file(RELATIVE_PATH Renormalize_BIN2LIB "${PROJECT_BINARY_DIR}/${Renormalize_BINDIR}" "${PROJECT_BINARY_DIR}/${Renormalize_LIBDIR}")
file(RELATIVE_PATH Renormalize_BIN2INC "${PROJECT_BINARY_DIR}/${Renormalize_BINDIR}" "${PROJECT_BINARY_DIR}/${Renormalize_INCDIR}")
file(RELATIVE_PATH Renormalize_BIN2DATA "${PROJECT_BINARY_DIR}/${Renormalize_BINDIR}" "${PROJECT_BINARY_DIR}/${Renormalize_DATADIR}")
file(RELATIVE_PATH Renormalize_CMAKE2ROOT "${PROJECT_BINARY_DIR}/${Renormalize_CMAKEDIR}" "${PROJECT_BINARY_DIR}/")
file(RELATIVE_PATH Renormalize_CMAKE2BIN "${PROJECT_BINARY_DIR}/${Renormalize_CMAKEDIR}" "${PROJECT_BINARY_DIR}/${Renormalize_BINDIR}")
file(RELATIVE_PATH Renormalize_CMAKE2LIB "${PROJECT_BINARY_DIR}/${Renormalize_CMAKEDIR}" "${PROJECT_BINARY_DIR}/${Renormalize_LIBDIR}")
file(RELATIVE_PATH Renormalize_CMAKE2INC "${PROJECT_BINARY_DIR}/${Renormalize_CMAKEDIR}" "${PROJECT_BINARY_DIR}/${Renormalize_INCDIR}")
include(CMakePackageConfigHelpers)
write_basic_package_version_file( "${PROJECT_BINARY_DIR}/RenormalizeConfigVersion.cmake"
        VERSION ${Renormalize_VERSION} COMPATIBILITY SameMajorVersion )



if (MSVC)
    # warning level 4 -- add /WX for all warnings as errors
    add_compile_options(/W4)
    # suppress MSVC warning C4996 about 'localtime' vs 'localtime_s'
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Allow UTF-8 identifiers https://stackoverflow.com/a/47704050
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    # Prevent Windows.h from defining its own min and max macros
    add_definitions(-DNOMINMAX -DREADOUT_SHARED -DREADOUT_EXPORT)
elseif(APPLE)
    # lots of warnings -- add -Werror for  all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic)
    if (READOUT_BUILD_ON_CONDA)
        # Enable use of, e.g. std::filesystem::path on macOS under Conda:
        # https://conda-forge.org/docs/maintainer/knowledge_base/#newer-c-features-with-old-sdk
        add_definitions(-D_LIBCPP_DISABLE_AVAILABILITY)
    endif()
else()
    # lots of warnings -- add -Werror for  all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic)
endif()

set(CMAKE_MACOSX_RPATH OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# With GCC 10+ the interproceedural optmization only adds to compilation time without improving performance
SET(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)



set(RENORMALIZE_LIB_TARGETS ${RENORMALIZE_LIBRARY_TARGET})
add_library(${RENORMALIZE_LIBRARY_TARGET} SHARED)
#set_target_properties(${RENORMALIZE_LIBRARY_TARGET} PROPERTIES
#        POSITION_INDEPENDENT_CODE ON
#        CXX_STANDARD 20
#        CXX_STANDARD_REQUIRED ON
#        CXX_EXTENSIONS OFF
#        PUBLIC_HEADER "lib/renormalize.h"
#)
set(Renormalize_LIBNAME "${CMAKE_SHARED_LIBRARY_PREFIX}${RENORMALIZE_LIBRARY_TARGET}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set_target_properties(${RENORMALIZE_LIBRARY_TARGET} PROPERTIES PUBLIC_HEADER lib/renormalize.h)
set_target_properties(${RENORMALIZE_LIBRARY_TARGET} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

add_library(renormalize::${RENORMALIZE_LIBRARY_TARGET} ALIAS ${RENORMALIZE_LIBRARY_TARGET})


set(RENORMALIZE_BIN_TARGETS ${RENORMALIZE_COMBINE_TARGET} ${RENORMALIZE_CONFIG_TARGET})

add_executable(${RENORMALIZE_COMBINE_TARGET})
target_include_directories(${RENORMALIZE_COMBINE_TARGET} PRIVATE ${MCPL_INCLUDE_DIRS})
target_link_libraries(${RENORMALIZE_COMBINE_TARGET} PRIVATE MCPL::mcpl ${RENORMALIZE_LIBRARY_TARGET})

add_executable(${RENORMALIZE_CONFIG_TARGET})

#foreach (target ${RENORMALIZE_BIN_TARGETS})
#    target_include_directories(${target} PRIVATE ${MCPL_INCLUDE_DIRS})
#    target_link_libraries(${target} PRIVATE MCPL::mcpl ${RENORMALIZE_LIBRARY_TARGET})
#endforeach ()

add_subdirectory(src)
add_subdirectory(lib)



install(TARGETS ${RENORMALIZE_LIBRARY_TARGET} ${RENORMALIZE_COMBINE_TARGET} ${RENORMALIZE_CONFIG_TARGET} EXPORT RenormalizeTargets
        RUNTIME DESTINATION ${Renormalize_BINDIR}
        LIBRARY DESTINATION ${Renormalize_LIBDIR}
        ARCHIVE DESTINATION ${Renormalize_LIBDIR}
        PUBLIC_HEADER DESTINATION ${Renormalize_INCDIR})
# The following would allow for importing Renormalize by another CMake ... but causes a CMake configuration error
install(EXPORT ${PROJECT_NAME}Targets FILE ${PROJECT_NAME}Targets.cmake NAMESPACE renormalize:: DESTINATION ${Renormalize_CMAKEDIR})

configure_file("${PROJECT_SOURCE_DIR}/cmake/RenormalizeConfig.cmake.in" "${PROJECT_BINARY_DIR}/RenormalizeConfig.cmake" @ONLY)

install(FILES "${PROJECT_BINARY_DIR}/RenormalizeConfigVersion.cmake" "${PROJECT_BINARY_DIR}/RenormalizeConfig.cmake"
        DESTINATION ${Renormalize_CMAKEDIR} )

configure_file("${PROJECT_SOURCE_DIR}/cmake/renormalize_config.h.in" "${CMAKE_BINARY_DIR}/renormalize_config.h" @ONLY)

# Copy the component file to the include directory too
file(GLOB ALL_COMPONENTS "${CMAKE_CURRENT_SOURCE_DIR}/components/*.comp")
install(FILES ${ALL_COMPONENTS} DESTINATION ${Renormalize_DATADIR})