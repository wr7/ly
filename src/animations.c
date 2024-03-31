#include "animations.h"

#include "config.h"
#include "dragonfail.h"
#include "dragonfail_error.h"
#include "draw.h"
#include "stdio.h"
#include "termbox.h"
#include "utils.h"

#include "animations/doom.h"
#include "animations/matrix.h"

#include <stdlib.h>
#include <string.h>

struct animation {
	void *(*const init)(struct term_buf *buf);
	void (*const free)(void *state);
	void (*const draw)(void *state, struct term_buf *buf);
};

static struct random_state *random_init(struct term_buf *buf);
static void random_free(struct random_state *state);
static void random_draw(struct random_state *state, struct term_buf *term_buf);

static const struct animation ANIMATIONS[] = {
	{
		// Cast `random_state *` to `void *`
		.init = (void *(*)(struct term_buf *buf))random_init,
		.free = (void (*)(void *state))random_free,
		.draw = (void (*)(void *state, struct term_buf *buf))random_draw,
	},
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

// Random animation //

struct random_state {
	const struct animation *animation;
	void *animation_state;
};

static struct random_state *random_init(struct term_buf *buf) {
	const size_t animation_idx =
		((size_t)rand()) % (ARRAY_LENGTH(ANIMATIONS) - 1) + 1;

	struct random_state *state = malloc_or_throw(sizeof(*state));

	state->animation = &ANIMATIONS[animation_idx];
	state->animation_state = state->animation->init(buf);

	return state;
}

static void random_free(struct random_state *state) {
	state->animation->free(state->animation_state);
	free(state);
}

static void random_draw(struct random_state *state, struct term_buf *term_buf) {
	state->animation->draw(state->animation_state, term_buf);
}
