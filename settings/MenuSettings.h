#pragma once

static inline void reset_color(float color[4], float r, float g, float b, float a) {
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = a;
}
static inline void set_color2(float color[4], float selected_color[4]) {
	color[0] = selected_color[0];
	color[1] = selected_color[1];
	color[2] = selected_color[2];
	color[3] = selected_color[3];
}

class MenuSettings {
   public:
	float text_selected_color[4];
	float text_enabled_color[4];
	float text_disabled_color[4];
	float text_background_color[4];
	float text_out_of_bounds_color[4];
	float menu_background_color[4];

	MenuSettings() {
		reset_color(text_selected_color, 0, 0, 1, 1);
		reset_color(text_enabled_color, 1, 1, 1, 1);
		reset_color(text_disabled_color, .5f, .5f, 1, 1);
		reset_color(text_background_color, 0, 0, 0, 0);
		reset_color(text_out_of_bounds_color, 0, 0, 0, 1);
		reset_color(menu_background_color, 1, 1, 1, 1);
	}
};
