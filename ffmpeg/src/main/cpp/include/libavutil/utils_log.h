//
// Created by DELL on 2021/11/8.
//

#ifndef AVUTIL_UTILS_LOG_H
#define AVUTIL_UTILS_LOG_H
/**
 * add log
 */
typedef int (*av_utils_log_print)(int level, const char *tag, const char* msg);

void av_set_utils_log_print(av_utils_log_print alp);
void av_get_utils_log_print(av_utils_log_print **alp);
void av_set_utils_log_level(int l);
void _av_utils_log_print(const char *tag, const char* msg);


#endif //AVUTIL_UTILS_LOG_H
