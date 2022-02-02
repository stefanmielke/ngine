#include "generated.h"

#include <sstream>

const char *change_scene_gen_c = R"(#include "change_scene.s.h"

#include "../game.s.h"
%s

void change_scene(short curr_scene, short next_scene) {
	switch (next_scene) {
%s
		default:
			abort();
	}
})";

void generate_change_scene_gen_c(std::string &filepath, Project &project) {
	std::stringstream header_content;
	std::stringstream content;
	for (auto &scene : project.scenes) {
		header_content << "#include \"scene_" << scene.id << ".gen.h\"" << std::endl;

		content << "\t\tcase " << scene.id << ": {" << std::endl;
		content << "\t\t\tscene_manager_set_callbacks(scene_manager, &scene_" << scene.id
				<< "_create, &scene_" << scene.id << "_tick, &scene_" << scene.id
				<< "_display, &scene_" << scene.id << "_destroy);" << std::endl;
		content << "\t\t} break;" << std::endl;
	}

	FILE *filestream = fopen(filepath.c_str(), "w");
	fprintf(filestream, change_scene_gen_c, header_content.str().c_str(), content.str().c_str());
	fclose(filestream);
}