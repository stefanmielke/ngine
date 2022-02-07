#include <filesystem>

#include "App.h"
#include "AppGui.h"
#include "ConsoleApp.h"
#include "DroppedAssets.h"
#include "Sdl.h"

ConsoleApp console;

int main() {
	App app;
	app.engine_settings.LoadFromDisk();
	app.state = ProjectState(app.engine_settings);

	Sdl::Init(&app.window, &app.renderer, app.default_title);

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
						std::string file(event.drop.file);
						if (file.ends_with(".png")) {
							DroppedImage dropped_image(event.drop.file);

							std::filesystem::path filepath(event.drop.file);
							strcpy(dropped_image.name,
								   filepath.filename().replace_extension().c_str());

							dropped_image.image_data = IMG_LoadTexture(app.renderer,
																	   event.drop.file);

							int w, h;
							SDL_QueryTexture(dropped_image.image_data, nullptr, nullptr, &w, &h);

							const float max_size = 300.f;
							if (w > h) {
								h = (int)(((float)h / (float)w) * max_size);
								w = (int)max_size;
							} else {
								w = (int)(((float)w / (float)h) * max_size);
								h = (int)max_size;
							}

							dropped_image.w = w;
							dropped_image.h = h;

							app.state.dropped_image_files.push_back(dropped_image);

							ImGui::SetWindowFocus("Import Assets");
						} else if (file.ends_with(".wav")) {
							DroppedSound dropped_sound(event.drop.file);
							std::filesystem::path filepath(event.drop.file);
							strcpy(dropped_sound.name,
								   filepath.filename().replace_extension().c_str());

							app.state.dropped_sound_files.push_back(dropped_sound);

							ImGui::SetWindowFocus("Import Assets");
						}
					} else {
						std::filesystem::path dropped_path(event.drop.file);
						if (std::filesystem::is_directory(dropped_path)) {
							app.OpenProject(dropped_path.c_str());
						} else if (std::filesystem::is_regular_file(dropped_path)) {
							app.OpenProject(dropped_path.parent_path().c_str());
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

	app.project.Close();

	Sdl::Quit(app.window, app.renderer);

	return 0;
}
