#include "blizzard.h"

#include "bits/time.h"
#include "draw.h"
#include "stdlib.h"
#include "termbox.h"
#include "utils.h"
#include <stdint.h>
#include <time.h>

#include "utils/mtwister.h"

void *blizzard_init(struct term_buf *buf) {
	UNUSED(buf);
	return NULL;
}

void blizzard_free(void *state) { UNUSED(state); }

void blizzard_draw(void *state, struct term_buf *term_buf) {
	struct tb_cell *buf = tb_cell_buffer();

	const struct tb_cell snow_cells[] = {
		{
			.ch = '#',
			.fg = 8,
			.bg = 0,
		},
		{
			.ch = '#',
			.fg = 8,
			.bg = 0,
		},
		{
			.ch = '+',
			.fg = 8,
			.bg = 0,
		},
		{
			.ch = '*',
			.fg = 7,
			.bg = 0,
		},
	};

	const struct tb_cell empty_cell = {
		.ch = ' ',
		.fg = 9,
		.bg = 0,
	};

	const clock_t time = clock();
	const uint64_t vertical_ticks = time / (CLOCKS_PER_SEC / 400);

	for(uint64_t y = 0; y < term_buf->height; y++) {
		const uint64_t seed = (vertical_ticks + term_buf->height) - y;
		MTRand rng = seedRand(seed);

		const uint64_t horizontal_offset = y;
		for(uint64_t i = 0; i < horizontal_offset; i++) {
			genRandLong(&rng);
		}

		for(uint64_t x = 0; x < term_buf->width; x++) {
			size_t snow_num =
				genRandLong(&rng) % (25 * ARRAY_LENGTH(snow_cells));

			if(snow_num < ARRAY_LENGTH(snow_cells)) {
				buf[y * term_buf->width + x] = snow_cells[snow_num];
			} else {
				buf[y * term_buf->width + x] = empty_cell;
			}
		}
	}

	UNUSED(state);
}
