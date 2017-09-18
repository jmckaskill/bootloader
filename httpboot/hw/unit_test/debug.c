#include "../debug.h"
#include <stdio.h>

static void put(void *udata, char ch) {
    fputc(ch, (FILE*) udata);
}

void init_debug() {
    init_printf(stderr, &put);
}