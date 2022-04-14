#include "Project.h"

#include <filesystem>
#include <fstream>

#include "../App.h"
#include "../ConsoleApp.h"
#include "../json.hpp"

Project::Project(App *app) : project_settings(app) {
}

void Project::SaveToDisk(const std::string &project_directory) {
	std::filesystem::path scenes_folder(project_directory + "/.ngine/scenes");
	if (!std::filesystem::exists(scenes_folder)) {
		std::filesystem::create_directories(scenes_folder);
	}

	for (auto &scene : scenes) {
		nlohmann::json json;
		json["id"] = scene.id;
		json["name"] = scene.name;
		json["fill_color"] = scene.fill_color;
		json["script_name"] = scene.script_name;

		std::ofstream file(project_directory + "/.ngine/scenes/" + std::to_string(scene.id) +
						   ".scene.json");
		file << json.dump(4) << std::endl;
		file.close();
	}
}

void Project::LoadFromDisk(const std::string &project_directory) {
	std::filesystem::path scenes_folder(project_directory + "/.ngine/scenes");
	if (!std::filesystem::exists(scenes_folder)) {
		scenes.clear();
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iter(scenes_folder);
	for (auto &file_entry : dir_iter) {
		if (file_entry.is_regular_file()) {
			std::string filepath(file_entry.path().string());
			if (filepath.ends_with(".scene.json")) {
				nlohmann::json json;

				std::ifstream filestream(file_entry.path());
				filestream >> json;
				filestream.close();

				Scene scene;
				scene.id = json["id"];
				scene.name = json["name"];
				scene.script_name = json["script_name"];

				scene.fill_color[0] = json["fill_color"][0];
				scene.fill_color[1] = json["fill_color"][1];
				scene.fill_color[2] = json["fill_color"][2];

				scenes.push_back(scene);
			}
		}
	}
}

bool Project::Open(const char *path, App *app) {
	console.AddLog("Opening project at '%s'...", path);

	std::string project_filepath(path);
	if (!project_settings.LoadFromFile(app, project_filepath)) {
		return false;
	}

	LoadFromDisk(project_settings.project_directory);

	SDL_SetWindowTitle(app->window, ("NGine - " + project_settings.project_name + " - " +
									 project_settings.project_directory)
										.c_str());

	app->state.project_settings_screen.FromProjectSettings(project_settings);

	app->engine_settings.SetLastOpenedProject(project_filepath);

	ReloadScripts(app);
	ReloadImages(app->renderer);
	ReloadSounds();
	ReloadGeneralFiles();

	assets = Asset::BuildAsset(project_settings.project_directory);

	SaveToDisk(project_settings.project_directory);
	project_settings.SaveToDisk();

	console.AddLog("Project opened.");

	return true;
}

void Project::ReloadImages(SDL_Renderer *renderer) {
	images.clear();

	std::filesystem::path folder = project_settings.project_directory + "/.ngine/sprites";
	if (!std::filesystem::exists(folder)) {
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iter(folder);
	for (auto &file_entry : dir_iter) {
		if (file_entry.is_regular_file()) {
			std::string filepath(file_entry.path().string());
			if (filepath.ends_with(".sprite.json")) {
				auto image = std::make_unique<LibdragonImage>();
				image->LoadFromDisk(filepath);
				image->LoadImage(project_settings.project_directory, renderer);

				images.push_back(move(image));
			}
		}
	}
	std::sort(images.begin(), images.end(), libdragon_image_comparison);
}

void Project::ReloadScripts(App *app) {
	script_files.clear();

	std::filesystem::path script_folder = project_settings.project_directory + "/.ngine/scripts";
	if (!std::filesystem::exists(script_folder)) {
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iter(script_folder);
	for (auto &file_entry : dir_iter) {
		if (file_entry.is_regular_file()) {
			std::string filepath(file_entry.path().string());
			if (filepath.ends_with(".script.json")) {
				auto script = std::make_unique<LibdragonScript>();
				script->LoadFromDisk(filepath);
				script->LoadText(app);

				script_files.push_back(move(script));
			}
		}
	}
	std::sort(script_files.begin(), script_files.end(), libdragon_script_comparison);
}

void Project::ReloadSounds() {
	sounds.clear();

	std::filesystem::path folder = project_settings.project_directory + "/.ngine/sounds";
	if (!std::filesystem::exists(folder)) {
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iter(folder);
	for (auto &file_entry : dir_iter) {
		if (file_entry.is_regular_file()) {
			std::string filepath(file_entry.path().string());
			if (filepath.ends_with(".sound.json")) {
				auto sound = std::make_unique<LibdragonSound>(SOUND_UNKNOWN);
				sound->LoadFromDisk(filepath);

				sounds.push_back(move(sound));
			}
		}
	}
	std::sort(sounds.begin(), sounds.end(), libdragon_sound_comparison);
}

void Project::ReloadGeneralFiles() {
	general_files.clear();

	std::filesystem::path folder = project_settings.project_directory + "/.ngine/general";
	if (!std::filesystem::exists(folder)) {
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iter(folder);
	for (auto &file_entry : dir_iter) {
		if (file_entry.is_regular_file()) {
			std::string filepath(file_entry.path().string());
			if (filepath.ends_with(".general.json")) {
				auto file = std::make_unique<LibdragonFile>();
				file->LoadFromDisk(filepath);

				general_files.push_back(move(file));
			}
		}
	}
	std::sort(general_files.begin(), general_files.end(), libdragon_file_comparison);
}

void Project::ReloadAssets() {
	if (assets)
		delete assets;

	assets = Asset::BuildAsset(project_settings.project_directory);
}

void Project::Close(App *app) {
	scenes.clear();
	script_files.clear();
	images.clear();
	sounds.clear();

	delete assets;

	project_settings = ProjectSettings(app);
}

Project::~Project() {
	scenes.clear();
	script_files.clear();
	images.clear();
	sounds.clear();

	delete assets;
}
