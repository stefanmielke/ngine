#pragma once

#include <SDL2/SDL.h>
#include "ProjectState.h"
#include "settings/EngineSettings.h"
#include "settings/Project.h"

class App {
   public:
	SDL_Renderer *renderer;
	SDL_Window *window;
	ProjectState state;
	Project project;
	EngineSettings engine_settings;
};
