#include "ModulesSettings.h"

ModulesSettings::ModulesSettings()
	: display(true),
	  dfs(true),
	  rdp(true),
	  timer(true),
	  controller(true),
	  console(false),
	  audio(true),
	  audio_mixer(true),
	  scene_manager(true),
	  memory_pool(true),
	  debug_is_viewer(true),
	  debug_usb(false) {
}
