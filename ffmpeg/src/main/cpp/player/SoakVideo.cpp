
#include "SoakVideo.h"

SoakVideo::SoakVideo(PlayStatus *playStatus, SoakCallJava *callJava) {

    this->playStatus = playStatus;
    this->callJava = callJava;
    queue = new SoakQueue(playStatus);
    pthread_mutex_init(&codecMutex, nullptr);
}

void *playVideo(void *data)
{
    auto *video = static_cast<SoakVideo *>(data);
    while(video->playStatus != nullptr && !video->playStatus->exit)
    {
        if(video->playStatus->seek)
        {
            av_usleep(1000 * 100);
            continue;
        }
        if(video->playStatus->pause)
        {
            av_usleep(1000 * 100);
            continue;
        }
        if(video->queue->getQueueSize() == 0)
        {
            if(!video->playStatus->load)
            {
                video->playStatus->load = true;
                video->callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else{
            if(video->playStatus->load)
            {
                video->playStatus->load = false;
                video->callJava->onCallLoad(CHILD_THREAD, false);
            }
        }
        AVPacket *avPacket = av_packet_alloc();
        if(video->queue->getAvPacket(avPacket) != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = nullptr;
            continue;
        }
        if(video->codecType == CODEC_MEDIACODEC)
        {
            if(av_bsf_send_packet(video->abs_ctx, avPacket) != 0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = nullptr;
                continue;
            }
            while(av_bsf_receive_packet(video->abs_ctx, avPacket) == 0)
            {
                LOGE("开始解码");

                double diff = video->getFrameDiffTime(nullptr, avPacket);
                LOGE("diff is %f", diff);

                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallDecodeAVPacket(avPacket->size, avPacket->data);

                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }
            avPacket = nullptr;
        }
        else if(video->codecType == CODEC_YUV)
        {
            pthread_mutex_lock(&video->codecMutex);
            if(avcodec_send_packet(video->avCodecContext, avPacket) != 0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = nullptr;
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }
            AVFrame *avFrame = av_frame_alloc();
            if(avcodec_receive_frame(video->avCodecContext, avFrame) != 0)
            {
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = nullptr;
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = nullptr;
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }
            LOGE("子线程解码一个AVframe成功");
            if(avFrame->format == AV_PIX_FMT_YUV420P)
            {
                LOGE("当前视频是YUV420P格式");

                double diff = video->getFrameDiffTime(avFrame, nullptr);
                LOGE("diff is %f", diff);

                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallRenderYUV(
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        avFrame->data[0],
                        avFrame->data[1],
                        avFrame->data[2]);
            } else{
                LOGE("当前视频不是YUV420P格式");
                AVFrame *pFrameYUV420P = av_frame_alloc();
                int num = av_image_get_buffer_size(
                        AV_PIX_FMT_YUV420P,
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        1);
                uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
                av_image_fill_arrays(
                        pFrameYUV420P->data,
                        pFrameYUV420P->linesize,
                        buffer,
                        AV_PIX_FMT_YUV420P,
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        1);
                SwsContext *sws_ctx = sws_getContext(
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        video->avCodecContext->pix_fmt,
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        AV_PIX_FMT_YUV420P,
                        SWS_BICUBIC, nullptr, nullptr, nullptr);

                if(!sws_ctx)
                {
                    av_frame_free(&pFrameYUV420P);
                    av_free(pFrameYUV420P);
                    av_free(buffer);
                    pthread_mutex_unlock(&video->codecMutex);
                    continue;
                }
                sws_scale(
                        sws_ctx,
                        reinterpret_cast<const uint8_t *const *>(avFrame->data),
                        avFrame->linesize,
                        0,
                        avFrame->height,
                        pFrameYUV420P->data,
                        pFrameYUV420P->linesize);
                //渲染

                double diff = video->getFrameDiffTime(avFrame, nullptr);
                LOGE("diff is %f", diff);

                av_usleep(video->getDelayTime(diff) * 1000000);

                video->callJava->onCallRenderYUV(
                        video->avCodecContext->width,
                        video->avCodecContext->height,
                        pFrameYUV420P->data[0],
                        pFrameYUV420P->data[1],
                        pFrameYUV420P->data[2]);

                av_frame_free(&pFrameYUV420P);
                av_free(pFrameYUV420P);
                av_free(buffer);
                sws_freeContext(sws_ctx);
            }
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = nullptr;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = nullptr;
            pthread_mutex_unlock(&video->codecMutex);
        }

    }
    return nullptr;
}

void SoakVideo::play() {

    if(playStatus != nullptr && !playStatus->exit)
    {
        pthread_create(&thread_play, nullptr, playVideo, this);
    }
}

void SoakVideo::release() {

    if(queue != nullptr)
    {
        queue->noticeQueue();
    }
    pthread_join(thread_play, nullptr);

    if(queue != nullptr)
    {
        delete(queue);
        queue = nullptr;
    }
    if(abs_ctx != nullptr)
    {
        av_bsf_free(&abs_ctx);
        abs_ctx = nullptr;
    }
    if(avCodecContext != nullptr)
    {
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = nullptr;
        pthread_mutex_unlock(&codecMutex);
    }

    if(playStatus != nullptr)
    {
        playStatus = nullptr;
    }
    if(callJava != nullptr)
    {
        callJava = nullptr;
    }

}

SoakVideo::~SoakVideo() {
    pthread_mutex_destroy(&codecMutex);
}

double SoakVideo::getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket) {

    double pts = 0;
    if(avFrame != nullptr)
    {
        pts = avFrame->pts;
                //av_frame_get_best_effort_timestamp(avFrame);
    }
    if(avPacket != nullptr)
    {
        pts = avPacket->pts;
    }
    if(pts == AV_NOPTS_VALUE)
    {
        pts = 0;
    }
    pts *= av_q2d(time_base);

    if(pts > 0)
    {
        clock = pts;
    }

    double diff = audio->clock - clock;
    return diff;
}

double SoakVideo::getDelayTime(double diff) {

    if(diff > 0.003)
    {
        delayTime = delayTime * 2 / 3;
        if(delayTime < defaultDelayTime / 2)
        {
            delayTime = defaultDelayTime * 2 / 3;
        }
        else if(delayTime > defaultDelayTime * 2)
        {
            delayTime = defaultDelayTime * 2;
        }
    }
    else if(diff < - 0.003)
    {
        delayTime = delayTime * 3 / 2;
        if(delayTime < defaultDelayTime / 2)
        {
            delayTime = defaultDelayTime * 2 / 3;
        }
        else if(delayTime > defaultDelayTime * 2)
        {
            delayTime = defaultDelayTime * 2;
        }
    }
    else if(diff == 0.003)
    {

    }
    if(diff >= 0.5)
    {
        delayTime = 0;
    }
    else if(diff <= -0.5)
    {
        delayTime = defaultDelayTime * 2;
    }

    if(fabs(diff) >= 10)
    {
        delayTime = defaultDelayTime;
    }
    return delayTime;
}
