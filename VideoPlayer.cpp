#include "VideoPlayer.hpp"
#include <iostream>

VideoPlayer::VideoPlayer(SDL_Renderer* renderer) : renderer(renderer) {
    formatContext = nullptr;
    codecContext = nullptr;
    frame = nullptr;
    frameRGBA = nullptr;
    packet = nullptr;
    swsContext = nullptr;
    videoTexture = nullptr;
    rgbaBuffer = nullptr;
    tempSurface = nullptr;
}

VideoPlayer::~VideoPlayer() {
    cleanup();
}

void VideoPlayer::cleanup() {
    if(swsContext) sws_freeContext(swsContext);
    if(frameRGBA) av_frame_free(&frameRGBA);
    if(frame) av_frame_free(&frame);
    if(packet) av_packet_free(&packet);
    if(codecContext) avcodec_free_context(&codecContext);
    if(formatContext) avformat_close_input(&formatContext);
    if(videoTexture) SDL_DestroyTexture(videoTexture);
    if(rgbaBuffer) av_free(rgbaBuffer);
    if(tempSurface) SDL_FreeSurface(tempSurface);
    
    swsContext = nullptr;
    frameRGBA = nullptr;
    frame = nullptr;
    packet = nullptr;
    codecContext = nullptr;
    formatContext = nullptr;
    videoTexture = nullptr;
    rgbaBuffer = nullptr;
    tempSurface = nullptr;
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
    frameRGBA = av_frame_alloc();
    packet = av_packet_alloc();

    // Используем временную поверхность SDL для более точной обработки хромакея
    tempSurface = SDL_CreateRGBSurface(
        0, codecContext->width, codecContext->height, 32,
        0x00FF0000, // R
        0x0000FF00, // G
        0x000000FF, // B
        0xFF000000  // A
    );
    
    if (!tempSurface) {
        std::cout << "Failed to create surface: " << SDL_GetError() << std::endl;
        return false;
    }

    // Allocate RGBA buffer
    int rgbaBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, codecContext->width, codecContext->height, 1);
    rgbaBuffer = (uint8_t*)av_malloc(rgbaBytes);
    av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, rgbaBuffer, AV_PIX_FMT_RGBA,
                        codecContext->width, codecContext->height, 1);

    swsContext = sws_getContext(
        codecContext->width, codecContext->height, codecContext->pix_fmt,
        codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    AVRational fps = av_guess_frame_rate(formatContext, formatContext->streams[videoStreamIndex], nullptr);
    if (fps.num && fps.den) {
        frameDelay = 1000.0 * fps.den / fps.num;
    } else {
        frameDelay = 1000.0 / 25.0;
    }

    return true;
}

void VideoPlayer::processGreenScreenSDL(SDL_Surface* surface) {
    // Убедимся, что формат поверхности соответствует ожидаемому
    SDL_LockSurface(surface);
    
    // Получаем указатель на пиксельные данные
    Uint32* pixels = (Uint32*)surface->pixels;
    int pixelCount = surface->w * surface->h;
    
    // Получаем маски для извлечения компонентов из пикселя
    Uint32 rMask = surface->format->Rmask;
    Uint32 gMask = surface->format->Gmask;
    Uint32 bMask = surface->format->Bmask;
    Uint32 aMask = surface->format->Amask;
    
    // Получаем смещения для масок
    int rShift = surface->format->Rshift;
    int gShift = surface->format->Gshift;
    int bShift = surface->format->Bshift;
    int aShift = surface->format->Ashift;
    
    // Для поверхности формата с альфа-каналом
    for (int i = 0; i < pixelCount; i++) {
        Uint32 pixel = pixels[i];
        
        // Извлекаем компоненты
        Uint8 r = (pixel & rMask) >> rShift;
        Uint8 g = (pixel & gMask) >> gShift;
        Uint8 b = (pixel & bMask) >> bShift;
        
        // Сверхагрессивное определение зеленого фона
        if ((g > 60 && g > r * 1.1 && g > b * 1.1) ||  // Зеленый
            (b > 180 && b > r * 1.2 && b > g * 1.2)) { // Синий фон
            
            // Создаем полностью прозрачный пиксель (R=0,G=0,B=0,A=0)
            // Это намного эффективнее, чем использование цветового ключа
            pixels[i] = 0;
        }
        else {
            // Обеспечиваем полную непрозрачность для не-зеленых пикселей
            pixels[i] = (pixel & ~aMask) | aMask;
        }
    }
    
    SDL_UnlockSurface(surface);
}

bool VideoPlayer::decodeNextFrame() {
    while(av_read_frame(formatContext, packet) >= 0) {
        if(packet->stream_index == videoStreamIndex) {
            if(avcodec_send_packet(codecContext, packet) == 0) {
                if(avcodec_receive_frame(codecContext, frame) == 0) {
                    // Конвертация в RGBA
                    sws_scale(swsContext,
                        frame->data, frame->linesize, 0, codecContext->height,
                        frameRGBA->data, frameRGBA->linesize);
                    
                    // Обновляем данные в SDL_Surface
                    SDL_LockSurface(tempSurface);
                    
                    // Копируем данные из RGBA буфера в поверхность SDL
                    uint8_t* src = frameRGBA->data[0];
                    uint8_t* dst = (uint8_t*)tempSurface->pixels;
                    
                    for (int i = 0; i < codecContext->height; i++) {
                        memcpy(dst, src, codecContext->width * 4);
                        src += frameRGBA->linesize[0];
                        dst += tempSurface->pitch;
                    }
                    
                    SDL_UnlockSurface(tempSurface);
                    
                    // Обрабатываем зеленый экран
                    processGreenScreenSDL(tempSurface);
                    
                    // НЕ используем color key - он может быть причиной проблем
                    // Создаем текстуру с поддержкой альфа-канала
                    if (videoTexture) {
                        SDL_DestroyTexture(videoTexture);
                    }
                    
                    videoTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
                    if (!videoTexture) {
                        std::cout << "Failed to create texture: " << SDL_GetError() << std::endl;
                        av_packet_unref(packet);
                        return false;
                    }
                    
                    // Устанавливаем режим смешивания для прозрачности
                    SDL_SetTextureBlendMode(videoTexture, SDL_BLENDMODE_BLEND);
                    
                    av_packet_unref(packet);
                    return true;
                }
            }
        }
        av_packet_unref(packet);
    }
    return false;
}
