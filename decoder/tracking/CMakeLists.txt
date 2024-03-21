cmake_minimum_required(VERSION 3.5)

PROJECT(HCTracking CXX)

#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)

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
    HCTracking.cpp
    GEMModule.cpp
    MPDModule.cxx
    MPDModuleVMEv4.cxx
#    test.cpp
    EventWrapper.cpp
    Database/Database.cxx
    Database/VarType.cxx
    Database/Textvars.cxx
    ha_analyzer/THaEvData.cxx
    ha_analyzer/Module.cxx
    ha_analyzer/THaSlotData.cxx
    ha_analyzer/THaCrateMap.cxx
    ha_analyzer/CodaDecoder.cxx
    ha_analyzer/THaUsrstrutils.cxx
    ha_analyzer/DAQconfig.cxx
    ha_analyzer/VmeModule.cxx
)

set(headers
    HCTracking.h
    GEMModule.h
    MPDModule.h
    MPDModuleVMEv4.h
    EventWrapper.h
    Database/Textvars.h
    Database/Database.h
    Database/Helper.h
    Database/VarDef.h
    Database/VarType.h
    ha_analyzer/THaEvData.h
    ha_analyzer/Decoder.h
    ha_analyzer/Module.h
    ha_analyzer/THaSlotData.h
    ha_analyzer/CustomAlloc.h
    ha_analyzer/THaCrateMap.h
    ha_analyzer/THaBenchmark.h
    ha_analyzer/CodaDecoder.h
    ha_analyzer/THaUsrstrutils.h
    ha_analyzer/DAQconfig.h
    ha_analyzer/VmeModule.h
)

set(LIBNAME hctracking)

add_library(${LIBNAME} SHARED ${src})
#add_executable(${LIBNAME} ${src} ${headers})

target_include_directories(${LIBNAME}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/Database>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/ha_analyzer>
)

target_link_libraries(${LIBNAME}
PUBLIC
    ${ROOT_LIBRARIES}
    ROOT::Spectrum
    evc
    conf
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

#add_subdirectory(third_party)
