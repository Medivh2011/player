#ifndef _FFMPEG_CONFIG_H
#define _FFMPEG_CONFIG_H

#if ABI_ARCH_AARCH64
#   include "aarch64/config.h"
#elif ABI_ARCH_ARM
#   include "arm/config.h"
#endif

#endif //_FFMPEG_CONFIG_H