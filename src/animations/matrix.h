#pragma once

#include "draw.h"

struct matrix_state *matrix_init(struct term_buf *buf);
void matrix(struct matrix_state *s, struct term_buf *buf);
void matrix_free(struct matrix_state *state);
