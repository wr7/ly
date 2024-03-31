#include "animations.h"

#include "config.h"
#include "dragonfail.h"
#include "dragonfail_error.h"
#include "draw.h"
#include "stdio.h"
#include "termbox.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

struct animation {
	void *(*const init)(struct term_buf *buf);
	void (*const free)(void *state);
	void (*const draw)(void *state, struct term_buf *buf);
};

static struct doom_state *doom_init(struct term_buf *buf);
static void doom_free(struct doom_state *state);
static void doom(struct doom_state *state, struct term_buf *term_buf);

static struct matrix_state *matrix_init(struct term_buf *buf);
static void matrix(struct matrix_state *s, struct term_buf *buf);
static void matrix_free(struct matrix_state *state);

static const struct animation ANIMATIONS[] = {
	{
		// Cast `doom_state *` to `void *`
		.init = (void *(*)(struct term_buf *buf))doom_init,
		.free = (void (*)(void *state))doom_free,
		.draw = (void (*)(void *state, struct term_buf *buf))doom,
	},
	{
		// Cast `matrix_state *` to `void *`
		.init = (void *(*)(struct term_buf *buf))matrix_init,
		.free = (void (*)(void *state))matrix_free,
		.draw = (void (*)(void *state, struct term_buf *buf))matrix,
	},
};

// Generic public facing functions //

void animate(struct term_buf *buf) {
	if(config.animation >= ARRAY_LENGTH(ANIMATIONS)) {
		return;
	}

	buf->width = tb_width();
	buf->height = tb_height();
	const struct animation *const animation = &ANIMATIONS[config.animation];

	if((buf->width != buf->init_width) || (buf->height != buf->init_height)) {
		animation->free(buf->animation_state);
		buf->animation_state = animation->init(buf);
	}

	buf->init_height = buf->height;
	buf->init_width = buf->width;

	animation->draw(buf->animation_state, buf);
}

void animation_init(struct term_buf *buf) {
	if(config.animation >= ARRAY_LENGTH(ANIMATIONS)) {
		return;
	}

	buf->init_width = tb_width();
	buf->init_height = tb_height();

	const struct animation *const animation = &ANIMATIONS[config.animation];
	buf->animation_state = animation->init(buf);
}

void animation_free(struct term_buf *buf) {
	if(config.animation >= ARRAY_LENGTH(ANIMATIONS)) {
		return;
	}

	const struct animation *const animation = &ANIMATIONS[config.animation];

	animation->free(buf->animation_state);
}

// Doom animation //

#define DOOM_STEPS 13

struct doom_state {
	uint8_t *buf;
};

static struct doom_state *doom_init(struct term_buf *buf) {
	struct doom_state *state = malloc_or_throw(sizeof(struct doom_state));

	uint16_t tmp_len = buf->width * buf->height;
	state->buf = malloc_or_throw(tmp_len);
	tmp_len -= buf->width;

	memset(state->buf, 0, tmp_len);
	memset(state->buf + tmp_len, DOOM_STEPS - 1, buf->width);

	return state;
}

static void doom_free(struct doom_state *state) {
	free(state->buf);
	free(state);
}

static void doom(struct doom_state *state, struct term_buf *term_buf) {
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

// Matrix animation //

struct matrix_dot {
	int val;
	bool is_head;
};

struct matrix_state {
	struct matrix_dot **grid;
	int *length;
	int *spaces;
	int *updates;
};

// Adapted from cmatrix
static struct matrix_state *matrix_init(struct term_buf *buf) {
	struct matrix_state *s = malloc_or_throw(sizeof(struct matrix_state));

	uint16_t len = buf->height + 1;
	s->grid = malloc_or_throw(sizeof(struct matrix_dot *) * len);

	len = (buf->height + 1) * buf->width;
	(s->grid)[0] = malloc_or_throw(sizeof(struct matrix_dot) * len);

	for(int i = 1; i <= buf->height; ++i) {
		s->grid[i] = s->grid[i - 1] + buf->width;
	}

	s->length = malloc_or_throw(buf->width * sizeof(int));

	s->spaces = malloc_or_throw(buf->width * sizeof(int));

	s->updates = malloc_or_throw(buf->width * sizeof(int));

	if(buf->height <= 3) {
		return s;
	}

	// Initialize grid
	for(int i = 0; i <= buf->height; ++i) {
		for(int j = 0; j <= buf->width - 1; j += 2) {
			s->grid[i][j].val = -1;
		}
	}

	for(int j = 0; j < buf->width; j += 2) {
		s->spaces[j] = (int)rand() % buf->height + 1;
		s->length[j] = (int)rand() % (buf->height - 3) + 3;
		s->grid[1][j].val = ' ';
		s->updates[j] = (int)rand() % 3 + 1;
	}

	return s;
}

// Adapted from cmatrix
static void matrix(struct matrix_state *s, struct term_buf *buf) {
	static int frame = 3;
	const int frame_delay = 8;
	static int count = 0;
	bool first_col;

	// Allowed codepoints
	const int randmin = 33;
	const int randnum = 123 - randmin;
	// Chars change mid-scroll
	const bool changes = true;

	if(buf->height <= 3) {
		return;
	}

	count += 1;
	if(count > frame_delay) {
		frame += 1;
		if(frame > 4)
			frame = 1;
		count = 0;

		for(int j = 0; j < buf->width; j += 2) {
			int tail;
			if(frame > s->updates[j]) {
				if(s->grid[0][j].val == -1 && s->grid[1][j].val == ' ') {
					if(s->spaces[j] > 0) {
						s->spaces[j]--;
					} else {
						s->length[j] = (int)rand() % (buf->height - 3) + 3;
						s->grid[0][j].val = (int)rand() % randnum + randmin;
						s->spaces[j] = (int)rand() % buf->height + 1;
					}
				}

				int i = 0, seg_len = 0;
				first_col = 1;
				while(i <= buf->height) {
					// Skip over spaces
					while(i <= buf->height && (s->grid[i][j].val == ' ' ||
					                           s->grid[i][j].val == -1)) {
						i++;
					}

					if(i > buf->height)
						break;

					// Find the head of this col
					tail = i;
					seg_len = 0;
					while(i <= buf->height && (s->grid[i][j].val != ' ' &&
					                           s->grid[i][j].val != -1)) {
						s->grid[i][j].is_head = false;
						if(changes) {
							if(rand() % 8 == 0)
								s->grid[i][j].val =
									(int)rand() % randnum + randmin;
						}
						i++;
						seg_len++;
					}

					// Head's down offscreen
					if(i > buf->height) {
						s->grid[tail][j].val = ' ';
						continue;
					}

					s->grid[i][j].val = (int)rand() % randnum + randmin;
					s->grid[i][j].is_head = true;

					if(seg_len > s->length[j] || !first_col) {
						s->grid[tail][j].val = ' ';
						s->grid[0][j].val = -1;
					}
					first_col = 0;
					i++;
				}
			}
		}
	}

	uint32_t blank;
	utf8_char_to_unicode(&blank, " ");

	for(int j = 0; j < buf->width; j += 2) {
		for(int i = 1; i <= buf->height; ++i) {
			uint32_t c;
			int fg = TB_GREEN;
			int bg = TB_DEFAULT;

			if(s->grid[i][j].val == -1 || s->grid[i][j].val == ' ') {
				tb_change_cell(j, i - 1, blank, fg, bg);
				continue;
			}

			char tmp[2];
			tmp[0] = s->grid[i][j].val;
			tmp[1] = '\0';
			if(utf8_char_to_unicode(&c, tmp)) {
				if(s->grid[i][j].is_head) {
					fg = TB_WHITE | TB_BOLD;
				}
				tb_change_cell(j, i - 1, c, fg, bg);
			}
		}
	}
}

static void matrix_free(struct matrix_state *state) {
	free(state->grid[0]);
	free(state->grid);
	free(state->length);
	free(state->spaces);
	free(state->updates);
	free(state);
}
