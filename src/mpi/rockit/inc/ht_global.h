/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                  *
*        *************************************************         *
*        *©2021-2027 HT Corporation All rights reserved *          *
*        *************************************************         *
*                                                                  *
* FileName    : ht_global.h                                        *
*                                                                  *
* Author      : linus                                              *
*                                                                  *
* Email       : luoyaojun@haitutech.com                            *
*                                                                  *
* Date        : 2022-8-8                                           *
*                                                                  *
* Description :                                                    *
*                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __HT_GLOBAL_DEFINE_H
#define __HT_GLOBAL_DEFINE_H

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>

#include "ht_type.h"

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({ \
        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type,member) );})
        
#define __init		__attribute__((constructor))
#define __exit		__attribute__((destructor))
#define __noreturn	__attribute__((noreturn))
#define __unused	__attribute__((__unused__))
#define __inline	__attribute__((always_inline))

#endif
