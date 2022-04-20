#include <filesystem>

#include "App.h"
#include "AppGui.h"
#include "ConsoleApp.h"
#include "DroppedAssets.h"
#include "Sdl.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

ConsoleApp console;

App *g_app;

std::string GetExeDirectory() {
#ifdef _WIN32
	wchar_t szPath[MAX_PATH];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
#else
	char szPath[4096];
	ssize_t count = readlink("/proc/self/exe", szPath, 4096);
	if (count < 0 || count >= 4096)
		return {};
	szPath[count] = '\0';
#endif
	return std::filesystem::path{szPath}.parent_path().string();
}

int main(int argv, char **args) {
	App app(GetExeDirectory());
	app.engine_settings.LoadFromDisk(&app);
	app.state = ProjectState(app.engine_settings);

	g_app = &app;

	Sdl::Init(&app);

	app.LoadAssets();

	AppGui::ChangeTheme(app, app.engine_settings.GetTheme());

	while (app.is_running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			Sdl::ProcessEvent(&event);

			switch (event.type) {
				case SDL_QUIT:
					app.is_running = false;
					break;
				case SDL_DROPFILE: {
					console.AddLog("Dropped file: %s", event.drop.file);

					if (app.project.project_settings.IsOpen()) {
						AppGui::ProcessImportFile(app, event.drop.file);
					} else {
						std::filesystem::path dropped_path(event.drop.file);
						if (std::filesystem::is_directory(dropped_path)) {
							app.OpenProject(dropped_path.string());
						} else if (std::filesystem::is_regular_file(dropped_path)) {
							app.OpenProject(dropped_path.parent_path().string());
						}
					}
				} break;
				default:
					break;
			}
		}

		if (!app.is_running)
			break;

		Sdl::NewFrame();

		AppGui::Update(app);

		Sdl::RenderStart(app.renderer);

		// render SDL stuff here

		Sdl::RenderEnd(app.renderer);
	}

	app.project.Close(&app);

	Sdl::Quit(&app, app.window, app.renderer);

	return 0;
}
