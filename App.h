#pragma once

#include <SDL2/SDL.h>
#include "ProjectState.h"
#include "settings/EngineSettings.h"
#include "settings/Project.h"

class App {
   public:
	SDL_Renderer *renderer;
	SDL_Window *window;
	EngineSettings engine_settings;
	Project project;
	ProjectState state;

	bool is_running;

	App();
};
