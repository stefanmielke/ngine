#pragma once

#include <string>
#include <SDL2/SDL.h>

#include "LibdragonImage.h"
#include "LibdragonSound.h"
#include "LibdragonTiledMap.h"

struct DroppedImage {
	std::string image_path;
	LibdragonImageType type;

	SDL_Texture *image_data;
	SDL_Texture *image_data_overlay;
	int w, h;
	float width_mult, height_mult;

	char name[50] = "\0";
	char dfs_folder[100] = "/\0";
	int h_slices, v_slices;

	DroppedImage(const char *image_path, LibdragonImageType type)
		: image_path(image_path),
		  type(type),
		  image_data(nullptr),
		  image_data_overlay(nullptr),
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
	bool copy_to_filesystem;

	explicit DroppedGeneralFile(const char *file_path)
		: file_path(file_path), copy_to_filesystem(true) {
	}
};

struct DroppedTiledMap {
	std::string file_path;

	char name[50] = "\0";
	char dfs_folder[100] = "/\0";

	std::vector<LibdragonMapLayer> layers;

	explicit DroppedTiledMap(const char *file_path) : file_path(file_path) {
	}
};

struct DroppedLDtkMap {
	std::string file_path;

	char name[50] = "\0";
	char dfs_folder[100] = "/\0";

	std::vector<LibdragonMapLayer> layers;

	explicit DroppedLDtkMap(const char *file_path) : file_path(file_path) {
	}
};

struct DroppedFont {
	std::string font_path;

	SDL_Texture *font_data;
	int w, h;
	float width_mult, height_mult;

	char name[50] = "\0";
	char dfs_folder[100] = "/\0";
	int font_size;

	explicit DroppedFont(const char *font_path)
		: font_path(font_path),
		  font_data(nullptr),
		  w(0),
		  h(0),
		  width_mult(1),
		  height_mult(1),
		  font_size(16) {
	}
};
