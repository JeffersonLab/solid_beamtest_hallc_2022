cmake_minimum_required(VERSION 3.13)

project(gui CXX)
set(EXE gem_gui)

find_package(ROOT REQUIRED CONFIG)
include(${ROOT_USE_FILE})


# Turn on automatic invocation of the MOC, UIC & RCC
set(CMAKE_AUTOMOC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Widgets Gui Core)
#find_package(Qt5 REQUIRED Concurrent Core DBus Gui Network OpenGL Sql Widgets Xml)
#find_package(Qt5Widgets REQUIRED)
#find_package(Qt5Gui REQUIRED)
#find_package(Qt5Core REQUIRED)


set(srcs
    src/ColorSpectrum.cpp
    src/GEMAnalyzer.cpp
    src/HistoWidget.cpp
    src/QMainCanvas.cpp
    src/ComponentsSchematic.cpp
    src/GEMReplay.cpp
    src/InfoCenter.cpp
    src/QRootCanvas.cpp
    src/Detector2DItem.cpp
    src/HistoItem.cpp
    src/main.cpp
    src/Viewer.cpp
    src/Detector2DView.cpp
    src/HistoView.cpp
    src/OnlineAnalysisInterface.cpp
    )

set(headers
    include/ColorSpectrum.h
    include/Detector2DItem.h
    include/GEMAnalyzer.h
    include/HistoItem.h
    include/HistoWidget.h
    include/OnlineAnalysisInterface.h
    include/QRootCanvas.h
    include/ComponentsSchematic.h
    include/Detector2DView.h
    include/GEMReplay.h
    include/HistoView.h
    include/InfoCenter.h
    include/QMainCanvas.h
    include/Viewer.h
    )

# https://github.com/bincrafters/community/issues/466
# need to add your header files in add_executable, otherwise the moc won't parse them
add_executable(${EXE} ${srcs} ${headers})

target_include_directories(${EXE}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC /usr/include/qt5/QtGui
    PUBLIC /usr/include/qt5
    PUBLIC /usr/include/qt5/QtWidgets
    PUBLIC /usr/include/qt5/QtCore
    )

target_link_libraries(${EXE}
    PUBLIC gem_ana
    PUBLIC gem_decoder
    PUBLIC ${ROOT_LIBRARIES}
    PUBLIC Qt5::Widgets Qt5::Core Qt5::Gui
    )

install(TARGETS ${EXE}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

