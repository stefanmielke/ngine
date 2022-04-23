#include "LibdragonFont.h"

#include <fstream>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_custom.h"

LibdragonFont::LibdragonFont()
	: dfs_folder("/"),
	  font_size(16),
	  width(0),
	  height(0),
	  display_width(0),
	  display_height(0),
	  loaded_image(nullptr) {
}

LibdragonFont::~LibdragonFont() {
	if (loaded_image) {
		SDL_DestroyTexture(loaded_image);
		loaded_image = nullptr;
	}
}

void LibdragonFont::LoadImage(const std::string &project_directory, SDL_Renderer *renderer) {
	std::string path(project_directory + "/" + font_path);

	if (loaded_image) {
		SDL_DestroyTexture(loaded_image);
		loaded_image = nullptr;
	}

	loaded_image = LoadTextureFromFont(path.c_str(), font_size, renderer);

	int w, h;
	SDL_QueryTexture(loaded_image, nullptr, nullptr, &w, &h);

	width = w;
	height = h;

	const float max_size = 100.f;
	if (w > h) {
		h = (int)(((float)h / (float)w) * max_size);
		w = (int)max_size;
	} else {
		w = (int)(((float)w / (float)h) * max_size);
		h = (int)max_size;
	}

	display_width = w;
	display_height = h;
}

SDL_Surface *LibdragonFont::LoadSurfaceFromFont(const char *font_path, int font_size,
												SDL_Renderer *renderer) {
	const SDL_Color fg = {255, 255, 255, 255};

	auto *font = TTF_OpenFont(font_path, font_size);
	std::string font_text;

	int largest_width = 0;
	int largest_height = 0;
	for (int i = 0; i < 128; ++i) {
		int c = i;
		if (c == '\0')
			c = ' ';
		font_text.push_back(c);

		int w, h;
		TTF_SizeText(font, std::string(1, (char)c).c_str(), &w, &h);

		if (w > largest_width)
			largest_width = w;
		if (h > largest_height)
			largest_height = h;
	}
	SDL_Surface *target_surface = SDL_CreateRGBSurface(0, largest_width * 16, largest_height * 8,
													   32, 0x000000FF, 0x0000FF00, 0x00FF0000,
													   0xFF000000);

	SDL_Rect target_rect = {0, 0, 0, 0};
	for (auto font_c : font_text) {
		target_rect.x = (font_c % 16) * largest_width;
		target_rect.y = (font_c / 16) * largest_height;

		SDL_Surface *surface = TTF_RenderGlyph32_Blended(font, font_c, fg);
		SDL_BlitSurface(surface, nullptr, target_surface, &target_rect);
		SDL_FreeSurface(surface);
	}
	TTF_CloseFont(font);

	return target_surface;
}

SDL_Texture *LibdragonFont::LoadTextureFromFont(const char *font_path, int font_size,
												SDL_Renderer *renderer) {
	SDL_Surface *surface = LoadSurfaceFromFont(font_path, font_size, renderer);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	return texture;
}

void LibdragonFont::SaveToDisk(const std::string &project_directory) {
	nlohmann::json json = {
		{"name", name},
		{"font_path", font_path},
		{"dfs_folder", dfs_folder},
		{"font_size", font_size},
	};

	std::string directory = project_directory + "/.ngine/fonts/";
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directories(directory);

	std::string filepath = directory + name + ".font.json";

	std::ofstream filestream(filepath);
	filestream << json.dump(4) << std::endl;
	filestream.close();
}

void LibdragonFont::LoadFromDisk(const std::string &filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
	font_path = json["font_path"];
	dfs_folder = json["dfs_folder"];
	font_size = json["font_size"];

	std::replace(dfs_folder.begin(), dfs_folder.end(), '\\', '/');
}

void LibdragonFont::DeleteFromDisk(const std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/fonts/" + name + ".font.json";
	std::filesystem::remove(json_filepath);

	std::string image_filepath = project_directory + "/" + font_path;
	std::filesystem::remove(image_filepath);
}

void LibdragonFont::DrawTooltip() const {
	std::stringstream tooltip;
	tooltip << "Path: " << font_path << "\nDFS_Path: " << dfs_folder << name
			<< ".font\nSize: " << font_size;

	ImGui::BeginTooltip();
	render_badge("font", ImVec4(.8f, .8f, .4f, 0.7f));
	ImGui::SameLine();
	ImGui::Text("%s", name.c_str());
	ImGui::Separator();

	ImGui::Text("%s", tooltip.str().c_str());
	ImGui::Separator();

	ImGui::Image((ImTextureID)(intptr_t)loaded_image, ImVec2((float)width, (float)height));
	ImGui::EndTooltip();
}
