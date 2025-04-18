#include "VideoPlayer.hpp"
#include <iostream>

VideoPlayer::VideoPlayer(SDL_Renderer* renderer) : renderer(renderer) {
    formatContext = nullptr;
    codecContext = nullptr;
    frame = nullptr;
    frameYUV = nullptr;
    packet = nullptr;
    swsContext = nullptr;
    videoTexture = nullptr;
}

VideoPlayer::~VideoPlayer() {
    cleanup();
}

void VideoPlayer::cleanup() {
    if(swsContext) sws_freeContext(swsContext);
    if(frameYUV) av_frame_free(&frameYUV);
    if(frame) av_frame_free(&frame);
    if(packet) av_packet_free(&packet);
    if(codecContext) avcodec_free_context(&codecContext);
    if(formatContext) avformat_close_input(&formatContext);
    if(videoTexture) SDL_DestroyTexture(videoTexture);
    
    swsContext = nullptr;
    frameYUV = nullptr;
    frame = nullptr;
    packet = nullptr;
    codecContext = nullptr;
    formatContext = nullptr;
    videoTexture = nullptr;
}

bool VideoPlayer::initialize(const std::string& path) {
    cleanup();

    if(avformat_open_input(&formatContext, path.c_str(), nullptr, nullptr) < 0) {
        std::cout << "Could not open file" << std::endl;
        return false;
    }

    if(avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cout << "Could not find stream information" << std::endl;
        return false;
    }

    videoStreamIndex = -1;
    for(unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if(videoStreamIndex == -1) {
        std::cout << "Could not find video stream" << std::endl;
        return false;
    }

    const AVCodec* codec = avcodec_find_decoder(formatContext->streams[videoStreamIndex]->codecpar->codec_id);
    if(!codec) {
        std::cout << "Unsupported codec" << std::endl;
        return false;
    }

    codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar);

    if(avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cout << "Could not open codec" << std::endl;
        return false;
    }

    frame = av_frame_alloc();
    frameYUV = av_frame_alloc();
    packet = av_packet_alloc();

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes);
    av_image_fill_arrays(frameYUV->data, frameYUV->linesize, buffer, AV_PIX_FMT_YUV420P,
                        codecContext->width, codecContext->height, 1);

    videoTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_YV12,
        SDL_TEXTUREACCESS_STREAMING,
        codecContext->width,
        codecContext->height
    );

    swsContext = sws_getContext(
        codecContext->width, codecContext->height, codecContext->pix_fmt,
        codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    AVRational fps = av_guess_frame_rate(formatContext, formatContext->streams[videoStreamIndex], nullptr);
    if (fps.num && fps.den) {
        frameDelay = 1000.0 * fps.den / fps.num;
        std::cout << "Video FPS: " << fps.num / (double)fps.den << " (delay: " << frameDelay << "ms)" << std::endl;
    } else {
        frameDelay = 1000.0 / 25.0;
    }

    return true;
}

bool VideoPlayer::decodeNextFrame() {
    while(av_read_frame(formatContext, packet) >= 0) {
        if(packet->stream_index == videoStreamIndex) {
            if(avcodec_send_packet(codecContext, packet) == 0) {
                if(avcodec_receive_frame(codecContext, frame) == 0) {
                    sws_scale(swsContext,
                        frame->data, frame->linesize, 0, codecContext->height,
                        frameYUV->data, frameYUV->linesize);
                    
                    SDL_UpdateYUVTexture(
                        videoTexture, nullptr,
                        frameYUV->data[0], frameYUV->linesize[0],
                        frameYUV->data[1], frameYUV->linesize[1],
                        frameYUV->data[2], frameYUV->linesize[2]
                    );
                    
                    av_packet_unref(packet);
                    return true;
                }
            }
        }
        av_packet_unref(packet);
    }
    return false;
}
