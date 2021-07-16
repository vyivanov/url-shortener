#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>

typedef struct opaque* xkcd_comic;

const xkcd_comic XKCD_COMIC_NULL = (xkcd_comic) 0;

xkcd_comic xkcd_comic_latest();
xkcd_comic xkcd_comic_random();
xkcd_comic xkcd_comic_exact(uint32_t number);

const char* xkcd_comic_error();

typedef struct {
    uint32_t    number;
    time_t      date;
    const char* url;
    const char* title;
    const char* alt;
} xkcd_info;

xkcd_info xkcd_comic_info(xkcd_comic idx);
void xkcd_comic_destroy(xkcd_comic idx);

#ifdef __cplusplus
}
#endif
