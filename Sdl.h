#pragma once

#include <SDL2/SDL.h>

class App;

class Sdl {
   public:
	static void Init(App *app);
	static void Quit(SDL_Window *window, SDL_Renderer *renderer);

	static void ProcessEvent(SDL_Event *event);

	static void NewFrame();
	static void RenderStart(SDL_Renderer *renderer);
	static void RenderEnd(SDL_Renderer *renderer);
};
