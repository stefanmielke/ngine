#pragma once

#include <string>

#include "AudioMixerSettings.h"
#include "AudioSettings.h"
#include "DisplaySettings.h"
#include "ModulesSettings.h"

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

	int initial_screen_id;
	std::string global_script_name;

	int global_mem_alloc_size;
	int scene_mem_alloc_size;

	AudioSettings audio;
	AudioMixerSettings audio_mixer;
	DisplaySettings display;
	ModulesSettings modules;

	std::string libdragon_branch;

	ProjectSettings();

	bool LoadFromFile(const std::string &folder);
	void SaveToDisk();

	[[nodiscard]] bool IsOpen() const {
		return is_open;
	};
};
