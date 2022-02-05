#include "ProjectSettings.h"

#include <fstream>

#include "../ConsoleApp.h"
#include "../json.hpp"

extern ConsoleApp console;

ProjectSettings::ProjectSettings()
	: is_open(false),
	  next_scene_id(0),
	  project_name("Hello NGine"),
	  rom_name("hello_ngine"),
	  initial_screen_id(0),
	  global_mem_alloc_size(1024),
	  scene_mem_alloc_size(1024 * 2) {
}

bool ProjectSettings::LoadFromFile(std::string &folder) {
	project_directory = folder;

	if (!std::filesystem::exists(project_directory + "/ngine.project.json")) {
		console.AddLog("Project file not found (is the project still being created?).");
		return false;
	}

	std::ifstream project_file(project_directory + "/ngine.project.json");

	nlohmann::json json;
	project_file >> json;
	project_file.close();

	project_name = json["project"]["name"];
	next_scene_id = json["project"]["next_scene_id"];
	rom_name = json["project"]["rom"];
	global_script_name = json["project"]["global_script_name"];

	global_mem_alloc_size = json["project"]["global_mem_alloc_size"];
	scene_mem_alloc_size = json["project"]["scene_mem_alloc_size"];
	initial_screen_id = json["project"]["initial_screen_id"];

	display.SetResolution(json["display"]["resolution"]);
	display.SetBitDepth(json["display"]["bit_depth"]);
	display.buffers = json["display"]["buffers"];
	display.SetGamma(json["display"]["gamma"]);
	display.SetAntialias(json["display"]["antialias"]);

	modules.console = json["modules"]["console"];
	modules.controller = json["modules"]["controller"];
	modules.debug_usb = json["modules"]["debug_usb"];
	modules.debug_is_viewer = json["modules"]["debug_is_viewer"];
	modules.dfs = json["modules"]["dfs"];
	modules.display = json["modules"]["display"];
	modules.rdp = json["modules"]["rdp"];
	modules.timer = json["modules"]["timer"];

	is_open = true;
	return true;
}

void ProjectSettings::SaveToDisk() {
	if (project_directory.empty())
		return;

	nlohmann::json json;

	json["project"]["name"] = project_name;
	json["project"]["next_scene_id"] = next_scene_id;
	json["project"]["rom"] = rom_name;
	json["project"]["global_mem_alloc_size"] = global_mem_alloc_size;
	json["project"]["scene_mem_alloc_size"] = scene_mem_alloc_size;
	json["project"]["initial_screen_id"] = initial_screen_id;
	json["project"]["global_script_name"] = global_script_name;

	json["display"]["resolution"] = display.GetResolution();
	json["display"]["bit_depth"] = display.GetBitDepth();
	json["display"]["buffers"] = display.buffers;
	json["display"]["gamma"] = display.GetGamma();
	json["display"]["antialias"] = display.GetAntialias();

	json["modules"]["console"] = modules.console;
	json["modules"]["controller"] = modules.controller;
	json["modules"]["debug_usb"] = modules.debug_usb;
	json["modules"]["debug_is_viewer"] = modules.debug_is_viewer;
	json["modules"]["dfs"] = modules.dfs;
	json["modules"]["display"] = modules.display;
	json["modules"]["rdp"] = modules.rdp;
	json["modules"]["timer"] = modules.timer;

	std::ofstream project_file(project_directory + "/ngine.project.json");

	project_file << json.dump(4);

	project_file.close();
}

void ProjectSettings::CloseProject() {
	is_open = false;
	project_directory.clear();
}
