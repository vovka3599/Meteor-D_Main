cmake_minimum_required(VERSION 3.8)

project(Meteor-D_Main LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp

    config.h

    dma_proxy.h

    lmk.h
    lmk.cpp
    lmk_reg.h

    register.h

    udp.h
    udp.cpp

    l_complex.h
    l_complex.cpp

    device.h
    device.cpp
    )

add_executable(Meteor-D_Main ${SOURCES})

target_link_libraries(Meteor-D_Main pthread)

set(CMAKE_INSTALL_PREFIX /home/root)
install(TARGETS Meteor-D_Main DESTINATION /home/root/)
