#pragma once

#include "draw.h"

void *blizzard_init(struct term_buf *buf);
void blizzard_free(void *state);
void blizzard_draw(void *state, struct term_buf *term_buf);
