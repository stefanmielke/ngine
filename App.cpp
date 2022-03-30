#include "App.h"

#include <utility>

#include "ConsoleApp.h"

App::App(std::string engine_directory)
	: renderer(nullptr),
	  window(nullptr),
	  engine_settings(),
	  project(),
	  state(engine_settings),
	  is_running(true),
	  engine_directory(std::move(engine_directory)) {
}

bool App::OpenProject(const std::string &path) {
	if (project.project_settings.IsOpen()) {
		CloseProject();
	}

	return project.Open(path.c_str(), this);
}

void App::CloseProject() {
	console.AddLog("Closing Project...");

	project.Close();
	state = ProjectState(engine_settings);

	SDL_SetWindowTitle(window, default_title);

	console.AddLog("Project closed.");
}
