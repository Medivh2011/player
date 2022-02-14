
#include "SoakFFmpeg.h"

SoakFFmpeg::SoakFFmpeg(SoakPlaystatus *status, SoakCallJava *callJava, const char *url) {
    this->playStatus = status;
    this->callJava = callJava;
    this->url = url;
    exit = false;
    pthread_mutex_init(&init_mutex, nullptr);
    pthread_mutex_init(&seek_mutex, nullptr);
}

void *decodeFFmpeg(void *data) {
    auto *fFmpeg = (SoakFFmpeg *) data;
    fFmpeg->decodeFFmpegThread();
    return nullptr;
}

void SoakFFmpeg::prepare() {
    pthread_create(&decodeThread, nullptr, decodeFFmpeg, this);
}

int avformat_callback(void *ctx) {
    auto *fFmpeg = (SoakFFmpeg *) ctx;
    if (fFmpeg->playStatus->exit) {
        return AVERROR_EOF;
    }
    return 0;
}


void SoakFFmpeg::decodeFFmpegThread() {
    pthread_mutex_lock(&init_mutex);
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    pFormatCtx->interrupt_callback.callback = avformat_callback;
    pFormatCtx->interrupt_callback.opaque = this;
    int width = 0, height = 0;
    int codec_num = 0, codec_den = 0;
    if (avformat_open_input(&pFormatCtx, url, nullptr, nullptr) != 0) {
        LOGE("can not open url :%s", url)
        callJava->onCallError(CHILD_THREAD, 1001, "can not open url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        LOGE("can not find streams from %s", url)
        callJava->onCallError(CHILD_THREAD, 1002, "can not find streams from url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)//得到音频流
        {
            if (audio == nullptr) {
                audio = new SoakAudio(playStatus, pFormatCtx->streams[i]->codecpar->sample_rate,
                                      callJava);
                audio->streamIndex = i;
                audio->codecPar = pFormatCtx->streams[i]->codecpar;
                audio->duration = pFormatCtx->duration / AV_TIME_BASE;
                audio->time_base = pFormatCtx->streams[i]->time_base;
                duration = audio->duration;
            }
        } else if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video == nullptr) {
                video = new SoakVideo(playStatus, callJava);
                video->streamIndex = i;
                video->codecPar = pFormatCtx->streams[i]->codecpar;
                video->time_base = pFormatCtx->streams[i]->time_base;
                width = pFormatCtx->streams[i]->codecpar->width;
                height = pFormatCtx->streams[i]->codecpar->height;
                codec_num = pFormatCtx->streams[i]->codecpar->sample_aspect_ratio.num;
                codec_den = pFormatCtx->streams[i]->codecpar->sample_aspect_ratio.den;

                int num = pFormatCtx->streams[i]->avg_frame_rate.num;
                int den = pFormatCtx->streams[i]->avg_frame_rate.den;
                if (num != 0 && den != 0) {
                    int fps = num / den;//[25 / 1]
                    video->defaultDelayTime = 1.0 / fps;
                }
            }
        }
    }

    if (audio != nullptr) {
        getCodecContext(audio->codecPar, &audio->avCodecContext);
    }
    if (video != nullptr) {
        getCodecContext(video->codecPar, &video->avCodecContext);
    }

    if (callJava != nullptr) {
        if (playStatus != nullptr && !playStatus->exit) {
            int num, den;
            float dar;
            av_reduce(&num, &den, width * (int64_t) codec_num, height * (int64_t) codec_den,
                      1080 * 1920);
            dar = num * 1.0f / den;
            callJava->onCallVideoSizeChanged(CHILD_THREAD, width, height, dar);
            callJava->onCallPrepared(CHILD_THREAD);
            LOGE("width=%d, height=%d, num=%d, den=%d", width, height, num, den)
        } else {
            exit = true;
        }
    }
    pthread_mutex_unlock(&init_mutex);
}

void SoakFFmpeg::start() {

    if (video != nullptr) {
        supportMediacodec = false;
        video->audio = audio;
        const char *codecName = ((const AVCodec *) video->avCodecContext->codec)->name;
        supportMediacodec = callJava->onCallIsSupportVideo(codecName);
        if (supportMediacodec) {
            LOGE("当前设备支持硬解码当前视频");
            if (strcasecmp(codecName, "h264") == 0) {
                bsFilter = av_bsf_get_by_name("h264_mp4toannexb");
            } else if (strcasecmp(codecName, "h265") == 0) {
                bsFilter = av_bsf_get_by_name("hevc_mp4toannexb");
            }
            if (bsFilter == nullptr) {
                supportMediacodec = false;
                goto end;
            }
            if (av_bsf_alloc(bsFilter, &video->abs_ctx) != 0) {
                supportMediacodec = false;
                goto end;
            }
            if (avcodec_parameters_copy(video->abs_ctx->par_in, video->codecPar) < 0) {
                supportMediacodec = false;
                av_bsf_free(&video->abs_ctx);
                video->abs_ctx = nullptr;
                goto end;
            }
            if (av_bsf_init(video->abs_ctx) != 0) {
                supportMediacodec = false;
                av_bsf_free(&video->abs_ctx);
                video->abs_ctx = nullptr;
                goto end;
            }
            video->abs_ctx->time_base_in = video->time_base;
        }
        end:
        if (supportMediacodec) {
            video->codecType = CODEC_MEDIACODEC;
            video->callJava->onCallInitMediaCodec(
                    codecName,
                    video->avCodecContext->width,
                    video->avCodecContext->height,
                    video->avCodecContext->extradata_size,
                    video->avCodecContext->extradata_size,
                    video->avCodecContext->extradata,
                    video->avCodecContext->extradata
            );
        }
    }
    if (audio != nullptr) {
        audio->play();
    }
    if (nullptr != video) {
        video->play();
    }

    while (playStatus != nullptr && !playStatus->exit) {
        if (playStatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }

        if (audio->queue->getQueueSize() > 40) {
            av_usleep(1000 * 100);
            continue;
        }
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(pFormatCtx, avPacket) == 0) {
            if (nullptr != audio || nullptr != video) {
                if (avPacket->stream_index == audio->streamIndex) {
                    audio->queue->putAvPacket(avPacket);
                } else if (avPacket->stream_index == video->streamIndex) {
                    video->queue->putAvPacket(avPacket);
                } else {
                    av_packet_free(&avPacket);
                    av_free(avPacket);
                }
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            while (playStatus != nullptr && !playStatus->exit) {
                if (audio != nullptr) {
                    if (audio->queue->getQueueSize() > 0) {
                        av_usleep(1000 * 100);
                        continue;
                    } else {
                        if (!playStatus->seek) {
                            av_usleep(1000 * 100);
                            playStatus->exit = true;
                        }
                        break;
                    }
                }
            }
            break;
        }
    }
    if (callJava != nullptr) {
        callJava->onCallComplete(CHILD_THREAD);
    }
    exit = true;

}

void SoakFFmpeg::pause() {

    if (playStatus != nullptr) {
        playStatus->pause = true;
    }
    if (audio != nullptr) {
        audio->pause();
    }
}

void SoakFFmpeg::resume() {
    if (playStatus != nullptr) {
        playStatus->pause = false;
    }
    if (audio != nullptr) {
        audio->resume();
    }
}

void SoakFFmpeg::release() {

    LOGE("开始释放Ffmpeg")
    playStatus->exit = true;
    pthread_join(decodeThread, nullptr);
    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 300) {
            exit = true;
        }
        LOGE("wait ffmpeg  exit %d", sleepCount);
        sleepCount++;
        av_usleep(1000 * 10);//暂停10毫秒
    }
    LOGE("释放 Audio");
    if (audio != nullptr) {
        audio->release();
        delete (audio);
        audio = nullptr;
    }
    LOGE("释放 video");
    if (video != nullptr) {
        video->release();
        delete (video);
        video = nullptr;
    }
    LOGE("释放 封装格式上下文");
    if (pFormatCtx != nullptr) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = nullptr;
    }
    LOGE("释放 callJava");
    if (callJava != nullptr) {
        callJava = nullptr;
    }
    LOGE("释放 playStatus");
    if (playStatus != nullptr) {
        playStatus = nullptr;
    }
    pthread_mutex_unlock(&init_mutex);
}

SoakFFmpeg::~SoakFFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);
}

void SoakFFmpeg::seek(int64_t seconds) {

    LOGE("seek time %d", seconds)
    if (duration <= 0) {
        return;
    }
    if (seconds >= 0 && seconds <= duration) {
        playStatus->seek = true;
        pthread_mutex_lock(&seek_mutex);
        int64_t rel = seconds * AV_TIME_BASE;
        LOGE("rel time %d", seconds)
        avformat_seek_file(pFormatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
        if (audio != nullptr) {
            audio->queue->clearAvPacket();
            audio->clock = 0;
            audio->last_time = 0;
            pthread_mutex_lock(&audio->codecMutex);
            avcodec_flush_buffers(audio->avCodecContext);
            pthread_mutex_unlock(&audio->codecMutex);
        }
        if (video != nullptr) {
            video->queue->clearAvPacket();
            video->clock = 0;
            pthread_mutex_lock(&video->codecMutex);
            avcodec_flush_buffers(video->avCodecContext);
            pthread_mutex_unlock(&video->codecMutex);
        }
        pthread_mutex_unlock(&seek_mutex);
        playStatus->seek = false;
    }
}

int SoakFFmpeg::getCodecContext(AVCodecParameters *avCodecParameters,
                                AVCodecContext **avCodecContext) {
    AVCodec *dec = avcodec_find_decoder(avCodecParameters->codec_id);
    if (!dec) {

        LOGE("can not find decoder");
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    *avCodecContext = avcodec_alloc_context3(dec);
    if (!*avCodecContext) {

        LOGE("can not alloc new decodecctx");
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if (avcodec_parameters_to_context(*avCodecContext, avCodecParameters) < 0) {
        LOGE("can not fill decodecctx");
        callJava->onCallError(CHILD_THREAD, 1005, "ccan not fill decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if (avcodec_open2(*avCodecContext, dec, 0) != 0) {

        LOGE("cant not open audio strames");
        callJava->onCallError(CHILD_THREAD, 1006, "cant not open audio strames");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    return 0;
}
