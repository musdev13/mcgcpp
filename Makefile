CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -MMD -MP
INCLUDES = -I/usr/include/ffmpeg
LIBS = $(shell sdl2-config --cflags --libs) \
       $(shell pkg-config --cflags --libs libavcodec libavformat libswscale libavutil) \
       -lSDL2_image

SRCS = main.cpp Game.cpp SceneManager.cpp VideoPlayer.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
TARGET = main

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
