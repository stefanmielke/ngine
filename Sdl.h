#pragma once

#include <SDL2/SDL.h>

class Sdl {
   public:
	static void Init(SDL_Window *window, SDL_Renderer *renderer, const char *window_title);
	static void Quit(SDL_Window *window, SDL_Renderer *renderer);
};
