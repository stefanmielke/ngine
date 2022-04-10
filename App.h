#pragma once

#include <SDL2/SDL.h>
#include "ProjectState.h"
#include "settings/EngineSettings.h"
#include "settings/Project.h"
#include "GUIImage.h"

class App {
   public:
	SDL_Renderer *renderer;
	SDL_Window *window;
	EngineSettings engine_settings;
	Project project;
	ProjectState state;
	SDL_Texture *app_texture;

	const char *default_title = "NGine - N64 Engine Powered by Libdragon";

	bool is_running;

	explicit App(std::string engine_directory);

	bool LoadAssets();
	void GetImagePosition(std::string name, ImVec2 &uv0, ImVec2 &uv1);

	bool OpenProject(const std::string &path);
	void CloseProject();

	[[nodiscard]] std::string GetEngineDirectory() const {
		return engine_directory;
	}

   private:
	std::string engine_directory;
	std::vector<GUIImage> images;
	ImVec2 image_size;
};
