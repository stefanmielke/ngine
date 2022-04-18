#pragma once

#include <memory>
#include <vector>

#include "LibdragonFile.h"
#include "LibdragonFont.h"
#include "LibdragonImage.h"
#include "LibdragonLDtkMap.h"
#include "LibdragonSound.h"
#include "LibdragonTiledMap.h"
#include "settings/EngineSettings.h"
#include "settings/ProjectSettings.h"

class Content {
   public:
	static void CreateSprites(const EngineSettings &engine_settings,
							  const ProjectSettings &project_settings,
							  const std::vector<std::unique_ptr<LibdragonImage>> &images);
	static void CreateSounds(const EngineSettings &engine_settings,
							 const ProjectSettings &project_settings,
							 const std::vector<std::unique_ptr<LibdragonSound>> &sounds);
	static void CreateGeneralFiles(const EngineSettings &engine_settings,
								   const ProjectSettings &project_settings,
								   const std::vector<std::unique_ptr<LibdragonFile>> &files);
	static void CreateFonts(const EngineSettings &engine_settings,
							const ProjectSettings &project_settings,
							const std::vector<std::unique_ptr<LibdragonFont>> &fonts);
	static void CreateTiledMaps(const EngineSettings &engine_settings,
								const ProjectSettings &project_settings,
								const std::vector<std::unique_ptr<LibdragonTiledMap>> &maps);
	static void CreateLDtkMaps(const EngineSettings &engine_settings,
								const ProjectSettings &project_settings,
								const std::vector<std::unique_ptr<LibdragonLDtkMap>> &maps);
};
