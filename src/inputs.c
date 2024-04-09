#include "inputs.h"
#include "config.h"
#include "dragonfail.h"
#include "termbox2.h"
#include "utils.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

void handle_desktop(void *input_struct, struct tb_event *event) {
	struct desktop *target = (struct desktop *)input_struct;

	if((event != NULL) && (event->type == TB_EVENT_KEY)) {
		if(event->key == TB_KEY_ARROW_LEFT || (event->key == TB_KEY_CTRL_H)) {
			input_desktop_right(target);
		} else if(event->key == TB_KEY_ARROW_RIGHT ||
		          (event->key == TB_KEY_CTRL_L)) {
			input_desktop_left(target);
		}
	}

	tb_set_cursor(target->x + 2, target->y);
}

void handle_text(void *input_struct, struct tb_event *event) {
	struct text *target = (struct text *)input_struct;

	if((event != NULL) && (event->type == TB_EVENT_KEY)) {
		if(event->key == TB_KEY_ARROW_LEFT) {
			input_text_left(target);
		} else if(event->key == TB_KEY_ARROW_RIGHT) {
			input_text_right(target);
		} else if(event->key == TB_KEY_DELETE) {
			input_text_delete(target);
		} else if((event->key == TB_KEY_BACKSPACE) ||
		          (event->key == TB_KEY_BACKSPACE2)) {
			input_text_backspace(target);
		} else if(((event->ch > 31) && (event->ch < 127)) ||
		          (event->key == TB_KEY_SPACE)) {
			char buf[7] = {0};

			if(event->key == TB_KEY_SPACE) {
				buf[0] = ' ';
			} else {
				tb_utf8_unicode_to_char(buf, event->ch);
			}

			input_text_write(target, buf[0]);
		}
	}

	tb_set_cursor(target->x + (target->cur - target->visible_start), target->y);
}

void input_desktop(struct desktop *target) {
	target->list = NULL;
	target->list_simple = NULL;
	target->cmd = NULL;
	target->display_server = NULL;
	target->cur = 0;
	target->len = 0;

	input_desktop_add(target, strdup(lang.shell), strdup(""), DS_SHELL);
	input_desktop_add(target, strdup(lang.xinitrc), strdup(config.xinitrc),
	                  DS_XINITRC);
#if 0
	input_desktop_add(target, strdup(lang.wayland), strdup(""), DS_WAYLAND);
#endif
}

void input_text(struct text *target) {
	target->len = 32;
	target->text = malloc_or_throw(sizeof(char)*target->len);

	memset(target->text, 0, target->len);

	target->cur = target->text;
	target->end = target->text;
	target->visible_start = target->text;
	target->x = 0;
	target->y = 0;
}

void input_text_resize(struct text *target, int64_t new_size) {
	if(new_size <= target->len) {
		return;
	}

	target->len = new_size;

	const ssize_t end_offset = target->end - target->text;
	const ssize_t cur_offset = target->cur - target->text;
	const ssize_t vstart_offset = target->visible_start- target->text;

	target->text = realloc_or_throw(target->text, target->len);
	target->end = target->text + end_offset;
	target->cur = target->text + cur_offset;
	target->visible_start = target->text + vstart_offset;
}

void input_desktop_free(struct desktop *target) {
	if(target != NULL) {
		for(uint16_t i = 0; i < target->len; ++i) {
			if(target->list[i] != NULL) {
				free(target->list[i]);
			}

			if(target->cmd[i] != NULL) {
				free(target->cmd[i]);
			}
		}

		free(target->list);
		free(target->cmd);
		free(target->display_server);
	}
}

void input_text_free(struct text *target) {
	memset(target->text, 0, target->len);
	free(target->text);
}

void input_desktop_right(struct desktop *target) {
	++(target->cur);

	if(target->cur >= target->len) {
		target->cur = 0;
	}
}

void input_desktop_left(struct desktop *target) {
	--(target->cur);

	if(target->cur >= target->len) {
		target->cur = target->len - 1;
	}
}

void input_desktop_add(struct desktop *target, char *name, char *cmd,
                       enum display_server display_server) {
	++(target->len);
	target->list =
		realloc_or_throw(target->list, target->len * (sizeof(*target->list)));
	target->list_simple =
		realloc_or_throw(target->list_simple, target->len * (sizeof(*target->list_simple)));
	target->cmd = realloc_or_throw(target->cmd, target->len * (sizeof(*target->cmd)));
	target->display_server = realloc_or_throw(
		target->display_server, target->len * (sizeof(*target->display_server)));
	target->cur = target->len - 1;

	target->list[target->cur] = name;

	int name_len = strlen(name);
	char *name_simple = strdup(name);

	if(strstr(name_simple, " ") != NULL) {
		name_simple = strtok(name_simple, " ");
	}

	for(int i = 0; i < name_len; i++) {
		name_simple[i] = tolower(name_simple[i]);
	}

	target->list_simple[target->cur] = name_simple;
	target->cmd[target->cur] = cmd;
	target->display_server[target->cur] = display_server;
}

void input_text_right(struct text *target) {
	if(target->cur < target->end) {
		++(target->cur);

		if((target->cur - target->visible_start) > target->visible_len) {
			++(target->visible_start);
		}
	}
}

void input_text_left(struct text *target) {
	if(target->cur > target->text) {
		--(target->cur);

		if((target->cur - target->visible_start) < 0) {
			--(target->visible_start);
		}
	}
}

void input_text_write(struct text *target, char ascii) {
	if(ascii <= 0) {
		return; // unices do not support usernames and passwords other than
		        // ascii
	}

	if((target->end - target->text + 1) >= target->len) {
		input_text_resize(target, 2*target->len);
	}

	// moves the text to the right to add space for the new ascii char
	memcpy(target->cur + 1, target->cur, target->end - target->cur);
	++(target->end);
	// adds the new char and moves the cursor to the right
	*(target->cur) = ascii;
	input_text_right(target);
}

void input_text_delete(struct text *target) {
	if(target->cur < target->end) {
		// moves the text on the right to overwrite the currently pointed char
		memcpy(target->cur, target->cur + 1, target->end - target->cur + 1);
		--(target->end);
	}
}

void input_text_backspace(struct text *target) {
	if(target->cur > target->text) {
		input_text_left(target);
		input_text_delete(target);
	}
}

void input_text_clear(struct text *target) {
	memset(target->text, 0, target->len);
	target->cur = target->text;
	target->end = target->text;
	target->visible_start = target->text;
}
