#pragma once

#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>

struct test_eth {
    int sz;
    uint8_t data[1500];
};

extern struct test_eth test_tx[];
extern size_t text_tx_len;

extern unsigned test_tick;

extern int test_tcp();


static inline void check(int *err, int64_t have, int64_t want, const char *test) {
	fprintf(stderr, "  %s: ", test);
	if (have == want) {
		fprintf(stderr, "OK\n");
	} else {
		fprintf(stderr, "FAIL\n    Want: %" PRId64 "\n    Have: %" PRId64 "\n", want, have);
		(*err)++;
	}
}

static inline void check_not_null(int *err, const void *have, const char *test) {
	fprintf(stderr, "  %s: ", test);
	if (have) {
		fprintf(stderr, "OK\n");
	} else {
		fprintf(stderr, "FAIL\n    Want: Not NULL\n    Have: NULL\n");
		(*err)++;
	}
}

static inline void check_null(int *err, const void *have, const char *test) {
	fprintf(stderr, "  %s: ", test);
	if (!have) {
		fprintf(stderr, "OK\n");
	} else {
		fprintf(stderr, "FAIL\n    Want: NULL\n    Have: %p\n", have);
		(*err)++;
	}
}

static inline void check_ptr(int *err, const void *have, const void *want, const char *test) {
	fprintf(stderr, "  %s: ", test);
	if (have != want) {
		fprintf(stderr, "OK\n");
	} else {
		fprintf(stderr, "FAIL\n    Want: %p\n    Have: %p\n", want, have);
		(*err)++;
	}
}

static inline void check_range(int *err, int64_t have, int64_t min, int64_t max, const char *test) {
	fprintf(stderr, "  %s: ", test);
	if (min <= have && have <= max) {
		fprintf(stderr, "OK\n");
	} else {
		fprintf(stderr, "FAIL\n    Want: %" PRId64 " to %" PRId64 "\n    Have: %" PRId64 "\n", min, max, have);
		(*err)++;
	}
}

static inline void print_data(const uint8_t *data, size_t len) {
	for (size_t i = 0; i < len; i++) {
		if ((i & 2) == 0) {
			fputc(' ', stderr);
		}
		fprintf(stderr, "%02X", data[i]);
	}
}

static inline void check_data(int *err, const void *have, const void *want, size_t len, const char *test) {
    uint8_t* h = (uint8_t*) have;
    uint8_t* w = (uint8_t*) want;
	fprintf(stderr, "  %s: ", test);
	for (size_t i = 0; i < len; i++) {
		if (h[i] != w[i]) {
			fprintf(stderr, "FAIL\n    Want:");
			print_data(w, len);
			fprintf(stderr, "\n    Have:");
			print_data(h, len);
			fprintf(stderr, "\n");
			(*err)++;
			return;
		}
	}

	fprintf(stderr, "OK\n");
}

