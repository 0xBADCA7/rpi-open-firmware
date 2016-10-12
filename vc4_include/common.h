#pragma once

#include <setjmp.h>
#include "../lib/stdarg.h"

typedef unsigned long long u64;
typedef unsigned long long uint64_t;
typedef long long int64_t;

typedef unsigned int u32;
typedef unsigned int uint32_t;
typedef int int32_t;

typedef unsigned short u16;
typedef unsigned short uint16_t;
typedef short int16_t;

typedef unsigned char u8;
typedef unsigned char uint8_t;

typedef int bool;

#define true 1
#define false 0

#define NULL ((void*)0)

typedef uint32_t size_t;
typedef uint32_t intptr_t;

# define ALWAYS_INLINE __attribute__((always_inline)) inline

#define _OPEN_SOURCE

extern void udelay(uint32_t time);
extern uint32_t __div64_32(uint64_t *n, uint32_t base);

#include "../lib/panic.h"

#define do_div __div64_32

/*
 * this is done like that because clang likes using __builtin_memcpy
 * which makes LLC choke in a fabulous way.
 */
extern void *__memcpy(void *_dst, const void *_src, unsigned len);
#define bcopy(s,d,l) __memcpy(d,s,l)
#define memcpy(d,s,l) __memcpy(d,s,l)

extern int putchar(int c);
extern int vprintf(const char* fmt, va_list arp);
extern int printf(const char *fmt, ...);
extern int puts(const char* str);
extern void *memset(void *s, int c, size_t n);

#define TESTHARNESS_ONLY
extern jmp_buf restart_shell;
