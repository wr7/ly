#include "utils.h"
#include <stdlib.h>
#include <string.h>

#define DOOM_STEPS 13

struct doom_state {
	uint8_t *buf;
};

struct doom_state *doom_init(struct term_buf *buf) {
	struct doom_state *state = malloc_or_throw(sizeof(*state));

	uint16_t tmp_len = buf->width * buf->height;
	state->buf = malloc_or_throw(tmp_len);
	tmp_len -= buf->width;

	memset(state->buf, 0, tmp_len);
	memset(state->buf + tmp_len, DOOM_STEPS - 1, buf->width);

	return state;
}

void doom_free(struct doom_state *state) {
	free(state->buf);
	free(state);
}

void doom(struct doom_state *state, struct term_buf *term_buf) {
	static struct tb_cell fire[DOOM_STEPS] = {
		{' ', 9, 0},    // default
		{0x2591, 2, 0}, // red
		{0x2592, 2, 0}, // red
		{0x2593, 2, 0}, // red
		{0x2588, 2, 0}, // red
		{0x2591, 4, 2}, // yellow
		{0x2592, 4, 2}, // yellow
		{0x2593, 4, 2}, // yellow
		{0x2588, 4, 2}, // yellow
		{0x2591, 8, 4}, // white
		{0x2592, 8, 4}, // white
		{0x2593, 8, 4}, // white
		{0x2588, 8, 4}, // white
	};

	uint16_t src;
	uint16_t random;
	uint16_t dst;

	uint16_t w = term_buf->init_width;
	uint8_t *tmp = state->buf;

	struct tb_cell *buf = tb_cell_buffer();

	for(uint16_t x = 0; x < w; ++x) {
		for(uint16_t y = 1; y < term_buf->init_height; ++y) {
			src = y * w + x;
			random = ((rand() % 7) & 3);
			dst = src - random + 1;

			if(w > dst) {
				dst = 0;
			} else {
				dst -= w;
			}

			tmp[dst] = tmp[src] - (random & 1);

			if(tmp[dst] > 12) {
				tmp[dst] = 0;
			}

			buf[dst] = fire[tmp[dst]];
			buf[src] = fire[tmp[src]];
		}
	}
}
