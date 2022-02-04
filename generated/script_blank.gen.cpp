const char *script_blank_gen_h = R"(#pragma once

#include <libdragon.h>

void script_%s_create();
short script_%s_tick();
void script_%s_display(display_context_t disp);
void script_%s_destroy();
)";

const char *script_blank_gen_c = R"(#include "%s.script.h"

void script_%s_create() {
	// you can use 'scene_memory_pool' to allocate this instead of 'malloc'
}

short script_%s_tick() {
	// return >0 to change to that screen.
	return -1;
}

void script_%s_display(display_context_t disp) {
	// runs after screen blank and before 'display_show'
}

void script_%s_destroy() {
	// remember to destroy anything that uses malloc!
})";