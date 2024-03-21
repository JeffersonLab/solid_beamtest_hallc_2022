cmake_minimum_required(VERSION 3.13)

project(gem_decoder LANGUAGES CXX)
set(LIBNAME gem_decoder)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/${MAIN_PROJECT_NAME_LC})

set(srcs 
    src/AbstractRawDecoder.cpp
    src/EvioFileReader.cpp
    src/MPDSSPRawEventDecoder.cpp
    src/EventParser.cpp
    src/MPDDataStruct.cpp
    src/MPDVMERawEventDecoder.cpp
    )

set(headers
    include/AbstractRawDecoder.h
    include/EvioFileReader.h
    include/MPDDataStruct.h
    include/MPDVMERawEventDecoder.h
    include/sspApvdec.h
    include/EventParser.h
    include/GeneralEvioStruct.h
    include/MPDSSPRawEventDecoder.h
    include/RolStruct.h
    )


find_package(ROOT REQUIRED CONFIG)
include(${ROOT_USE_FILE})

add_library(${LIBNAME} SHARED ${srcs})

target_include_directories(${LIBNAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PUBLIC $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>
    )

target_link_libraries(${LIBNAME}
    PUBLIC ${ROOT_LIBRARIES}
    PUBLIC evio
    )

install(TARGETS ${LIBNAME}
    EXPORT ${LIBNAME}-exports
    LIBRARY DESTINATION ${CMAKE_INSTLL_LIBDIR}
    )

install(FILES ${headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
