#pragma once

#include "draw.h"
#include "stddef.h"

void animate(struct term_buf* buf);
void animation_init(struct term_buf *buf);
void animation_free(struct term_buf *buf);

extern const size_t NUM_ANIMATIONS;
