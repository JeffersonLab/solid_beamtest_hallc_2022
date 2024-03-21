# build analyzer binary
# with tracking executable
set(exe analyze_tracking)
add_executable(${exe} analyze.cpp)

target_include_directories(${exe}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    ${ROOT_INCLUDE_DIRS}
)

target_link_libraries(${exe}
LINK_PUBLIC
    ${ROOT_LIBRARIES}
    evc
    fdec
    ssp
    tidec
    EpicSys
    conf
    gem_decoder
    gem_ana
    hctracking
    hctracking_dev
)
target_link_libraries(${exe} PRIVATE  nlohmann_json::nlohmann_json)

install(TARGETS ${exe} DESTINATION ${CMAKE_INSTALL_BINDIR})

# without tracking executable
set(exe2 analyze)
add_executable(${exe2} analyze_no_tracking.cpp)
target_include_directories(${exe2}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    ${ROOT_INCLUDE_DIRS}
)

target_link_libraries(${exe2}
LINK_PUBLIC
    ${ROOT_LIBRARIES}
    evc
    fdec
    ssp
    tidec
    EpicSys
    conf
    gem_decoder
    gem_ana
    hctracking
    hctracking_dev
)
target_link_libraries(${exe2} PRIVATE  nlohmann_json::nlohmann_json)

install(TARGETS ${exe2} DESTINATION ${CMAKE_INSTALL_BINDIR})
