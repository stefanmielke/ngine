#include "generated.h"

#include <sstream>

const char *setup_gen_c = R"(#include <libdragon.h>

#include "game.s.h"
#include "scenes/change_scene.s.h"

void setup() {
%s
	mem_zone_init(&global_memory_pool, %d);
	mem_zone_init(&scene_memory_pool, %d);

	scene_manager_init(&global_memory_pool, &scene_memory_pool, change_scene);
})";

void generate_setup_gen_c(std::string &setup_path, ProjectSettings &settings) {
	std::stringstream function_body;

	if (settings.modules.display) {
		function_body << "\tdisplay_init(" << settings.display.GetResolution() << ", "
					  << settings.display.GetBitDepth() << ", " << settings.display.buffers << ", "
					  << settings.display.GetGamma() << ", " << settings.display.GetAntialias()
					  << ");" << std::endl;
	}
	if (settings.modules.dfs) {
		function_body << "\tdfs_init(DFS_DEFAULT_LOCATION);" << std::endl;
	}
	if (settings.modules.rdp) {
		function_body << "\trdp_init();" << std::endl;
	}
	if (settings.modules.timer) {
		function_body << "\ttimer_init();" << std::endl;
	}
	if (settings.modules.controller) {
		function_body << "\tcontroller_init();" << std::endl;
	}
	if (settings.modules.console) {
		function_body << "\tconsole_init();" << std::endl;
	}
	if (settings.modules.debug_is_viewer) {
		function_body << "\tdebug_init_isviewer();" << std::endl;
	}
	if (settings.modules.debug_usb) {
		function_body << "\tdebug_init_usblog();" << std::endl
					  << "\tconsole_set_debug(true);" << std::endl;
	}

	FILE *filestream = fopen(setup_path.c_str(), "w");
	fprintf(filestream, setup_gen_c, function_body.str().c_str(),
			settings.global_mem_alloc_size * 1024, settings.scene_mem_alloc_size * 1024);
	fclose(filestream);
}