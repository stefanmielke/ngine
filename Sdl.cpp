#include "Sdl.h"

#include <SDL2/SDL_image.h>

#include "imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"

void Sdl::Init(SDL_Window *window, SDL_Renderer *renderer, const char *window_title) {
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	windowFlags = SDL_WINDOW_RESIZABLE;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	IMG_Init(IMG_INIT_PNG);

	window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024,
							  768, windowFlags);

	if (!window) {
		printf("Failed to open window: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	renderer = SDL_CreateRenderer(window, -1, rendererFlags);

	if (!renderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForSDLRenderer(window);
	ImGui_ImplSDLRenderer_Init(renderer);
}

void Sdl::Quit(SDL_Window *window, SDL_Renderer *renderer) {
	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
}

void Sdl::ProcessEvent(SDL_Event *event) {
	ImGui_ImplSDL2_ProcessEvent(event);
}

void Sdl::NewFrame() {
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void Sdl::RenderStart(SDL_Renderer *renderer) {
	ImGui::Render();

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

void Sdl::RenderEnd(SDL_Renderer *renderer) {
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(renderer);
}
