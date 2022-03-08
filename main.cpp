#include <filesystem>

#include "App.h"
#include "AppGui.h"
#include "ConsoleApp.h"
#include "DroppedAssets.h"
#include "Sdl.h"

ConsoleApp console;

int main(int argv, char **args) {
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
								   filepath.filename().replace_extension().string().c_str());

							dropped_image.image_data = IMG_LoadTexture(app.renderer,
																	   event.drop.file);

							int w, h;
							SDL_QueryTexture(dropped_image.image_data, nullptr, nullptr, &w, &h);

							const float max_size = 300.f;
							if (w > h) {
								dropped_image.height_mult = (float)h / (float)w;
								h = (int)(dropped_image.height_mult * max_size);
								w = (int)max_size;
							} else {
								dropped_image.width_mult = (float)w / (float)h;
								w = (int)(dropped_image.width_mult * max_size);
								h = (int)max_size;
							}

							dropped_image.w = w;
							dropped_image.h = h;

							app.state.dropped_image_files.push_back(dropped_image);

							ImGui::SetWindowFocus("Import Assets");
						} else if (file.ends_with(".wav") || file.ends_with(".xm") ||
								   file.ends_with(".ym")) {
							LibdragonSoundType type;
							if (file.ends_with(".wav"))
								type = SOUND_WAV;
							else if (file.ends_with(".xm"))
								type = SOUND_XM;
							else
								type = SOUND_YM;

							DroppedSound dropped_sound(event.drop.file, type);
							std::filesystem::path filepath(event.drop.file);
							strcpy(dropped_sound.name,
								   filepath.filename().replace_extension().string().c_str());

							app.state.dropped_sound_files.push_back(dropped_sound);

							ImGui::SetWindowFocus("Import Assets");
						} else {
							DroppedGeneralFile dropped_file(event.drop.file);

							std::filesystem::path filepath(event.drop.file);
							strcpy(dropped_file.name,
								   filepath.filename().replace_extension().string().c_str());
							strcpy(dropped_file.extension, filepath.extension().string().c_str());

							app.state.dropped_general_files.push_back(dropped_file);

							ImGui::SetWindowFocus("Import Assets");
						}
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

	app.project.Close();

	Sdl::Quit(app.window, app.renderer);

	return 0;
}
