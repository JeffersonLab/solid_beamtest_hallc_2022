# build analyzer binary
set(sources
    et_feeder.cpp
    evchan_test.cpp
)

foreach(src ${sources})
    # I used a simple string replace, to cut off .cpp.
    string( REPLACE ".cpp" "" exe ${src} )
    add_executable(${exe} ${src})
    target_include_directories(${exe}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        ${ROOT_INCLUDE_DIRS}
    )
    target_link_libraries(${exe}
    LINK_PUBLIC
        ${ROOT_LIBRARIES}
        evc
        conf
    )
    install(TARGETS ${exe} DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach(src ${sources})

