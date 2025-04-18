#!/bin/bash
g++ main.cpp Game.cpp SceneManager.cpp -o main -I/usr/include/ffmpeg `sdl2-config --cflags --libs` `pkg-config --cflags --libs libavcodec libavformat libswscale libavutil`