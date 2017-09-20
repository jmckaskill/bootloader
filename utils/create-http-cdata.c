#include "../zlib-1.2.11/zlib.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifdef _MSC_VER
#define strncasecmp(a,b,n) _strnicmp(a,b,n)
#endif

#define GZIP_ENCODING 16

static void print_data(FILE *w, const uint8_t *buf, int sz) {
    while (sz) {
        int lsz = min(16,sz);
        fprintf(w, "\n\"");
        for (int i = 0; i < lsz; i++) {
            fprintf(w, "\\x%02x", buf[i]);
        }
        fprintf(w, "\"");

        fprintf(w, "/* ");
        for (int i = 0; i < lsz; i++) {
            if (' ' <= buf[i] && buf[i] < 0x7F && buf[i] != '*') {
                fputc(buf[i], w);
            } else {
                fputc('.', w);
            }
        }
        fprintf(w, " */");

        buf += lsz;
        sz -= lsz;
    }
}

struct buffer {
    uint8_t *p;
    int sz;
};

static void buffer_add(struct buffer *b, const uint8_t *buf, int sz) {
    b->p = (uint8_t*) realloc(b->p, b->sz + sz);
    memcpy(b->p + b->sz, buf, sz);
    b->sz += sz;
}

static int do_deflate(struct buffer *b, z_stream *stream, int flush) {
	static uint8_t out[1024*1024];
	stream->next_out = out;
	stream->avail_out = sizeof(out);
	int	err = deflate(stream, flush);
	if (err && err != Z_STREAM_END) {
		fprintf(stderr, "zlib error %d\n", err);
		exit(1);
	}

    buffer_add(b, out, stream->next_out - out);
	return err;
}

static void deflate_file(struct buffer *b, z_stream *stream, FILE *in) {
	while (!feof(in)) {
		static uint8_t buf[1024 * 1024];
		int n = fread(buf, 1, sizeof(buf)-1, in);
		if (n < 0) {
			fprintf(stderr, "read error\n");
			exit(1);
		}

		stream->next_in = buf;
		stream->avail_in = n;

		while (stream->avail_in) {
            do_deflate(b, stream, Z_NO_FLUSH);
		}
	}

	while (do_deflate(b, stream, Z_FINISH) != Z_STREAM_END) {
	}
}

static void to_csymbol(char *buf, const char *fn, int fsz) {
    *(buf++) = 'g';
    *(buf++) = '_';
    for (int i = 0; i < fsz; i++) {
        if (('a' <= fn[i] && fn[i] <= 'z')
        || ('A' <= fn[i] && fn[i] <= 'Z')
        || ('0' <= fn[i] && fn[i] <= '9')) {
            *(buf++) = fn[i];
        } else {
            *(buf++) = '_';
        }
    }
    *(buf++) = '\0';
}

static int ends_with(const char *str, const char *test) {
    size_t strn = strlen(str);
    size_t testn = strlen(test);
    return strn >= testn && !strncasecmp(str + strn - testn, test, testn);
}

static int is_text(const char *filename) {
    if (ends_with(filename, ".html")) {
        return 1;
    } else if (ends_with(filename, ".js")) {
        return 1;
    } else if (ends_with(filename, ".css")) {
        return 1;
    } else {
        return 0;
    }
}

static const char *content_type(const char *filename) {
    if (ends_with(filename, ".html")) {
        return "text/html";
    } else if (ends_with(filename, ".js")) {
        return "application/json";
    } else if (ends_with(filename, ".css")) {
        return "text/css";
    } else if (ends_with(filename, ".svg")) {
        return "image/svg+xml";
    } else if (ends_with(filename, ".ttf")) {
        return "font/opentype";
    } else {
        return "application/data";
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4 || strcmp(argv[2], "-o")) {
        fprintf(stderr, "%s <in> -o <out>\n", argv[0]);
        return 2;
    }

    const char *in = argv[1];
    const char *out = argv[3];

    FILE *w = fopen(out, "w");
    if (!w) {
        fprintf(stderr, "failed to open %s\n", out);
        return 3;
    }

    FILE *r = fopen(in, is_text(in) ? "r" : "rb");
    if (!r) {
        fprintf(stderr, "failed to open %s\n", in);
        return 4;
    }

    char csym[32+3/*g_\0*/];
    to_csymbol(csym, in, min(strlen(in), 32));


    struct buffer b = {0};
    z_stream stream = {0};
    deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 15+GZIP_ENCODING, 9, Z_DEFAULT_STRATEGY);
    deflate_file(&b, &stream, r);
    deflateEnd(&stream);


    char hdr[128];
    int hsz = sprintf(hdr,
        "HTTP/1.1 OK\r\n"
        "Content-Type:%s\r\n"
        "Content-Encoding:gzip\r\n"
        "Content-Length:%d\r\n"
        "\r\n",
        content_type(in),
        b.sz);

    fprintf(w, "#ifdef __arm__\n");
    fprintf(w, "#define ALIGNED __attribute((aligned(4)))\n");
    fprintf(w, "#endif\n");
    fprintf(w, "extern unsigned long %s_sz;\n", csym);
    fprintf(w, "extern const char %s[];\n", csym);
    fprintf(w, "unsigned long %s_sz = %d;\n", csym, hsz + b.sz);
    fprintf(w, "const char ALIGNED %s[] = \"\"", csym);
    print_data(w, (uint8_t*) hdr, hsz);
    print_data(w, b.p, b.sz);
    fprintf(w, ";\n\n");

    free(b.p);
    fclose(w);
    fclose(r);
    return 0;
}
