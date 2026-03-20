/**
 * Copyright (c) 2025 Oray Inc. All rights reserved.
 *
 * No Part of this file may be reproduced, stored
 * in a retrieval system, or transmitted, in any form, or by any means,
 * electronic, mechanical, photocopying, recording, or otherwise,
 * without the prior consent of Oray Inc.
 *
 * @author: LiJun
 * @date: 2025-03-11
 * @fileName: log_tag.h
 * @description: 带模块 TAG 的日志封装
 */

#ifndef _VIDEO_SERVER_LOG_TAG_H_
#define _VIDEO_SERVER_LOG_TAG_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#ifdef ORAY_LOGGER 
#include "logger/phlogger.h"
#else
#define LOG_INFO "[info]"
#define LOG_DEBUG "[debug]"
#define LOG_ERROR "[error]"
#define LOG_WARNING "[warning]"
#ifndef _vscprintf
#define _vscprintf(a,b) 10240
#endif

static void write_log(const char *evt, const char *str, ...) {
    char stime[32] = "";
    time_t now = time(NULL);
    struct tm *tm = localtime(&(now));
    if (tm) {
            strftime(stime, sizeof(stime), "%Y-%m-%d %H:%M:%S", tm);
    }
    char *fmtstr = NULL;
    int len = 0;
    va_list   varg;
    va_start(varg, str);
    len = _vscprintf(str, varg);
    fmtstr = (char *)malloc((size_t)len + 1);
    if (!fmtstr)
        return;
    fmtstr[len] = 0;
    vsnprintf(fmtstr, (size_t)len + 1, str, varg);
    va_end(varg);

    struct timeval us;
    memset(&us, '0', sizeof(struct timeval));
    gettimeofday(&us, NULL);

    printf("%s.%06d%c%s%c%s\n", stime, (int)(us.tv_usec), '\t', evt, '\t', fmtstr);
    free(fmtstr);
}

#endif /* ORAY_LOGGER */

#ifndef MODULE_TAG
#define MODULE_TAG "unknown"
#endif

#ifdef ORAY_LOGGER
#define WriteLog(evt, fmt, ...) \
    flogger::GetInitance().write_log(evt, "[" MODULE_TAG "] " fmt, ##__VA_ARGS__)
#else
#define WriteLog(evt, fmt, ...) \
    write_log(evt, "[" MODULE_TAG "] " fmt, ##__VA_ARGS__)
#endif

#endif /* _VIDEO_SERVER_LOG_TAG_H_ */
