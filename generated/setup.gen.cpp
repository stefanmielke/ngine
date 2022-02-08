#include "generated.h"

#include <sstream>

const char *setup_gen_c = R"(#include <libdragon.h>

#include "game.gen.h"
#include "scenes/change_scene.s.h"
%s
%s
void setup() {
%s
%s}

void tick() {
%s
%s}

void display() {
%s})";

void generate_setup_gen_c(std::string &setup_path, ProjectSettings &settings) {
	std::stringstream setup_body;
	std::stringstream setup_end_body;
	std::stringstream tick_body;
	std::stringstream tick_end_body;
	std::stringstream display_body;
	std::stringstream includes;
	std::stringstream variables;

	if (settings.modules.display) {
		setup_body << "\tdisplay_init(" << settings.display.GetResolution() << ", "
				   << settings.display.GetBitDepth() << ", " << settings.display.buffers << ", "
				   << settings.display.GetGamma() << ", " << settings.display.GetAntialias() << ");"
				   << std::endl;

		display_body << "\tstatic display_context_t disp = 0;" << std::endl
					 << "\twhile (!(disp = display_lock()));" << std::endl;
	}
	if (settings.modules.memory_pool) {
		variables << "MemZone global_memory_pool;" << std::endl
				  << "MemZone scene_memory_pool;" << std::endl;

		setup_end_body << "\tmem_zone_init(&global_memory_pool, "
					   << settings.global_mem_alloc_size * 1024 << ");" << std::endl
					   << "\tmem_zone_init(&scene_memory_pool, "
					   << settings.scene_mem_alloc_size * 1024 << ");" << std::endl;
	}
	if (settings.modules.scene_manager) {
		variables << "SceneManager *scene_manager;" << std::endl;

		tick_end_body << "\tscene_manager_tick(scene_manager);" << std::endl;

		if (settings.modules.display) {
			display_body << "\tscene_manager_display(scene_manager, disp);" << std::endl;
		}

		if (settings.modules.memory_pool) {
			setup_end_body << "\tscene_manager = scene_manager_init(&global_memory_pool, "
							  "&scene_memory_pool, change_scene);"
						   << std::endl
						   << "\tscene_manager_change_scene(scene_manager, "
						   << settings.initial_screen_id << ");" << std::endl;
		} else {
			setup_end_body << "\tscene_manager = scene_manager_init(NULL, NULL, change_scene);"
						   << std::endl
						   << "\tscene_manager_change_scene(scene_manager, 0);" << std::endl;
		}
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
	if (settings.modules.audio) {
		setup_body << "\taudio_init(" << settings.audio.frequency << ", " << settings.audio.buffers
				   << ");" << std::endl;
	}
	if (settings.modules.audio_mixer) {
		setup_body << "\tmixer_init(" << settings.audio_mixer.channels << ");" << std::endl;

		tick_end_body << "\tif (audio_can_write()) {" << std::endl
					  << "\t\tshort *buf = audio_write_begin();" << std::endl
					  << "\t\tmixer_poll(buf, audio_get_buffer_length());" << std::endl
					  << "\t\taudio_write_end();" << std::endl
					  << "\t}" << std::endl;
	}
	if (settings.modules.debug_is_viewer) {
		setup_body << "\tdebug_init_isviewer();" << std::endl;
	}
	if (settings.modules.debug_usb) {
		setup_body << "\tdebug_init_usblog();" << std::endl
				   << "\tconsole_set_debug(true);" << std::endl;
	}

	if (!settings.global_script_name.empty()) {
		setup_end_body << "\tscript_" << settings.global_script_name << "_create();" << std::endl;

		tick_body << "\tscript_" << settings.global_script_name << "_tick();" << std::endl;

		if (settings.modules.display) {
			display_body << "\tscript_" << settings.global_script_name << "_display(disp);"
						 << std::endl;
		}

		includes << "#include \"scripts/" << settings.global_script_name << ".script.h\""
				 << std::endl;
	}

	if (settings.modules.display) {
		display_body << "\tdisplay_show(disp);" << std::endl;
	}

	FILE *filestream = fopen(setup_path.c_str(), "w");
	fprintf(filestream, setup_gen_c, includes.str().c_str(), variables.str().c_str(),
			setup_body.str().c_str(), setup_end_body.str().c_str(), tick_body.str().c_str(),
			tick_end_body.str().c_str(), display_body.str().c_str());
	fclose(filestream);
}