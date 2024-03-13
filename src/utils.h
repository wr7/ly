#ifndef H_LY_UTILS
#define H_LY_UTILS

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

#include "draw.h"
#include "inputs.h"
#include "config.h"

void desktop_load(struct desktop* target);
void hostname(char** out);
void free_hostname();
void switch_tty(struct term_buf* buf);
void save(struct desktop* desktop, struct text* login);
void load(struct desktop* desktop, struct text* login);

#endif
