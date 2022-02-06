#pragma once

#include <memory>
#include <vector>

#include "DroppedAssets.h"
#include "LibdragonImage.h"
#include "LibdragonSound.h"
#include "settings/ProjectSettingsScreen.h"
#include "settings/Scene.h"

struct ProjectState {
   public:
	std::unique_ptr<LibdragonImage> *selected_image;
	std::unique_ptr<LibdragonImage> *image_editing;
	bool reload_image_edit;

	std::unique_ptr<LibdragonSound> *selected_sound;
	std::unique_ptr<LibdragonSound> *sound_editing;
	bool reload_sound_edit;

	std::vector<DroppedImage> dropped_image_files;
	std::vector<DroppedSound> dropped_sound_files;

	char input_new_project[255];
	char input_open_project[255];
	char emulator_path[255];
	ProjectSettingsScreen project_settings_screen;

	Scene *current_scene;
	char scene_name[100];

	ProjectState()
		: selected_image(nullptr),
		  image_editing(nullptr),
		  reload_image_edit(false),
		  selected_sound(nullptr),
		  sound_editing(nullptr),
		  reload_sound_edit(false),
		  input_new_project(),
		  input_open_project(),
		  emulator_path(),
		  project_settings_screen(),
		  current_scene(nullptr),
		  scene_name() {
		memset(input_new_project, 0, 255);
		memset(input_open_project, 0, 255);
		memset(emulator_path, 0, 255);
		memset(scene_name, 0, 100);
	}

	~ProjectState() {
		for (auto &image : dropped_image_files) {
			if (image.image_data)
				SDL_DestroyTexture(image.image_data);
		}

		dropped_sound_files.clear();
		dropped_image_files.clear();
	}
};
