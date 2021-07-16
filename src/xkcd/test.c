#include "xkcd.h"

#include <stdio.h>
#include <time.h>

void print_info(const xkcd_comic comic) {
    const xkcd_info info = xkcd_comic_info(comic);
    printf("number = %d\n"
           "date   = %s\n"
           "url    = %s\n"
           "title  = %s\n"
           "alt    = %s\n"
           "\n", info.number,
                 ctime(&info.date),
                 info.url,
                 info.title,
                 info.alt);
}

int main() {
    xkcd_comic comic = XKCD_COMIC_NULL;

    comic = xkcd_comic_latest();
    if (comic) {
        print_info(comic);
        xkcd_comic_destroy(comic);
    } else {
        printf("%s\n", xkcd_comic_error());
    }

    comic = xkcd_comic_random();
    if (comic) {
        print_info(comic);
        xkcd_comic_destroy(comic);
    } else {
        printf("%s\n", xkcd_comic_error());
    }

    comic = xkcd_comic_exact(100);
    if (comic) {
        print_info(comic);
        xkcd_comic_destroy(comic);
    } else {
        printf("%s\n", xkcd_comic_error());
    }

    return 0;
}
