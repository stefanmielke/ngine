#pragma once

#include <string>
#include <SDL2/SDL.h>

struct DroppedImage {
	std::string image_path;
	SDL_Texture *image_data;
	int w, h;

	char name[50] = "\0";
	char dfs_folder[100] = "/\0";
	int h_slices, v_slices;

	explicit DroppedImage(const char *image_path)
		: image_path(image_path), image_data(nullptr), w(0), h(0), h_slices(1), v_slices(1) {
	}
};

struct DroppedSound {
	std::string sound_path;

	char name[50] = "\0";
	char dfs_folder[100] = "/\0";

	explicit DroppedSound(const char *sound_path) : sound_path(sound_path) {
	}
};
