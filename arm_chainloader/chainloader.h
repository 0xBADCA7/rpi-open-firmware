#pragma once

#include <stdint.h>
#include <lib/xprintf.h>

#define printf xprintf

#ifdef __cplusplus
extern "C" {
#endif

extern void udelay(uint32_t time);

#define STATIC_INIT_PRIORITY(x) __attribute__((init_priority(x)))

#define STATIC_CPRMAN_DRIVER STATIC_INIT_PRIORITY(101)
#define STATIC_DRIVER STATIC_INIT_PRIORITY(200)
#define STATIC_APP STATIC_INIT_PRIORITY(600)

#define mfence() __sync_synchronize()

#ifdef __cplusplus
}
#endif

