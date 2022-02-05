#include "generated.h"

#include <sstream>

const char *setup_gen_c = R"(#include <libdragon.h>

#include "game.s.h"
#include "scenes/change_scene.s.h"

%s

void setup() {
%s
	mem_zone_init(&global_memory_pool, %d);
	mem_zone_init(&scene_memory_pool, %d);

	scene_manager = scene_manager_init(&global_memory_pool, &scene_memory_pool, change_scene);
	scene_manager_change_scene(scene_manager, %d);
}

void tick() {
%s
}
)";

void generate_setup_gen_c(std::string &setup_path, ProjectSettings &settings) {
	std::stringstream setup_body;
	std::stringstream tick_body;
	std::stringstream variables;

	if (settings.modules.display) {
		setup_body << "\tdisplay_init(" << settings.display.GetResolution() << ", "
				   << settings.display.GetBitDepth() << ", " << settings.display.buffers << ", "
				   << settings.display.GetGamma() << ", " << settings.display.GetAntialias() << ");"
				   << std::endl;
	}
	if (settings.modules.dfs) {
		setup_body << "\tdfs_init(DFS_DEFAULT_LOCATION);" << std::endl;
	}
	if (settings.modules.rdp) {
		setup_body << "\trdp_init();" << std::endl;
	}
	if (settings.modules.timer) {
		setup_body << "\ttimer_init();" << std::endl;
	}
	if (settings.modules.controller) {
		setup_body << "\tcontroller_init();" << std::endl;

		tick_body << "\tcontroller_scan();" << std::endl
				  << "\tkeys_pressed = get_keys_pressed();" << std::endl
				  << "\tkeys_down = get_keys_down();" << std::endl
				  << "\tkeys_up = get_keys_up();" << std::endl;

		variables << "struct controller_data keys_up;" << std::endl
				  << "struct controller_data keys_down;" << std::endl
				  << "struct controller_data keys_pressed;" << std::endl;
	}
	if (settings.modules.console) {
		setup_body << "\tconsole_init();" << std::endl;
	}
	if (settings.modules.debug_is_viewer) {
		setup_body << "\tdebug_init_isviewer();" << std::endl;
	}
	if (settings.modules.debug_usb) {
		setup_body << "\tdebug_init_usblog();" << std::endl
				   << "\tconsole_set_debug(true);" << std::endl;
	}

	FILE *filestream = fopen(setup_path.c_str(), "w");
	fprintf(filestream, setup_gen_c, variables.str().c_str(), setup_body.str().c_str(),
			settings.global_mem_alloc_size * 1024, settings.scene_mem_alloc_size * 1024,
			settings.initial_screen_id, tick_body.str().c_str());
	fclose(filestream);
}