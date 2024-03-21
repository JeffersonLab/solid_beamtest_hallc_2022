cmake_minimum_required(VERSION 3.13)

project(gem_ana LANGUAGES CXX)
set(LIBNAME gem_ana)

find_package(ROOT REQUIRED CONFIG)
include(${ROOT_USE_FILE})

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/${MAIN_PROJECT_NAME_LC})

set(srcs
    src/APVStripMapping.cpp
    src/GEMAPV.cpp
    src/GEMException.cpp
    src/GEMRootHitTree.cpp
    src/GEMCluster.cpp
    src/GEMMPD.cpp
    src/GEMSystem.cpp
    src/GEMDataHandler.cpp
    src/GEMPedestal.cpp
    src/GEMDetector.cpp
    src/GEMPlane.cpp
    src/PreAnalysis.cpp
    src/GEMDetectorLayer.cpp
    src/GEMRootClusterTree.cpp
    src/ValueType.cpp
    src/Cuts.cpp
    )

set(headers
    include/APVStripMapping.h
    include/GEMDetector.h
    include/GEMPedestal.h
    include/GEMStruct.h
    include/GEMAPV.h
    include/GEMDetectorLayer.h
    include/GEMPlane.h
    include/GEMSystem.h
    include/GEMCluster.h
    include/GEMException.h
    include/GEMRootClusterTree.h
    include/hardcode.h
    include/GEMDataHandler.h
    include/GEMMPD.h
    include/GEMRootHitTree.h
    include/PreAnalysis.h
    include/ValueType.h
    include/Cuts.h
    )

add_library(${LIBNAME} ${srcs})

target_include_directories(${LIBNAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${ROOT_INCLUDE_DIRS}
    )

target_link_libraries(${LIBNAME}
    PUBLIC ${ROOT_LIBRARIES}
    PUBLIC gem_decoder
    PUBLIC evio
    PUBLIC conf
    )

install(TARGETS ${LIBNAME}
    EXPORT ${LIBNAME}-export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

install(FILES ${headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
