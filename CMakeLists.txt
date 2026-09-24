cmake_minimum_required(VERSION 3.14)
project(qids VERSION 1.3.4 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# Core library
add_library(qids_static STATIC src/qids.c)
set_target_properties(qids_static PROPERTIES OUTPUT_NAME "qids")
target_include_directories(qids_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(qids_shared SHARED src/qids.c)
set_target_properties(qids_shared PROPERTIES OUTPUT_NAME "qids")
target_include_directories(qids_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(UNIX)
    target_link_libraries(qids_static PUBLIC m)
    target_link_libraries(qids_shared PUBLIC m)
endif()

# Primary alias target
add_library(qids::qids ALIAS qids_static)

# Tests
enable_testing()
add_executable(test_qids tests/test_qids.c)
target_link_libraries(test_qids PRIVATE qids_static)
add_test(NAME qids_verification_suite COMMAND test_qids)

# Installation
install(TARGETS qids_static qids_shared
    EXPORT qidsTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES include/qids.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
