CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(TiltEscape)

set(SD2_INCLUDE_DIR "/usr/local/Cellar/sdl2/2.0.8/include/SDL2/")

include_directories(${SD2_INCLUDE_DIR})

AUX_SOURCE_DIRECTORY(. DIR_SRCS)
ADD_EXECUTABLE(main ${DIR_SRCS} Board.cpp Board.h SecurityCamera.cpp SecurityCamera.h)