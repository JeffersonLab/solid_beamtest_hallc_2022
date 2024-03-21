cmake_minimum_required(VERSION 3.5)

PROJECT(EPICSystem CXX)

# required packages
find_package(ROOT 6.0 REQUIRED CONFIG)
include(${ROOT_USE_FILE})

#----------------------------------------------------------------------------
# Install in GNU-style directory layout
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/${MAIN_PROJECT_NAME_LC})

#----------------------------------------------------------------------------
# Sources and headers
set(src
    src/EPICSystem.cpp
)

set(headers
    include/EPICSystem.h
)

set(LIBNAME EpicSys)

add_library(${LIBNAME} SHARED ${src})

target_include_directories(${LIBNAME}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>
)

target_link_libraries(${LIBNAME}
PUBLIC
    ${ROOT_LIBRARIES}
    evio
    ROOT::Spectrum
)

install(TARGETS ${LIBNAME}
    EXPORT ${MAIN_PROJECT_NAME_LC}-exports
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES
    DESTINATION
    ${CMAKE_INSTALL_LIBDIR})
