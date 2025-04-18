#include "SceneManager.hpp"
#include <iostream>

SceneManager::SceneManager(SDL_Renderer* renderer) : renderer(renderer) {
    backgroundColor = {255, 255, 255, 255};
    formatContext = nullptr;
    codecContext = nullptr;
    frame = nullptr;
    frameYUV = nullptr;
    packet = nullptr;
    swsContext = nullptr;
    videoTexture = nullptr;
    currentSceneType = SceneType::STATIC;
}

SceneManager::~SceneManager() {
    cleanupVideo();
}

void SceneManager::cleanupVideo() {
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

bool SceneManager::initializeVideo(const std::string& path) {
    cleanupVideo();

    if(avformat_open_input(&formatContext, path.c_str(), nullptr, nullptr) < 0) {
        std::cout << "Could not open file" << std::endl;
        return false;
    }

    if(avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cout << "Could not find stream information" << std::endl;
        return false;
    }

    // Находим видео поток
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

    // Получаем кодек
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

    // Добавляем эти строки для выделения буферов для YUV frame
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

    // Меняем расчет задержки между кадрами на более точный
    AVRational fps = av_guess_frame_rate(formatContext, formatContext->streams[videoStreamIndex], nullptr);
    if (fps.num && fps.den) {
        frameDelay = 1000.0 * fps.den / fps.num; // Конвертируем в миллисекунды
        std::cout << "Video FPS: " << fps.num / (double)fps.den << " (delay: " << frameDelay << "ms)" << std::endl;
    } else {
        frameDelay = 1000.0 / 25.0; // Fallback на 25 FPS если не удалось определить
    }

    nextFrameTime = SDL_GetTicks();
    
    return true;
}

bool SceneManager::loadScene(const std::string& sceneName) {
    std::string filePath = gamePath + "/scenes/" + sceneName + ".json";
    std::ifstream file(filePath);
    
    if (!file.is_open()) {
        std::cout << "Failed to open scene file: " << filePath << std::endl;
        return false;
    }

    try {
        file >> currentScene;
        
        std::string sceneType = currentScene["type"];
        if(sceneType == "video") {
            currentSceneType = SceneType::VIDEO;
            loadVideoScene(currentScene);
        } else {
            currentSceneType = SceneType::STATIC;
            loadStaticScene(currentScene);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cout << "Error parsing scene file: " << e.what() << std::endl;
        return false;
    }
}

void SceneManager::loadVideoScene(const json& sceneData) {
    std::string videoPath = gamePath + "/video/" + sceneData["videoFile"].get<std::string>();
    nextSceneName = sceneData["nextScene"];
    
    if(!initializeVideo(videoPath)) {
        std::cout << "Failed to initialize video: " << videoPath << std::endl;
        loadScene(nextSceneName);
    }
}

bool SceneManager::decodeNextFrame() {
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

void SceneManager::loadStaticScene(const json& sceneData) {
    auto bgColor = sceneData["backgroundColor"];
    backgroundColor.r = bgColor["r"];
    backgroundColor.g = bgColor["g"];
    backgroundColor.b = bgColor["b"];
    backgroundColor.a = bgColor["a"];
}

void SceneManager::update() {
    if(currentSceneType == SceneType::VIDEO) {
        Uint32 currentTime = SDL_GetTicks();
        if(currentTime >= nextFrameTime) {
            if(!decodeNextFrame()) {
                loadScene(nextSceneName);
                return;
            }
            nextFrameTime = currentTime + frameDelay;
            
            // Если мы сильно отстаем, сбрасываем таймер
            if (currentTime > nextFrameTime + frameDelay * 2) {
                nextFrameTime = currentTime;
            }
        }
    }
}

void SceneManager::render() {
    if(currentSceneType == SceneType::VIDEO && videoTexture) {
        SDL_RenderCopy(renderer, videoTexture, nullptr, nullptr);
    } else {
        SDL_SetRenderDrawColor(renderer, 
            backgroundColor.r, 
            backgroundColor.g, 
            backgroundColor.b, 
            backgroundColor.a);
        SDL_RenderClear(renderer);
    }
}
