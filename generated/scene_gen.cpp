#include "generated.h"

const char *scene_gen_h = R"(#pragma once

#include <libdragon.h>

void scene_%d_create();
short scene_%d_tick();
void scene_%d_display(display_context_t disp);
void scene_%d_destroy();
)";

const char *scene_gen_c = R"(#include "scene_%d.gen.h"

void scene_%d_create() {
}

short scene_%d_tick() {
	return %d;
}

void scene_%d_display(display_context_t disp) {
	graphics_fill_screen(disp, 0);
	graphics_set_color(0xfff, 0);
	graphics_draw_text(disp, 20, 20, "Scene %d");
}

void scene_%d_destroy() {
})";

void generate_scene_gen_files(std::string &project_folder, Project &project) {
	for (auto &scene : project.scenes) {
		std::string header_name = project_folder + "/src/scenes/scene_" + std::to_string(scene.id) +
								  ".gen.h";
		FILE *filestream = fopen(header_name.c_str(), "w");
		fprintf(filestream, scene_gen_h, scene.id, scene.id, scene.id, scene.id);
		fclose(filestream);

		std::string c_name = project_folder + "/src/scenes/scene_" + std::to_string(scene.id) +
							 ".gen.c";
		filestream = fopen(c_name.c_str(), "w");
		fprintf(filestream, scene_gen_c, scene.id, scene.id, scene.id, scene.id, scene.id, scene.id,
				scene.id);
		fclose(filestream);
	}
}