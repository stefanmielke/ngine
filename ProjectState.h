#pragma once

#include <memory>
#include <vector>

#include "DroppedAssets.h"
#include "LibdragonImage.h"
#include "LibdragonFile.h"
#include "LibdragonScript.h"
#include "LibdragonSound.h"
#include "LibdragonTiledMap.h"
#include "content/AssetReference.h"
#include "settings/EngineSettings.h"
#include "settings/ProjectSettingsScreen.h"
#include "settings/Scene.h"

class ProjectState {
   public:
	AssetReference asset_selected;
	AssetReference asset_editing;
	bool reload_asset_edit;

	std::unique_ptr<LibdragonScript> *selected_script;
	std::unique_ptr<LibdragonScript> *script_editing;
	bool reload_script_edit;

	std::vector<DroppedImage> dropped_image_files;
	std::vector<DroppedSound> dropped_sound_files;
	std::vector<DroppedGeneralFile> dropped_general_files;
	std::vector<DroppedFont> dropped_font_files;
	std::vector<DroppedTiledMap> dropped_tiled_files;
	std::vector<DroppedLDtkMap> dropped_ldtk_files;

	char emulator_path[255];
	char editor_path[255];
	char libdragon_exe_path[255];
	bool libdragon_use_bundled;
	ProjectSettingsScreen project_settings_screen;

	Scene *current_scene;
	char scene_name[100];

	void LoadEngineSetings(const EngineSettings &engine_settings) {
		strcpy(emulator_path, engine_settings.GetEmulatorPath().c_str());
		strcpy(editor_path, engine_settings.GetEditorLocation().c_str());
		strcpy(libdragon_exe_path, engine_settings.GetLibdragonExeLocation().c_str());

		libdragon_use_bundled = engine_settings.GetLibdragonUseBundled();
	}

	explicit ProjectState(const EngineSettings &engine_settings)
		: reload_asset_edit(false),
		  selected_script(nullptr),
		  script_editing(nullptr),
		  reload_script_edit(false),
		  emulator_path(),
		  editor_path(),
		  libdragon_exe_path(),
		  libdragon_use_bundled(true),
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
			if (image.image_data_overlay)
				SDL_DestroyTexture(image.image_data_overlay);
		}
		for (auto &image : dropped_font_files) {
			if (image.font_data)
				SDL_DestroyTexture(image.font_data);
		}

		dropped_general_files.clear();
		dropped_sound_files.clear();
		dropped_image_files.clear();
		dropped_font_files.clear();
		dropped_tiled_files.clear();
		dropped_ldtk_files.clear();
	}
};
