#include "App.h"

#include <fstream>
#include <utility>

#include "ConsoleApp.h"
#include "json.hpp"
#include "ThreadCommand.h"

const Uint32 docker_check_interval_ok = 60 * 1000;
const Uint32 docker_check_interval_error = 10 * 1000;

Uint32 docker_check_callback(Uint32 interval, void *param) {
	App *app = (App *)param;
	if (app) {
		app->engine_settings.ReloadDockerVersion();
		if (app->engine_settings.GetDockerVersion().starts_with("docker")) {
			return docker_check_interval_ok;
		}
	}
	return docker_check_interval_error;
}

App::App(std::string engine_directory)
	: renderer(nullptr),
	  window(nullptr),
	  engine_settings(),
	  project(this),
	  state(engine_settings),
	  audio_sample(nullptr),
	  audio_state(SS_STOPPED),
	  is_running(true),
	  engine_directory(std::move(engine_directory)) {
	SDL_AddTimer(docker_check_interval_error, &docker_check_callback, this);
}

bool App::LoadAssets() {
	std::string texture_path(engine_directory);
	texture_path.append("/sheet.png");
	app_texture = IMG_LoadTexture(renderer, texture_path.c_str());
	if (!app_texture) {
		return false;
	}

	std::string json_path(engine_directory);
	json_path.append("/sheet.json");
	std::ifstream filestream(json_path);
	if (!filestream.is_open()) {
		return false;
	}

	nlohmann::json json;
	filestream >> json;
	filestream.close();

	image_size.x = json["meta"]["size"]["w"];
	image_size.y = json["meta"]["size"]["h"];

	for (auto &frame : json["frames"]) {
		GUIImage image;
		image.name = frame["filename"];
		int x1 = frame["frame"]["x"];
		int y1 = frame["frame"]["y"];
		int x2 = x1 + (int)frame["frame"]["w"];
		int y2 = y1 + (int)frame["frame"]["h"];

		float uvx1 = x1 / image_size.x;
		float uvx2 = x2 / image_size.x;
		float uvy1 = y1 / image_size.y;
		float uvy2 = y2 / image_size.y;

		if (uvx2 > 1)
			uvx2 = 1.f;
		if (uvy2 > 1)
			uvy2 = 1.f;

		image.position = ImVec4(uvx1, uvy1, uvx2, uvy2);
		images.push_back(image);
	}

	return true;
}

void App::GetImagePosition(std::string name, ImVec2 &uv0, ImVec2 &uv1) {
	// TODO: change importer to use enum instead
	for (auto &image : images) {
		if (image.name == name) {
			uv0.x = image.position.x;
			uv0.y = image.position.y;
			uv1.x = image.position.z;
			uv1.y = image.position.w;
			return;
		}
	}
}

bool App::OpenProject(const std::string &path) {
	if (project.project_settings.IsOpen()) {
		CloseProject();
	}

	return project.Open(path.c_str(), this);
}

void App::CloseProject() {
	console.AddLog("Closing Project...");

	project.Close(this);
	state = ProjectState(engine_settings);

	SDL_SetWindowTitle(window, default_title);

	console.AddLog("Project closed.");
}

void App::OpenUrl(const char *url) {
#ifdef __LINUX__
	std::string command("xdg-open ");
#else
	std::string command("start ");
#endif
	command.append(url);

	ThreadCommand::RunCommandDetached(command);
}
