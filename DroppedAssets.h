#pragma once

#include <string>
#include <SDL2/SDL.h>

#include "LibdragonSound.h"

struct DroppedImage {
	std::string image_path;
	SDL_Texture *image_data;
	int w, h;
	float width_mult, height_mult;

	char name[50] = "\0";
	char dfs_folder[100] = "/\0";
	int h_slices, v_slices;

	explicit DroppedImage(const char *image_path)
		: image_path(image_path),
		  image_data(nullptr),
		  w(0),
		  h(0),
		  width_mult(1),
		  height_mult(1),
		  h_slices(1),
		  v_slices(1) {
	}
};

struct DroppedSound {
	std::string sound_path;
	LibdragonSoundType type;

	char name[50] = "\0";
	char dfs_folder[100] = "/\0";

	bool compress;
	bool loop;
	int loop_offset;

	DroppedSound(const char *sound_path, LibdragonSoundType type)
		: sound_path(sound_path), type(type), compress(false), loop(false), loop_offset(0) {
	}
};

struct DroppedGeneralFile {
	std::string file_path;

	char name[50] = "\0";
	char extension[10] = "\0";
	char dfs_folder[100] = "/\0";

	explicit DroppedGeneralFile(const char *file_path) : file_path(file_path) {
	}
};
