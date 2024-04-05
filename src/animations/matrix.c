#include "animations/matrix.h"
#include "utils.h"
#include <stdlib.h>

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
struct matrix_state *matrix_init(struct term_buf *buf) {
	struct matrix_state *s = malloc_or_throw(sizeof(*s));

	uint16_t len = buf->height + 1;
	s->grid = malloc_or_throw(sizeof(*s->grid) * len); // NOLINT

	len = (buf->height + 1) * buf->width;
	(s->grid)[0] = malloc_or_throw(sizeof((s->grid)[0]) * len); // NOLINT

	for(int i = 1; i <= buf->height; ++i) {
		s->grid[i] = s->grid[i - 1] + buf->width;
	}

	s->length = malloc_or_throw(buf->width * sizeof(*s->length));

	s->spaces = malloc_or_throw(buf->width * sizeof(*s->spaces));

	s->updates = malloc_or_throw(buf->width * sizeof(*s->updates));

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
void matrix(struct matrix_state *s, struct term_buf *buf) {
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

void matrix_free(struct matrix_state *state) {
	free(state->grid[0]);
	free(state->grid);
	free(state->length);
	free(state->spaces);
	free(state->updates);
	free(state);
}
