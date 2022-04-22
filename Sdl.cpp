#include "Sdl.h"

#include <cstdio>
#include <string>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"

#include "App.h"

void Sdl::Init(App *app) {
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
#ifndef DEBUG
	rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
#endif
	windowFlags = SDL_WINDOW_RESIZABLE;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF);
	TTF_Init();

	app->window = SDL_CreateWindow(app->default_title, SDL_WINDOWPOS_CENTERED,
								   SDL_WINDOWPOS_CENTERED, 1200, 768, windowFlags);

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

	if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512) < 0) {
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		exit(-1);
	}

	if (Mix_AllocateChannels(4) < 0) {
		fprintf(stderr, "Unable to allocate mixing channels: %s\n", SDL_GetError());
		exit(-1);
	}
}

void Sdl::Quit(App *app, SDL_Window *window, SDL_Renderer *renderer) {
	if (app->audio_sample) {
		Mix_FreeChunk(app->audio_sample);
	}
	Mix_CloseAudio();

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
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
