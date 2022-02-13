#include "SoakFFmpeg.h"
void *decodeThread(void *data)
{
    auto *fFmpeg = (SoakFFmpeg *) data;
    fFmpeg->decodeFFmpeg();
    pthread_exit(&fFmpeg->decodThread);
}


int SoakFFmpeg::preparedFFmpeg() {
    pthread_create(&decodThread, nullptr, decodeThread, this);
    return 0;
}

SoakFFmpeg::SoakFFmpeg(SoakJavaCall *javaCall, const char *url, bool onlymusic) {
    pthread_mutex_init(&init_mutex, nullptr);
    pthread_mutex_init(&seek_mutex, nullptr);
    exitByUser = false;
    isOnlyMusic = onlymusic;
    soakJavaCall = javaCall;
    urlpath = url;
    soakPlayStatus = new SoakPlayStatus();
}

int avformat_interrupt_cb(void *ctx)
{
    auto *fFmpeg = (SoakFFmpeg *) ctx;
    if(fFmpeg->soakPlayStatus->exit)
    {
        LOGE("avformat_interrupt_cb return 1")
        return AVERROR_EOF;
    }
        LOGE("avformat_interrupt_cb return 0")
    return 0;
}

int SoakFFmpeg::decodeFFmpeg() {
    pthread_mutex_lock(&init_mutex);
    exit = false;
    isavi = false;
   // av_register_all();
    avformat_network_init();
    int width = 0, height = 0;
    int codec_num = 0, codec_den = 0;
    int fps;
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, urlpath, nullptr, nullptr) != 0)
    {
       LOGE("can not open url:%s", urlpath)
        if(soakJavaCall != nullptr)
        {
            soakJavaCall->onError(SOAK_THREAD_CHILD, SOAK_FFMPEG_CAN_NOT_OPEN_URL, "can not open url");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    pFormatCtx->interrupt_callback.callback = avformat_interrupt_cb;
    pFormatCtx->interrupt_callback.opaque = this;

    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {

        LOGE("can not find streams from %s", urlpath);
        if(soakJavaCall != nullptr) {
            soakJavaCall->onError(SOAK_THREAD_CHILD, SOAK_FFMPEG_CAN_NOT_FIND_STREAMS,
                                  "can not find streams from url");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if(pFormatCtx == nullptr)
    {
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    duration = pFormatCtx->duration / 1000000;
    LOGD("channel numbers is %d", pFormatCtx->nb_streams);
    for(int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO )//音频
        {
            LOGE("音频")
            auto *pChannel = new SoakAudioChannel(i, pFormatCtx->streams[i]->time_base);
            audiochannels.push_front(pChannel);
        }
        else if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)//视频
        {
            if(!isOnlyMusic)
            {
                LOGE("视频")
                width = pFormatCtx->streams[i]->codecpar->width;
                height = pFormatCtx->streams[i]->codecpar->height;
                codec_num = pFormatCtx->streams[i]->codecpar->sample_aspect_ratio.num;
                codec_den = pFormatCtx->streams[i]->codecpar->sample_aspect_ratio.den;
                int num = pFormatCtx->streams[i]->avg_frame_rate.num;
                int den = pFormatCtx->streams[i]->avg_frame_rate.den;
                if(num != 0 && den != 0)
                {
                    //int fps = num / den;//[25 / 1]
                     fps = pFormatCtx->streams[i]->avg_frame_rate.num / pFormatCtx->streams[i]->avg_frame_rate.den;
                    auto *pChannel = new SoakAudioChannel(i, pFormatCtx->streams[i]->time_base, fps);
                   // video->defaultDelayTime = 1.0 / fps;
                    videochannels.push_front(pChannel);
                }
            }
        }
    }


    if(audiochannels.size() > 0)
    {
        soakAudio = new SoakAudio(soakPlayStatus, soakJavaCall);
        setAudioChannel(0);
        if(soakAudio->streamIndex >= 0 && soakAudio->streamIndex < pFormatCtx->nb_streams)
        {
            if(getAvCodecContext(pFormatCtx->streams[soakAudio->streamIndex]->codecpar, soakAudio) != 0)
            {
                exit = true;
                pthread_mutex_unlock(&init_mutex);
                return 1;
            }
        }


    }
    if(videochannels.size() > 0)
    {
        soakVideo = new SoakVideo(soakJavaCall, soakAudio, soakPlayStatus);
        setVideoChannel(0);
        if(soakVideo->streamIndex >= 0 && soakVideo->streamIndex < pFormatCtx->nb_streams)
        {
            if(getAvCodecContext(pFormatCtx->streams[soakVideo->streamIndex]->codecpar, soakVideo) != 0)
            {
                exit = true;
                pthread_mutex_unlock(&init_mutex);
                return 1;
            }
        }
    }

    if(soakAudio == nullptr && soakVideo == nullptr)
    {
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return 1;
    }
    if(soakAudio != nullptr)
    {
        soakAudio->duration = pFormatCtx->duration / 1000000;
        soakAudio->sample_rate = soakAudio->avCodecContext->sample_rate;
        if(soakVideo != nullptr)
        {
            soakAudio->setVideo(true);

        }
    }
    if(soakVideo != nullptr)
    {
            LOGE("codec name is %s", soakVideo->avCodecContext->codec->name)
            LOGE("codec long name is %s", soakVideo->avCodecContext->codec->long_name)
        soakVideo->defaultDelayTime = 1.0 / fps;
        LOGE("defaultDelayTime: = %lf", soakVideo->defaultDelayTime)
        if(!soakJavaCall->isOnlySoft(SOAK_THREAD_CHILD))
        {
            mimeType = getMimeType(soakVideo->avCodecContext->codec->name);
        } else{
            mimeType = -1;
        }

        if(mimeType != -1)
        {
            soakJavaCall->onInitMediacodec(SOAK_THREAD_CHILD, mimeType, soakVideo->avCodecContext->width, soakVideo->avCodecContext->height, soakVideo->avCodecContext->extradata_size, soakVideo->avCodecContext->extradata_size, soakVideo->avCodecContext->extradata, soakVideo->avCodecContext->extradata);
        }
        soakVideo->duration = pFormatCtx->duration / 1000000;
    }

    LOGE("准备ing")
    if (soakJavaCall){
        int num ,den;
        av_reduce(&num, &den, width * (int64_t)codec_num, height * (int64_t)codec_den, 1080 * 1920);
        float dar = num *1.0f/den;
        if (dar == 0) dar = 1.7777778f;
        soakJavaCall->onVideoSizeChanged(SOAK_THREAD_CHILD,width,height,dar);
    }
    soakJavaCall->onParpared(SOAK_THREAD_CHILD);
    LOGE("准备end")
    exit = true;
    pthread_mutex_unlock(&init_mutex);
    return 0;
}

int SoakFFmpeg::getAvCodecContext(AVCodecParameters *parameters, SoakBasePlayer *basePlayer) {

    AVCodec *dec = avcodec_find_decoder(parameters->codec_id);
    if(!dec)
    {
        soakJavaCall->onError(SOAK_THREAD_CHILD, 3, "get avcodec fail");
        exit = true;
        return 1;
    }
    basePlayer->avCodecContext = avcodec_alloc_context3(dec);
    if(!basePlayer->avCodecContext)
    {
        soakJavaCall->onError(SOAK_THREAD_CHILD, 4, "alloc avcodecctx fail");
        exit = true;
        return 1;
    }
    if(avcodec_parameters_to_context(basePlayer->avCodecContext, parameters) != 0)
    {
        soakJavaCall->onError(SOAK_THREAD_CHILD, 5, "copy avcodecctx fail");
        exit = true;
        return 1;
    }
    if(avcodec_open2(basePlayer->avCodecContext, dec, 0) != 0)
    {
        soakJavaCall->onError(SOAK_THREAD_CHILD, 6, "open avcodecctx fail");
        exit = true;
        return 1;
    }
    return 0;
}


SoakFFmpeg::~SoakFFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    LOGE("SoakFFmpegeg() 释放了")
}


int SoakFFmpeg::getDuration() {
    return duration;
}

int SoakFFmpeg::start() {
    exit = false;
    int count = 0;
    int ret  = -1;
    if(soakAudio != nullptr)
    {
        soakAudio->playAudio();
    }
    if(soakVideo != nullptr)
    {
        if(mimeType == -1)
        {
            soakVideo->playVideo(0);
        }
        else
        {
            soakVideo->playVideo(1);
        }
    }

    AVBitStreamFilterContext* mimType = nullptr;
    if(mimeType == 1)
    {
        mimType =  av_bitstream_filter_init("h264_mp4toannexb");
    }
    else if(mimeType == 2)
    {
        mimType =  av_bitstream_filter_init("hevc_mp4toannexb");
    }
    else if(mimeType == 3)
    {
        mimType =  av_bitstream_filter_init("h264_mp4toannexb");
    }
    else if(mimeType == 4)
    {
        mimType =  av_bitstream_filter_init("h264_mp4toannexb");
    }

    while(!soakPlayStatus->exit)
    {
        exit = false;
        if(soakPlayStatus->pause)//暂停
        {
            av_usleep(1000 * 100);
            continue;
        }
        if(soakAudio != nullptr && soakAudio->queue->getAvPacketSize() > 100)
        {
//            LOGE("soakAudio 等待..........");
            av_usleep(1000 * 100);
            continue;
        }
        if(soakVideo != nullptr && soakVideo->queue->getAvPacketSize() > 100)
        {
//            LOGE("soakVideo 等待..........");
            av_usleep(1000 * 100);
            continue;
        }
        AVPacket *packet = av_packet_alloc();
        pthread_mutex_lock(&seek_mutex);
        ret = av_read_frame(pFormatCtx, packet);
        pthread_mutex_unlock(&seek_mutex);
        if(soakPlayStatus->seek)
        {
            av_packet_free(&packet);
            av_free(packet);
            continue;
        }
        if(ret == 0)
        {
            if(soakAudio != nullptr && packet->stream_index == soakAudio->streamIndex)
            {
                count++;
                LOGE("解码第 %d 帧", count)
                soakAudio->queue->putAvpacket(packet);
            }else if(soakVideo != nullptr && packet->stream_index == soakVideo->streamIndex)
            {
                if(mimType != nullptr && !isavi)
                {
                    uint8_t *data;
                    //av_bsf_send_packet
                    av_bitstream_filter_filter(mimType, pFormatCtx->streams[soakVideo->streamIndex]->codec, nullptr, &data, &packet->size, packet->data, packet->size, 0);
                    uint8_t *tdata;
                    tdata = packet->data;
                    packet->data = data;
                    if(tdata != nullptr)
                    {
                        av_free(tdata);
                    }
                }
                soakVideo->queue->putAvpacket(packet);
            }
            else{
                av_packet_free(&packet);
                av_free(packet);
                packet = nullptr;
            }
        } else{
            av_packet_free(&packet);
            av_free(packet);
            packet = nullptr;
            if((soakVideo != nullptr && soakVideo->queue->getAvFrameSize() == 0) || (soakAudio != nullptr && soakAudio->queue->getAvPacketSize() == 0))
            {
                soakPlayStatus->exit = true;
                break;
            }
        }
    }
    if(mimType != nullptr)
    {
        av_bitstream_filter_close(mimType);
    }
    if(!exitByUser && soakJavaCall != nullptr)
    {
        soakJavaCall->onComplete(SOAK_THREAD_CHILD);
    }
    exit = true;
    return 0;
}

void SoakFFmpeg::release() {
    soakPlayStatus->exit = true;
    pthread_mutex_lock(&init_mutex);
    LOGE("开始释放 ffmpeg")
    int sleepCount = 0;
    while(!exit)
    {
        if(sleepCount > 300)//十秒钟还没有退出就自动强制退出
        {
            exit = true;
        }
        LOGE("wait ffmpeg  exit %d", sleepCount)
        sleepCount++;
        av_usleep(1000 * 10);//暂停10毫秒
    }
        LOGE("释放audio....................................")
    if(soakAudio != nullptr)
    {
       LOGE("释放audio....................................2")
        soakAudio->release();
        delete(soakAudio);
        soakAudio = nullptr;
    }

        LOGE("释放video....................................")


    if(soakVideo != nullptr)
    {
        LOGE("释放video....................................2")
        soakVideo->release();
        delete(soakVideo);
        soakVideo = nullptr;
    }
     LOGE("释放format...................................")

    if(pFormatCtx != nullptr)
    {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = nullptr;
    }
    LOGE("释放javacall.................................")
    if(soakJavaCall != nullptr)
    {
        soakJavaCall = nullptr;
    }
    pthread_mutex_unlock(&init_mutex);
}

void SoakFFmpeg::pause() {
    if(soakPlayStatus != nullptr)
    {
        soakPlayStatus->pause = true;
        if(soakAudio != nullptr)
        {
            soakAudio->pause();
        }
    }
}

void SoakFFmpeg::resume() {
    if(soakPlayStatus != nullptr)
    {
        soakPlayStatus->pause = false;
        if(soakAudio != nullptr)
        {
            soakAudio->resume();
        }
    }
}

int SoakFFmpeg::getMimeType(const char *codecName) {

    if(strcmp(codecName, "h264") == 0)
    {
        return 1;
    }
    if(strcmp(codecName, "hevc") == 0)
    {
        return 2;
    }
    if(strcmp(codecName, "mpeg4") == 0)
    {
        isavi = true;
        return 3;
    }
    if(strcmp(codecName, "wmv3") == 0)
    {
        isavi = true;
        return 4;
    }

    return -1;
}

int SoakFFmpeg::seek(int64_t sec) {
    if(sec >= duration)
    {
        return -1;
    }
    if(soakPlayStatus->load)
    {
        return -1;
    }
    if(pFormatCtx != nullptr)
    {
        soakPlayStatus->seek = true;
        pthread_mutex_lock(&seek_mutex);
        int64_t rel = sec * AV_TIME_BASE;
        int ret = avformat_seek_file(pFormatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
        if(soakAudio != nullptr)
        {
            soakAudio->queue->clearAvpacket();
//            av_seek_frame(pFormatCtx, soakAudio->streamIndex, sec * soakAudio->time_base.den, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
            soakAudio->setClock(0);
        }
        if(soakVideo != nullptr)
        {
            soakVideo->queue->clearAvFrame();
            soakVideo->queue->clearAvpacket();
//            av_seek_frame(pFormatCtx, soakVideo->streamIndex, sec * soakVideo->time_base.den, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
            soakVideo->setClock(0);
        }
        soakAudio->clock = 0;
        soakAudio->now_time = 0;
        pthread_mutex_unlock(&seek_mutex);
        soakPlayStatus->seek = false;
    }
    return 0;
}

void SoakFFmpeg::setAudioChannel(int index) {
    if(soakAudio != nullptr)
    {
        int channelsize = audiochannels.size();
        if(index < channelsize)
        {
            for(int i = 0; i < channelsize; i++)
            {
                if(i == index)
                {
                    soakAudio->time_base = audiochannels.at(i)->time_base;
                    soakAudio->streamIndex = audiochannels.at(i)->channelId;
                }
            }
        }
    }

}

void SoakFFmpeg::setVideoChannel(int id) {
    if(soakVideo != nullptr)
    {
        soakVideo->streamIndex = videochannels.at(id)->channelId;
        soakVideo->time_base = videochannels.at(id)->time_base;
        soakVideo->rate = 1000 / videochannels.at(id)->fps;
        if(videochannels.at(id)->fps >= 60)
        {
            soakVideo->frameratebig = true;
        } else{
            soakVideo->frameratebig = false;
        }

    }
}

int SoakFFmpeg::getAudioChannels() {
    return audiochannels.size();
}

int SoakFFmpeg::getVideoWidth() {
    if(soakVideo != nullptr && soakVideo->avCodecContext != nullptr)
    {
        return soakVideo->avCodecContext->width;
    }
    return 0;
}

int SoakFFmpeg::getVideoHeight() {
    if(soakVideo != nullptr && soakVideo->avCodecContext != nullptr)
    {
        return soakVideo->avCodecContext->height;
    }
    return 0;
}
