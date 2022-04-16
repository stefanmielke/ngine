#pragma once

#include <string>
#include <SDL2/SDL.h>

class LibdragonFont {
   public:
	std::string name;
	std::string font_path;
	std::string dfs_folder;
	int font_size;

	int width;
	int height;
	int display_width;
	int display_height;

	SDL_Texture *loaded_image;

	LibdragonFont();
	~LibdragonFont();

	void LoadImage(const std::string &project_directory, SDL_Renderer *renderer);

	void SaveToDisk(const std::string &project_directory);
	void LoadFromDisk(const std::string &filepath);
	void DeleteFromDisk(const std::string &project_directory) const;

	void DrawTooltip() const;

	static SDL_Surface *LoadSurfaceFromFont(const char *font_path, int font_size,
											SDL_Renderer *renderer);
	static SDL_Texture *LoadTextureFromFont(const char *font_path, int font_size,
											SDL_Renderer *renderer);
};
