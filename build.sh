#!/bin/bash

# Compiler and flags
CXX="g++"
CXXFLAGS="-O2 -Wall -Wextra"

# Source files
SOURCES="main.cpp Game.cpp SceneManager.cpp VideoPlayer.cpp"

# Include directories
INCLUDES="-I/usr/include/ffmpeg"

# Libraries
SDL_LIBS=`sdl2-config --cflags --libs`
FFMPEG_LIBS=`pkg-config --cflags --libs libavcodec libavformat libswscale libavutil`
EXTRA_LIBS="-lSDL2_image"

# Build command
$CXX $CXXFLAGS $SOURCES -o main $INCLUDES $SDL_LIBS $FFMPEG_LIBS $EXTRA_LIBS