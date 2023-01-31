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

    device.h
    device.cpp

    udp_receiver.h
    udp_receiver.cpp

    command_arguments.h
    cxxopts.hpp
    )

add_compile_options(-O3 -mcpu=cortex-a9 -mfpu=neon -mvectorize-with-neon-quad -mfloat-abi=hard -ffast-math -DNDEBUG)

add_executable(Meteor-D_Main ${SOURCES})

target_link_libraries(Meteor-D_Main pthread)

set(CMAKE_INSTALL_PREFIX /home/root)
install(TARGETS Meteor-D_Main DESTINATION /home/root/)
