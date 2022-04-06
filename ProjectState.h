#pragma once

#include <memory>
#include <vector>

#include "DroppedAssets.h"
#include "LibdragonImage.h"
#include "LibdragonFile.h"
#include "LibdragonScript.h"
#include "LibdragonSound.h"
#include "settings/EngineSettings.h"
#include "settings/ProjectSettingsScreen.h"
#include "settings/Scene.h"

class ProjectState {
   public:
	std::unique_ptr<LibdragonImage> *selected_image;
	std::unique_ptr<LibdragonImage> *image_editing;
	bool reload_image_edit;

	std::unique_ptr<LibdragonSound> *selected_sound;
	std::unique_ptr<LibdragonSound> *sound_editing;
	bool reload_sound_edit;

	std::unique_ptr<LibdragonFile> *selected_general_file;
	std::unique_ptr<LibdragonFile> *general_file_editing;
	bool reload_general_file_edit;

	std::unique_ptr<LibdragonScript> *selected_script;
	std::unique_ptr<LibdragonScript> *script_editing;
	bool reload_script_edit;

	std::vector<DroppedImage> dropped_image_files;
	std::vector<DroppedSound> dropped_sound_files;
	std::vector<DroppedGeneralFile> dropped_general_files;

	char emulator_path[255];
	char editor_path[255];
	char libdragon_exe_path[255];
	ProjectSettingsScreen project_settings_screen;

	Scene *current_scene;
	char scene_name[100];

	void LoadEngineSetings(const EngineSettings &engine_settings) {
		strcpy(emulator_path, engine_settings.GetEmulatorPath().c_str());
		strcpy(editor_path, engine_settings.GetEditorLocation().c_str());
		strcpy(libdragon_exe_path, engine_settings.GetLibdragonExeLocation().c_str());
	}

	explicit ProjectState(const EngineSettings &engine_settings)
		: selected_image(nullptr),
		  image_editing(nullptr),
		  reload_image_edit(false),
		  selected_sound(nullptr),
		  sound_editing(nullptr),
		  reload_sound_edit(false),
		  selected_general_file(nullptr),
		  general_file_editing(nullptr),
		  reload_general_file_edit(false),
		  selected_script(nullptr),
		  script_editing(nullptr),
		  reload_script_edit(false),
		  emulator_path(),
		  editor_path(),
		  libdragon_exe_path(),
		  project_settings_screen(),
		  current_scene(nullptr),
		  scene_name() {
		memset(scene_name, 0, 100);

		LoadEngineSetings(engine_settings);
	}

	~ProjectState() {
		for (auto &image : dropped_image_files) {
			if (image.image_data)
				SDL_DestroyTexture(image.image_data);
		}

		dropped_general_files.clear();
		dropped_sound_files.clear();
		dropped_image_files.clear();
	}
};
