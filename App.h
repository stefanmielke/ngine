#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "ProjectState.h"
#include "settings/EngineSettings.h"
#include "settings/Project.h"
#include "GUIImage.h"

enum SoundState { SS_STOPPED, SS_PLAYING, SS_PAUSED };

struct EngineVersion {
	int major;
	int minor;
	bool is_pre_release;

	std::string version_string;

	EngineVersion() : major(1), minor(3), is_pre_release(false), version_string("Version ") {
		version_string += std::to_string(major) + "." + std::to_string(minor) + ".0";
		if (is_pre_release)
			version_string.append("-pre");
	};
};

class App {
   public:
	EngineVersion engine_version;
	SDL_Renderer *renderer;
	SDL_Window *window;
	EngineSettings engine_settings;
	Project project;
	ProjectState state;
	SDL_Texture *app_texture;

	// audio
	Mix_Chunk *audio_sample;
	SoundState audio_state;

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
