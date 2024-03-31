#pragma once

#include "draw.h"

struct doom_state *doom_init(struct term_buf *buf);
void doom_free(struct doom_state *state);
void doom(struct doom_state *state, struct term_buf *term_buf);
