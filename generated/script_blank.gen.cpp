const char *script_blank_gen_h = R"(#pragma once

#include <libdragon.h>

void script_%s_create();
void script_%s_tick();
void script_%s_display(display_context_t disp);
void script_%s_destroy();
)";

const char *script_blank_gen_c = R"(#include "%s.script.h"

void script_%s_create() {
}

void script_%s_tick() {
}

void script_%s_display(display_context_t disp) {
}

void script_%s_destroy() {
})";