#include "SoakVideo.h"

SoakVideo::SoakVideo(SoakJavaCall *javaCall, SoakAudio *audio, SoakPlayStatus *playStatus) {
    streamIndex = -1;
    clock = 0;
    soakJavaCall = javaCall;
    soakAudio = audio;
    queue = new SoakQueue(playStatus);
    soakPlayStatus = playStatus;
}

void SoakVideo::release() {
     LOGE("开始释放audio ...")
    if(soakPlayStatus != nullptr)
    {
        soakPlayStatus->exit = true;
    }
    if(queue != nullptr)
    {
        queue->noticeThread();
    }
    int count = 0;
    while(!isExit || !isExit2)
    {
        LOGE("等待渲染线程结束...%d", count)
        if(count > 1000)
        {
            isExit = true;
            isExit2 = true;
        }
        count++;
        av_usleep(1000 * 10);
    }
    if(queue != nullptr)
    {
        queue->release();
        delete(queue);
        queue = nullptr;
    }
    if(soakJavaCall != nullptr)
    {
        soakJavaCall = nullptr;
    }
    if(soakAudio != nullptr)
    {
        soakAudio = nullptr;
    }
    if(avCodecContext != nullptr)
    {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = nullptr;
    }
    if(soakPlayStatus != nullptr)
    {
        soakPlayStatus = nullptr;
    }
}

void *decodeVideoT(void *data)
{
    auto *video = (SoakVideo *) data;
    video->decodeVideo();
    pthread_exit(&video->videoThread);

}

void *codecFrame(void *data)
{
    auto *pVideo = (SoakVideo *) data;

    while(!pVideo->soakPlayStatus->exit)
    {
        if(pVideo->soakPlayStatus->seek)
        {
            continue;
        }
        pVideo->isExit2 = false;
        if(pVideo->queue->getAvFrameSize() > 20)
        {
            continue;
        }
        if(pVideo->codecType == 1)
        {
            if(pVideo->queue->getAvPacketSize() == 0)//加载
            {
                if(!pVideo->soakPlayStatus->load)
                {
                    pVideo->soakJavaCall->onLoad(SOAK_THREAD_CHILD, true);
                    pVideo->soakPlayStatus->load = true;
                }
                continue;
            } else{
                if(pVideo->soakPlayStatus->load)
                {
                    pVideo->soakJavaCall->onLoad(SOAK_THREAD_CHILD, false);
                    pVideo->soakPlayStatus->load = false;
                }
            }
        }
        AVPacket *packet = av_packet_alloc();
        if(pVideo->queue->getAvpacket(packet) != 0)
        {
            av_packet_free(&packet);
            av_free(packet);
            packet = nullptr;
            continue;
        }

        int ret = avcodec_send_packet(pVideo->avCodecContext, packet);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            av_packet_free(&packet);
            av_free(packet);
            packet = nullptr;
            continue;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(pVideo->avCodecContext, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            av_frame_free(&frame);
            av_free(frame);
            frame = nullptr;
            av_packet_free(&packet);
            av_free(packet);
            packet = nullptr;
            continue;
        }
        pVideo->queue->putAvframe(frame);
        av_packet_free(&packet);
        av_free(packet);
        packet = nullptr;
    }
    pVideo->isExit2 = true;
    pthread_exit(&pVideo->decFrame);
}


void SoakVideo::playVideo(int type) {
    codecType = type;
    if(codecType == 0)
    {
        pthread_create(&decFrame, nullptr, codecFrame, this);
    }
    pthread_create(&videoThread, nullptr, decodeVideoT, this);

}

void SoakVideo::decodeVideo() {
    while(!soakPlayStatus->exit)
    {
        isExit = false;
        if(soakPlayStatus->pause)//暂停
        {
            continue;
        }
        if(soakPlayStatus->seek)
        {
            soakJavaCall->onLoad(SOAK_THREAD_CHILD, true);
            soakPlayStatus->load = true;
            continue;
        }
        if(queue->getAvPacketSize() == 0)//加载
        {
            if(!soakPlayStatus->load)
            {
                soakJavaCall->onLoad(SOAK_THREAD_CHILD, true);
                soakPlayStatus->load = true;
            }
            continue;
        } else{
            if(soakPlayStatus->load)
            {
                soakJavaCall->onLoad(SOAK_THREAD_CHILD, false);
                soakPlayStatus->load = false;
            }
        }
        if(codecType == 1)
        {
            AVPacket *packet = av_packet_alloc();
            if(queue->getAvpacket(packet) != 0)
            {
                av_free(packet->data);
                av_free(packet->buf);
                av_free(packet->side_data);
                continue;
            }
            double time = packet->pts * av_q2d(time_base);
            LOGE("video clock is %f", time)
            LOGE("audio clock is %f", soakAudio->clock)
            if(time < 0)
            {
                time = packet->dts * av_q2d(time_base);
            }

            if(time < clock)
            {
                time = clock;
            }
            clock = time;
            double diff = 0;
            if(soakAudio != nullptr)
            {
                diff = soakAudio->clock - clock;
            }
            playcount++;
            if(playcount > 500)
            {
                playcount = 0;
            }
            if(diff >= 0.5)
            {
                if(frameratebig)
                {
                    if(playcount % 3 == 0 && packet->flags != AV_PKT_FLAG_KEY)
                    {
                        av_free(packet->data);
                        av_free(packet->buf);
                        av_free(packet->side_data);
                        continue;
                    }
                } else{
                    av_free(packet->data);
                    av_free(packet->buf);
                    av_free(packet->side_data);
                    continue;
                }
            }

            delayTime = getDelayTime(diff);
            LOGE("delay time %f diff is %f", delayTime, diff)
            av_usleep(delayTime * 1000);
            soakJavaCall->onVideoInfo(SOAK_THREAD_CHILD, clock, duration);
            soakJavaCall->onDecMediacodec(SOAK_THREAD_CHILD, packet->size, packet->data, 0);
            av_free(packet->data);
            av_free(packet->buf);
            av_free(packet->side_data);
        }
        else if(codecType == 0)
        {
            AVFrame *frame = av_frame_alloc();
            if(queue->getAvframe(frame) != 0)
            {
                av_frame_free(&frame);
                av_free(frame);
                frame = nullptr;
                continue;
            }
            if ((framePts = av_frame_get_best_effort_timestamp(frame)) == AV_NOPTS_VALUE)
            {
               framePts = 0;
            }
            framePts *= av_q2d(time_base);
            clock = synchronize(frame, framePts);
            double diff = 0;
            if(soakAudio != nullptr)
            {
                diff = soakAudio->clock - clock;
            }
            delayTime = getDelayTime(diff);
            LOGE("delay time %f diff is %f", delayTime, diff)
//            if(diff >= 0.8)
//            {
//                av_frame_free(&frame);
//                av_free(frame);
//                frame = nullptr;
//                continue;
//            }

            playcount++;
            if(playcount > 500)
            {
                playcount = 0;
            }
            if(diff >= 0.5)
            {
                if(frameratebig)
                {
                    if(playcount % 3 == 0)
                    {
                        av_frame_free(&frame);
                        av_free(frame);
                        frame = nullptr;
                        queue->clearToKeyFrame();
                        continue;
                    }
                } else{
                    av_frame_free(&frame);
                    av_free(frame);
                    frame = nullptr;
                    queue->clearToKeyFrame();
                    continue;
                }
            }

            av_usleep(delayTime * 1000);
            soakJavaCall->onVideoInfo(SOAK_THREAD_CHILD, clock, duration);
            soakJavaCall->onGlRenderYuv(SOAK_THREAD_CHILD, frame->linesize[0], frame->height, frame->data[0], frame->data[1], frame->data[2]);
            av_frame_free(&frame);
            av_free(frame);
            frame = nullptr;
        }
    }
    isExit = true;

}

SoakVideo::~SoakVideo() {
    LOGE("video s释放完")
}

double SoakVideo::synchronize(AVFrame *srcFrame, double pts) {
    double frame_delay;

    if (pts != 0)
        video_clock = pts; // Get pts,then set video clock to it
    else
        pts = video_clock; // Don't get pts,set it to video clock

    frame_delay = av_q2d(time_base);
    frame_delay += srcFrame->repeat_pict * (frame_delay * 0.5);

    video_clock += frame_delay;

    return pts;
}

double SoakVideo::getDelayTime(double diff) {
    LOGD("audio video diff is %f", diff)
    if(diff > 0.003)
    {
        delayTime = delayTime / 3 * 2;
        if(delayTime < rate / 2)
        {
            delayTime = rate / 3 * 2;
        }
        else if(delayTime > rate * 2)
        {
            delayTime = rate * 2;
        }

    }
    else if(diff < -0.003)
    {
        delayTime = delayTime * 3 / 2;
        if(delayTime < rate / 2)
        {
            delayTime = rate / 3 * 2;
        }
        else if(delayTime > rate * 2)
        {
            delayTime = rate * 2;
        }
    }else if(diff == 0)
    {
        delayTime = rate;
    }
    if(diff > 1.0)
    {
        delayTime = 0;
    }
    if(diff < -1.0)
    {
        delayTime = rate * 2;
    }
    if(fabs(diff) > 10)
    {
        delayTime = rate;
    }
    return delayTime;
}

void SoakVideo::setClock(int secds) {
    clock = secds;
}



