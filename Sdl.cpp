#include "Sdl.h"

#include <cstdio>
#include <string>
#include <SDL2/SDL_image.h>

#include "imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"

#include "App.h"

void Sdl::Init(App *app) {
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;
	windowFlags = SDL_WINDOW_RESIZABLE;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	IMG_Init(IMG_INIT_PNG);

	app->window = SDL_CreateWindow(app->default_title, SDL_WINDOWPOS_CENTERED,
								   SDL_WINDOWPOS_CENTERED, 1024, 768, windowFlags);

	if (!app->window) {
		printf("Failed to open window: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	app->renderer = SDL_CreateRenderer(app->window, -1, rendererFlags);

	if (!app->renderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForSDLRenderer(app->window);
	ImGui_ImplSDLRenderer_Init(app->renderer);

	std::string font_path(app->GetEngineDirectory());
	font_path.append("/montserrat.ttf");

	ImGuiIO &io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.Fonts->AddFontFromFileTTF(font_path.c_str(), 14.0f);
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
