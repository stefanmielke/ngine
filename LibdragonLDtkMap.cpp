#include "LibdragonLDtkMap.h"

#include <fstream>
#include <LDtkLoader/World.hpp>

#include "App.h"
#include "ConsoleApp.h"
#include "json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_custom.h"
#include "LDtkLoader/Project.hpp"

extern App *g_app;

LibdragonLDtkMap::LibdragonLDtkMap() : dfs_folder("/") {
}

void LibdragonLDtkMap::SaveToDisk(const std::string &project_directory) {
	nlohmann::json json;
	json["name"] = name;
	json["file_path"] = file_path;
	json["dfs_folder"] = dfs_folder;

	std::string directory = project_directory + "/.ngine/ldtk_maps/";
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directories(directory);

	std::string filepath = directory + name + ".ldtk_maps.json";

	std::ofstream filestream(filepath);
	filestream << json.dump(4) << std::endl;
	filestream.close();
}

void LibdragonLDtkMap::LoadFromDisk(const std::string &filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
	file_path = json["file_path"];
	dfs_folder = json["dfs_folder"];

	LoadLayers();
}

void LibdragonLDtkMap::DeleteFromDisk(const std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/ldtk_maps/" + name + ".ldtk_maps.json";
	std::filesystem::remove(json_filepath);

	std::string filepath = project_directory + "/" + file_path;
	std::filesystem::remove(filepath);
}

void LibdragonLDtkMap::LoadLayers() {
	std::string full_file_path = g_app->project.project_settings.project_directory + "/" +
								 file_path;
	layers = LoadLayers(full_file_path);
}

std::vector<LibdragonMapLayer> LibdragonLDtkMap::LoadLayers(const std::string &file_path) {
	ldtk::Project project;

	try {
		project.loadFromFile(file_path);
	} catch (std::exception &ex) {
		console.AddLog("Error loading ldtk map: %s", ex.what());
		return std::vector<LibdragonMapLayer>(0);
	}

	std::vector<LibdragonMapLayer> layers;

	for (auto &world : project.allWorlds()) {
		for (const auto &level : world.allLevels()) {
			std::string level_name(level.name);
			for (const auto &ldtk_layer : level.allLayers()) {
				if (ldtk_layer.allTiles().empty())
					continue;

				LibdragonMapLayer layer;
				layer.name = level_name + "/" + ldtk_layer.getName();

				layers.push_back(layer);
			}
		}
	}

	return layers;
}

void LibdragonLDtkMap::DrawTooltip() const {
	std::stringstream tooltip;
	tooltip << "Path: " << file_path << "\n\nLayers:";
	for (auto &layer : layers) {
		tooltip << "\n- " << layer.name << " (DFS Path: " << dfs_folder << name << "/" << layer.name
				<< ".map)";
	}

	ImGui::BeginTooltip();
	render_badge("map", ImVec4(.4f, .1f, .1f, 0.7f));
	ImGui::SameLine();
	render_badge("ldtk", ImVec4(.4f, .1f, .1f, 0.7f));
	ImGui::SameLine();
	ImGui::Text("%s", name.c_str());
	ImGui::Separator();

	ImGui::Text("%s", tooltip.str().c_str());
	ImGui::EndTooltip();
}
