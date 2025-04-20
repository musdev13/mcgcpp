#ifndef VideoPlayer_hpp
#define VideoPlayer_hpp

#include "SDL2/SDL.h"
#include <string>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

class VideoPlayer {
public:
    VideoPlayer(SDL_Renderer* renderer);
    ~VideoPlayer();

    bool initialize(const std::string& path);
    bool decodeNextFrame();
    void cleanup();
    SDL_Texture* getTexture() const { return videoTexture; }
    Uint32 getFrameDelay() const { return frameDelay; }

private:
    SDL_Renderer* renderer;
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    AVFrame* frame;
    AVFrame* frameRGBA;
    AVPacket* packet;
    SwsContext* swsContext;
    SDL_Texture* videoTexture;
    int videoStreamIndex;
    Uint32 frameDelay;
    uint8_t* rgbaBuffer;
    SDL_Surface* tempSurface;
    
    void processGreenScreenSDL(SDL_Surface* surface);
};

#endif
