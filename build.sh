#!/bin/bash
g++ main.cpp Game.cpp SceneManager.cpp -o main `sdl2-config --cflags --libs`
