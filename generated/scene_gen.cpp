#include "generated.h"

const char *scene_gen_h = R"(#pragma once

#include <libdragon.h>

void scene_%d_create();
short scene_%d_tick();
void scene_%d_display(display_context_t disp);
void scene_%d_destroy();
)";

const char *scene_gen_c = R"(#include "scene_%d.gen.h"

%s

void scene_%d_create() {
	%s
}

short scene_%d_tick() {
	%s

	return %d;
}

void scene_%d_display(display_context_t disp) {
	graphics_fill_screen(disp, %u);
	graphics_set_color(0xfff, 0);

	%s
}

void scene_%d_destroy() {
	%s
})";

void generate_scene_gen_files(const Project &project) {
	if (!project.project_settings.modules.scene_manager)
		return;

	for (auto &scene : project.scenes) {
		std::string header_name = project.project_settings.project_directory + "/src/scenes/scene_" +
								  std::to_string(scene.id) + ".gen.h";
		FILE *filestream = fopen(header_name.c_str(), "w");
		fprintf(filestream, scene_gen_h, scene.id, scene.id, scene.id, scene.id);
		fclose(filestream);

		std::string c_name = project.project_settings.project_directory + "/src/scenes/scene_" +
							 std::to_string(scene.id) + ".gen.c";

		std::string includes;
		std::string create_method_impl;
		std::string tick_method_impl;
		std::string display_method_impl;
		std::string destroy_method_impl;

		if (!scene.script_name.empty()) {
			includes = "#include \"../scripts/" + scene.script_name + ".script.h\"";
			create_method_impl = "script_" + scene.script_name + "_create();";
			tick_method_impl = "short result = script_" + scene.script_name +
							   "_tick();\n\tif (result >= 0) return result;";
			display_method_impl = "script_" + scene.script_name + "_display(disp);";
			destroy_method_impl = "script_" + scene.script_name + "_destroy();";
		}

		unsigned int fill_color;
		if (project.project_settings.display.bit_depth == DEPTH_16_BPP) {
			auto sr = (unsigned char)(255 * scene.fill_color[0]);
			auto sg = (unsigned char)(255 * scene.fill_color[1]);
			auto sb = (unsigned char)(255 * scene.fill_color[2]);

			int r = sr >> 3;
			int g = sg >> 3;
			int b = sb >> 3;
			int a = 255;

			fill_color = ((r & 0x1F) << 11) | ((g & 0x1F) << 6) | ((b & 0x1F) << 1) | (a >> 7);
			fill_color = fill_color | (fill_color << 16);
		} else {
			auto sr = (unsigned char)(255 * scene.fill_color[0]);
			auto sg = (unsigned char)(255 * scene.fill_color[1]);
			auto sb = (unsigned char)(255 * scene.fill_color[2]);
			fill_color = (sr << 24) | (sg << 16) | (sb << 8) | ((unsigned char)255);
		}

		filestream = fopen(c_name.c_str(), "w");
		fprintf(filestream, scene_gen_c, scene.id, includes.c_str(), scene.id,
				create_method_impl.c_str(), scene.id, tick_method_impl.c_str(), scene.id, scene.id,
				fill_color, display_method_impl.c_str(), scene.id, destroy_method_impl.c_str());
		fclose(filestream);
	}
}