#pragma once

#include <string>

#include "AudioMixerSettings.h"
#include "AudioSettings.h"
#include "DisplaySettings.h"
#include "MenuSettings.h"
#include "ModulesSettings.h"

class App;

class ProjectSettings {
   private:
	bool is_open;

   public:
	int next_scene_id;
	std::string project_name;
	std::string rom_name;
	std::string project_directory;
	int save_type;
	bool region_free;

	int initial_scene_id;
	std::string global_script_name;

	int global_mem_alloc_size;
	int scene_mem_alloc_size;

	AudioSettings audio;
	AudioMixerSettings audio_mixer;
	DisplaySettings display;
	ModulesSettings modules;
	MenuSettings menu;

	std::string libdragon_branch;

	int ngine_version_major;
	int ngine_version_minor;
	bool ngine_version_is_pre_release;

	explicit ProjectSettings(App *app);

	bool LoadFromFile(App *app, const std::string &folder);
	void SaveToDisk();

	[[nodiscard]] bool IsOpen() const {
		return is_open;
	};
};
