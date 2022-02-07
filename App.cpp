#include "App.h"

#include "ConsoleApp.h"

App::App()
	: renderer(nullptr),
	  window(nullptr),
	  engine_settings(),
	  project(),
	  state(engine_settings),
	  is_running(true) {
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
