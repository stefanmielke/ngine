#include "generated.h"

#include <sstream>

const char *game_gen_h =
	R"(#include <libdragon.h>
%s
%s)";

void generate_game_gen_h(const Project &project) {
	std::stringstream includes;
	std::stringstream variables;

	if (project.project_settings.modules.memory_pool) {
		includes << "#include <mem_pool.h>" << std::endl;

		variables << "extern MemZone global_memory_pool;" << std::endl
				  << "extern MemZone scene_memory_pool;" << std::endl;
	}
	if (project.project_settings.modules.scene_manager) {
		includes << "#include <scene_manager.h>" << std::endl;

		variables << "extern SceneManager *scene_manager;" << std::endl;
	}
	if (project.project_settings.modules.controller) {
		variables << "extern struct controller_data keys_up;" << std::endl
				  << "extern struct controller_data keys_down;" << std::endl
				  << "extern struct controller_data keys_pressed;" << std::endl;
	}
	if (project.project_settings.modules.rtc) {
		variables << "extern bool rtc_initialized;" << std::endl;
	}

	FILE *filestream = fopen(
		(project.project_settings.project_directory + "/src/game.gen.h").c_str(), "w");
	fprintf(filestream, game_gen_h, includes.str().c_str(), variables.str().c_str());
	fclose(filestream);
}