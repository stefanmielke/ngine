#include <filesystem>
#include <fstream>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "ConsoleApp.h"
#include "DroppedAssets.h"
#include "Emulator.h"
#include "json.hpp"
#include "LibdragonImage.h"
#include "LibdragonSound.h"
#include "ProjectBuilder.h"
#include "ProjectState.h"
#include "ScriptBuilder.h"
#include "VSCode.h"
#include "settings/EngineSettings.h"
#include "settings/Project.h"
#include "settings/ProjectSettings.h"
#include "settings/ProjectSettingsScreen.h"

const char *default_title = "NGine - N64 Engine Powered by Libdragon";

ConsoleApp console;

static ProjectState state;

static Project project;
static std::vector<std::string> script_files;
static std::vector<std::unique_ptr<LibdragonSound>> sounds;
static std::vector<std::unique_ptr<LibdragonImage>> images;

static ProjectSettings project_settings;
static EngineSettings engine_settings;

bool open_project(const char *path);

static bool update_gui(SDL_Window *window);
static void render_image_import_windows();
static void reload_scripts();
static void load_sounds();
static void load_images();
static void load_image(std::unique_ptr<LibdragonImage> &image);

struct App {
	SDL_Renderer *renderer;
	SDL_Window *window;
} app;

static void initSDL() {
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	windowFlags = SDL_WINDOW_RESIZABLE;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	IMG_Init(IMG_INIT_PNG);

	app.window = SDL_CreateWindow(default_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								  1024, 768, windowFlags);

	if (!app.window) {
		printf("Failed to open window: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

	if (!app.renderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForSDLRenderer(app.window);
	ImGui_ImplSDLRenderer_Init(app.renderer);
}

int main() {
	engine_settings.LoadFromDisk();
	strcpy(state.input_open_project, engine_settings.GetLastOpenedProject().c_str());
	strcpy(state.emulator_path, engine_settings.GetEmulatorPath().c_str());

	initSDL();

	bool is_running = true;
	while (is_running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);

			switch (event.type) {
				case SDL_QUIT:
					is_running = false;
					break;
				case SDL_DROPFILE: {
					console.AddLog("Dropped file: %s", event.drop.file);

					if (project_settings.IsOpen()) {
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

							state.dropped_image_files.push_back(dropped_image);

							ImGui::SetWindowFocus("Import Assets");
						} else if (file.ends_with(".wav")) {
							DroppedSound dropped_sound(event.drop.file);
							std::filesystem::path filepath(event.drop.file);
							strcpy(dropped_sound.name,
								   filepath.filename().replace_extension().c_str());

							state.dropped_sound_files.push_back(dropped_sound);

							ImGui::SetWindowFocus("Import Assets");
						}
					} else {
						std::filesystem::path dropped_path(event.drop.file);
						if (std::filesystem::is_directory(dropped_path)) {
							open_project(dropped_path.c_str());
						} else if (std::filesystem::is_regular_file(dropped_path)) {
							open_project(dropped_path.parent_path().c_str());
						}
					}
				} break;
				default:
					break;
			}
		}

		if (!is_running)
			break;

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		is_running = update_gui(app.window);

		ImGui::Render();

		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
		SDL_RenderClear(app.renderer);

		// render SDL stuff here

		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(app.renderer);
	}

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(app.renderer);
	SDL_DestroyWindow(app.window);
	IMG_Quit();
	SDL_Quit();

	return 0;
}

bool open_project(const char *path) {
	console.AddLog("Opening project at '%s'...", path);

	if (project_settings.IsOpen()) {
		project_settings.CloseProject();
	}

	std::string project_filepath(path);
	if (!project_settings.LoadFromFile(project_filepath)) {
		return false;
	}

	project.LoadFromDisk(project_settings.project_directory);

	SDL_SetWindowTitle(app.window, ("NGine - " + project_settings.project_name + " - " +
									project_settings.project_directory)
									   .c_str());

	state.project_settings_screen.FromProjectSettings(project_settings);

	engine_settings.SetLastOpenedProject(project_filepath);

	reload_scripts();

	load_images();
	load_sounds();

	console.AddLog("Project opened.");

	return true;
}

bool update_gui(SDL_Window *window) {
	bool is_output_open = true;

	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);

	static bool new_project_window_open = false;
	static bool open_project_window_open = false;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Project")) {
				new_project_window_open = true;
			}
			if (ImGui::MenuItem("Open Project")) {
				open_project_window_open = true;
			}
			if (ImGui::MenuItem("Close Project", nullptr, false, project_settings.IsOpen())) {
				console.AddLog("Closing project...");

				project_settings.CloseProject();
				SDL_SetWindowTitle(window, default_title);

				console.AddLog("Project closed.");
			}
			if (ImGui::MenuItem("Exit")) {
				return false;
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Save Project", nullptr, false, project_settings.IsOpen())) {
			console.AddLog("Saving project...");

			project.SaveToDisk(project_settings.project_directory);
			project_settings.SaveToDisk();

			console.AddLog("Project saved.");
		}
		if (ImGui::MenuItem("Build", nullptr, false, project_settings.IsOpen())) {
			console.AddLog("Building project...");

			ProjectBuilder::Build(project_settings, project, images, sounds);
		}
		if (ImGui::BeginMenu("Tasks", project_settings.IsOpen())) {
			if (ImGui::MenuItem("Clean/Build")) {
				console.AddLog("Rebuilding project...");

				ProjectBuilder::Rebuild(project_settings, project, images, sounds);
			}
			if (ImGui::MenuItem("Regen Static Files")) {
				console.AddLog("Regenerating static files...");

				ProjectBuilder::GenerateStaticFiles(project_settings.project_directory);

				console.AddLog("Files regenerated.");
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Open in VSCode", nullptr, false, project_settings.IsOpen())) {
			console.AddLog("Opening project in VSCode...");

			VSCode::OpenPath(project_settings.project_directory);
		}
		if (ImGui::MenuItem(
				"Run", nullptr, false,
				project_settings.IsOpen() && !engine_settings.GetEmulatorPath().empty())) {
			Emulator::Run(engine_settings, project_settings, project, images, sounds);
		}
		ImGui::EndMainMenuBar();
	}

	if (new_project_window_open) {
		ImGui::SetNextWindowSize(ImVec2(300, 80));
		if (ImGui::Begin("New Project", &new_project_window_open)) {
			ImGui::TextUnformatted("Folder");
			ImGui::SameLine();
			ImGui::InputText("##", state.input_new_project, 255);
			if (ImGui::Button("Create", ImVec2(50, 20))) {
				std::string new_project_folder(state.input_new_project);

				ProjectBuilder::Create(new_project_folder);

				new_project_window_open = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(50, 20))) {
				new_project_window_open = false;
			}
			ImGui::End();
		}
	}
	if (open_project_window_open) {
		ImGui::SetNextWindowSize(ImVec2(300, 80));
		if (ImGui::Begin("Open Project", &open_project_window_open)) {
			ImGui::TextUnformatted("Folder");
			ImGui::SameLine();
			ImGui::InputText("##", state.input_open_project, 255);
			if (ImGui::Button("Open", ImVec2(50, 20))) {
				if (open_project(state.input_open_project)) {
					open_project_window_open = false;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(50, 20))) {
				open_project_window_open = false;
			}
			ImGui::End();
		}
	}

	render_image_import_windows();

	console.Draw("Output", window, is_output_open);

	const float center_x_size = (float)window_width - 600;
	const float center_y_offset = is_output_open ? 219 : 38;
	ImGui::SetNextWindowSize(ImVec2(center_x_size, (float)window_height - center_y_offset));
	ImGui::SetNextWindowPos(ImVec2(300, 19));
	if (ImGui::Begin("ContentBrowser", nullptr,
					 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
		if (ImGui::BeginTabBar("CenterContentTabs",
							   ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
			if (ImGui::BeginTabItem("Sprites Browser")) {
				if (project_settings.IsOpen()) {
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						ImGui::OpenPopup("PopupSpritesBrowser");
					}

					if (ImGui::BeginPopup("PopupSpritesBrowser")) {
						if (ImGui::Selectable("Refresh")) {
							load_images();
						}
						ImGui::EndPopup();
					}

					const int item_size = 100;
					int items_per_line = std::floor(ImGui::GetWindowWidth() / (float)item_size);
					if (items_per_line < 1)
						items_per_line = 1;

					int cur_i = 0;
					if (ImGui::BeginPopup("PopupSpritesBrowserImage")) {
						if (ImGui::Selectable("Edit Settings")) {
							if (state.selected_image) {
								state.image_editing = state.selected_image;
								state.reload_image_edit = true;
								state.selected_image = nullptr;
							}
						}
						if (ImGui::Selectable("Copy DFS Path")) {
							if (state.selected_image) {
								std::string dfs_path((*state.selected_image)->dfs_folder +
													 (*state.selected_image)->name + ".sprite");
								ImGui::SetClipboardText(dfs_path.c_str());
								state.selected_image = nullptr;
							}
						}
						if (ImGui::Selectable("Delete")) {
							if (state.selected_image) {
								(*state.selected_image)
									->DeleteFromDisk(project_settings.project_directory);

								for (int i = 0; i < images.size(); ++i) {
									if (images[i]->image_path == (*state.selected_image)->image_path) {
										images.erase(images.begin() + i);
										break;
									}
								}
								state.selected_image = nullptr;
							}
						}
						ImGui::EndPopup();
					}

					for (auto &image : images) {
						if (ImGui::ImageButton((ImTextureID)(intptr_t)image->loaded_image,
											   ImVec2(item_size, item_size))) {
							state.selected_image = &image;
							ImGui::OpenPopup("PopupSpritesBrowserImage");
						}
						if (ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text(
								"%s\nPath: %s\nDFS Path: %s%s.sprite\nSize: %dx%d\nSlices: %dx%d\n",
								image->name.c_str(), image->image_path.c_str(),
								image->dfs_folder.c_str(), image->name.c_str(), image->width,
								image->height, image->h_slices, image->v_slices);
							ImGui::Image((ImTextureID)(intptr_t)image->loaded_image,
										 ImVec2(image->width, image->height));
							ImGui::EndTooltip();
						}
						++cur_i;

						if (cur_i % items_per_line != 0)
							ImGui::SameLine();
					}
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Sounds Browser")) {
				if (project_settings.IsOpen()) {
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						ImGui::OpenPopup("PopupSoundsBrowser");
					}

					if (ImGui::BeginPopup("PopupSoundsBrowser")) {
						if (ImGui::Selectable("Refresh")) {
							load_sounds();
						}
						ImGui::EndPopup();
					}

					int cur_i = 0;
					if (ImGui::BeginPopup("PopupSoundsBrowserSound")) {
						if (ImGui::Selectable("Edit Settings")) {
							if (state.selected_sound) {
								state.sound_editing = state.selected_sound;
								state.reload_sound_edit = true;
								state.selected_sound = nullptr;
							}
						}
						if (ImGui::Selectable("Copy DFS Path")) {
							if (state.selected_sound) {
								std::string dfs_path((*state.selected_sound)->dfs_folder +
													 (*state.selected_sound)->name + ".wav64");
								ImGui::SetClipboardText(dfs_path.c_str());
								state.selected_sound = nullptr;
							}
						}
						if (ImGui::Selectable("Delete")) {
							if (state.selected_sound) {
								(*state.selected_sound)
									->DeleteFromDisk(project_settings.project_directory);

								for (int i = 0; i < sounds.size(); ++i) {
									if (sounds[i]->sound_path == (*state.selected_sound)->sound_path) {
										sounds.erase(sounds.begin() + i);
										break;
									}
								}
								state.selected_sound = nullptr;
							}
						}
						ImGui::EndPopup();
					}

					for (auto &sound : sounds) {
						if (ImGui::Selectable(sound->name.c_str())) {
							state.selected_sound = &sound;
							ImGui::OpenPopup("PopupSoundsBrowserSound");
						}
						if (ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("%s", sound->GetTooltip().c_str());
							ImGui::EndTooltip();
						}
						++cur_i;
					}
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Script Browser")) {
				if (project_settings.IsOpen()) {
					static char script_name_input[100] = {};
					ImGui::InputText("Script Name", script_name_input, 100);
					ImGui::SameLine();
					if (ImGui::Button("Create Script File")) {
						std::string script_name(script_name_input);
						if (!script_name.empty()) {
							ScriptBuilder::CreateScriptFile(project_settings, script_name_input);
							memset(script_name_input, 0, 100);

							reload_scripts();
						}
					}
					ImGui::Separator();
					for (auto &script_name : script_files) {
						ImGui::TextUnformatted(script_name.c_str());
						ImGui::SameLine();
						ImGui::PushID(script_name.c_str());
						if (ImGui::SmallButton("Edit")) {
							std::string path = project_settings.project_directory +
											   "/src/scripts/" + script_name + ".script.c";
							VSCode::OpenPath(path);
						}
						ImGui::PopID();
						ImGui::SameLine();
						ImGui::PushID((script_name + "D").c_str());
						if (ImGui::SmallButton("Delete")) {
							ScriptBuilder::DeleteScriptFile(project_settings, project,
															script_name.c_str());

							for (int i = 0; i < script_files.size(); ++i) {
								if (script_files[i] == script_name) {
									script_files.erase(script_files.begin() + i);
								}
							}
						}
						ImGui::PopID();
					}
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	const float prop_y_size = is_output_open ? 219 : 38;
	ImGui::SetNextWindowSize(ImVec2(300, (float)window_height - prop_y_size));
	ImGui::SetNextWindowPos(ImVec2(0, 19));
	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		if (state.current_scene) {
			if (ImGui::BeginTabBar("Properties", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
				//				if (ImGui::BeginTabItem("Nodes")) {
				//					if (ImGui::TreeNodeEx("Root Node")) {
				//						{
				//							ImGui::SameLine();
				//							if (ImGui::Selectable("Props")) {
				//								// do whatever
				//							}
				//						}
				//						{
				//							if (ImGui::Selectable("Test Node Press")) {
				//								// do whatever
				//							}
				//						}
				//
				//						if (ImGui::TreeNode("Test Node")) {
				//							ImGui::TreePop();
				//						}
				//						if (ImGui::TreeNode("Test Node 2")) {
				//							ImGui::TreePop();
				//						}
				//						ImGui::TreePop();
				//					}
				//					ImGui::EndTabItem();
				//				}
				if (ImGui::BeginTabItem("Settings")) {
					ImGui::TextColored(ImColor(100, 100, 255), "Id: %d", state.current_scene->id);
					ImGui::Spacing();
					ImGui::Separator();

					ImGui::Spacing();
					ImGui::InputText("Name", state.scene_name, 100);

					ImGui::Spacing();
					ImGui::TextUnformatted("Background Fill Color");
					ImGui::ColorPicker3("##FillColor", state.current_scene->fill_color);

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
					{
						std::string current_selected(state.current_scene->script_name);
						ImGui::TextUnformatted("Attached Script");
						if (ImGui::BeginCombo("##AttachedScript", current_selected.c_str())) {
							for (auto &script : script_files) {
								if (ImGui::Selectable(script.c_str(), script == current_selected)) {
									state.current_scene->script_name = script;
								}
							}
							ImGui::EndCombo();
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Remove")) {
							state.current_scene->script_name.clear();
						}
					}
					ImGui::Spacing();

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Save")) {
						state.current_scene->name = state.scene_name;
						project.SaveToDisk(project_settings.project_directory);
						project_settings.SaveToDisk();
					}
					ImGui::Spacing();

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Delete Scene")) {
						for (int i = 0; i < project.scenes.size(); ++i) {
							if (project.scenes[i].id == state.current_scene->id) {
								project.scenes.erase(project.scenes.begin() + i);
								std::string filename = project_settings.project_directory +
													   "/.ngine/scenes/" +
													   std::to_string(state.current_scene->id) +
													   ".scene.json";
								std::filesystem::remove(filename);

								state.current_scene = nullptr;
								break;
							}
						}
					}

					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		} else {
			ImGui::TextWrapped("Load a scene on the right side under 'Scenes' tab.");
		}
	}
	ImGui::End();

	const float prop_x_size = 300;
	ImGui::SetNextWindowSize(ImVec2(prop_x_size, (float)window_height - prop_y_size));
	ImGui::SetNextWindowPos(ImVec2((float)window_width - prop_x_size, 19));
	if (ImGui::Begin("General Settings", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_NoTitleBar)) {
		if (ImGui::BeginTabBar("Properties", ImGuiTabBarFlags_AutoSelectNewTabs)) {
			if (state.image_editing) {
				static char image_edit_name[50];
				static char image_edit_dfs_folder[100];
				static int image_edit_h_slices = 0;
				static int image_edit_v_slices = 0;
				if (state.reload_image_edit) {
					state.reload_image_edit = false;

					strcpy(image_edit_name, (*state.image_editing)->name.c_str());
					strcpy(image_edit_dfs_folder, (*state.image_editing)->dfs_folder.c_str());
					image_edit_h_slices = (*state.image_editing)->h_slices;
					image_edit_v_slices = (*state.image_editing)->v_slices;
				}
				if (ImGui::BeginTabItem("Image Settings")) {
					ImGui::Image((ImTextureID)(intptr_t)(*state.image_editing)->loaded_image,
								 ImVec2((*state.image_editing)->display_width * 2,
										(*state.image_editing)->display_height * 2));
					ImGui::Separator();
					ImGui::Spacing();
					ImGui::InputText("Name", image_edit_name, 50);
					ImGui::InputText("DFS Folder", image_edit_dfs_folder, 100);
					ImGui::InputInt("H Slices", &image_edit_h_slices);
					ImGui::InputInt("V Slices", &image_edit_v_slices);

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Save")) {
						bool will_save = true;
						if ((*state.image_editing)->name != image_edit_name) {
							std::string name_string(image_edit_name);
							auto find_by_name = [&name_string](std::unique_ptr<LibdragonImage> &i) {
								return i->name == name_string;
							};
							if (std::find_if(images.begin(), images.end(), find_by_name) !=
								std::end(images)) {
								console.AddLog(
									"Image with the name already exists. Please choose a "
									"different name.");
								will_save = false;
							} else {
								std::filesystem::copy_file(project_settings.project_directory +
															   "/" + (*state.image_editing)->image_path,
														   project_settings.project_directory +
															   "/assets/sprites/" +
															   image_edit_name + ".png");
								(*state.image_editing)
									->DeleteFromDisk(project_settings.project_directory);
							}
						}

						if (will_save) {
							(*state.image_editing)->name = image_edit_name;
							(*state.image_editing)->dfs_folder = image_edit_dfs_folder;
							(*state.image_editing)->h_slices = image_edit_h_slices;
							(*state.image_editing)->v_slices = image_edit_v_slices;
							(*state.image_editing)->image_path = "assets/sprites/" +
														   (*state.image_editing)->name + ".png";

							(*state.image_editing)->SaveToDisk(project_settings.project_directory);

							state.image_editing = nullptr;
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel")) {
						state.image_editing = nullptr;
					}

					ImGui::EndTabItem();
				}
			}
			if (state.sound_editing) {
				static char sound_edit_name[50];
				static char sound_edit_dfs_folder[100];
				if (state.reload_sound_edit) {
					state.reload_sound_edit = false;

					strcpy(sound_edit_name, (*state.sound_editing)->name.c_str());
					strcpy(sound_edit_dfs_folder, (*state.sound_editing)->dfs_folder.c_str());
				}
				if (ImGui::BeginTabItem("Sound Settings")) {
					ImGui::InputText("Name", sound_edit_name, 50);
					ImGui::InputText("DFS Folder", sound_edit_dfs_folder, 100);

					if ((*state.sound_editing)->type == SOUND_WAV) {
						ImGui::Checkbox("Loop", &(*state.sound_editing)->wav_loop);
						if ((*state.sound_editing)->wav_loop) {
							ImGui::InputInt("Loop Offset", &(*state.sound_editing)->wav_loop_offset);
						}
					} else if ((*state.sound_editing)->type == SOUND_YM) {
						ImGui::Checkbox("Compress", &(*state.sound_editing)->ym_compress);
					}

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Save")) {
						bool will_save = true;
						if ((*state.sound_editing)->name != sound_edit_name) {
							std::string name_string(sound_edit_name);
							auto find_by_name = [&name_string](std::unique_ptr<LibdragonSound> &i) {
								return i->name == name_string;
							};
							if (std::find_if(sounds.begin(), sounds.end(), find_by_name) !=
								std::end(sounds)) {
								console.AddLog(
									"Sound with the name already exists. Please choose a "
									"different name.");
								will_save = false;
							} else {
								std::filesystem::copy_file(project_settings.project_directory +
															   "/" + (*state.sound_editing)->sound_path,
														   project_settings.project_directory +
															   "/assets/sounds/" + sound_edit_name +
															   ".wav");
								(*state.sound_editing)
									->DeleteFromDisk(project_settings.project_directory);
							}
						}

						if (will_save) {
							(*state.sound_editing)->name = sound_edit_name;
							(*state.sound_editing)->dfs_folder = sound_edit_dfs_folder;
							(*state.sound_editing)->sound_path = "assets/sounds/" +
														   (*state.sound_editing)->name + ".wav";

							(*state.sound_editing)->SaveToDisk(project_settings.project_directory);

							state.sound_editing = nullptr;
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel")) {
						state.sound_editing = nullptr;
					}

					ImGui::EndTabItem();
				}
			}
			if (ImGui::BeginTabItem("Scenes")) {
				for (auto &scene : project.scenes) {
					if (ImGui::Selectable(scene.name.c_str(),
										  state.current_scene && scene.id == state.current_scene->id)) {
						state.current_scene = &scene;
						strcpy(state.scene_name, state.current_scene->name.c_str());
					}
				}

				ImGui::Separator();
				ImGui::Spacing();
				if (ImGui::Button("Create New Scene")) {
					project.scenes.emplace_back();
					state.current_scene = &project.scenes[project.scenes.size() - 1];
					state.current_scene->id = project_settings.next_scene_id++;
					state.current_scene->name = std::to_string(project.scenes.size());
					strcpy(state.scene_name, state.current_scene->name.c_str());
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Project")) {
				if (project_settings.IsOpen()) {
					if (ImGui::BeginTabBar("ProjectAllSettings")) {
						if (ImGui::BeginTabItem("General")) {
							ImGui::InputText("Name", state.project_settings_screen.project_name, 100);
							ImGui::InputText("Rom", state.project_settings_screen.rom_name, 100);

							ImGui::Separator();

							{
								std::string current_selected("None");
								for (auto &scene : project.scenes) {
									if (scene.id == project_settings.initial_screen_id) {
										current_selected = scene.name;
										break;
									}
								}
								ImGui::TextUnformatted("Initial Screen");
								if (ImGui::BeginCombo("##InitialScreen",
													  current_selected.c_str())) {
									for (auto &scene : project.scenes) {
										if (ImGui::Selectable(
												scene.name.c_str(),
												scene.id == project_settings.initial_screen_id)) {
											project_settings.initial_screen_id = scene.id;
										}
									}
									ImGui::EndCombo();
								}
							}

							{
								ImGui::TextUnformatted("Global Script");
								if (ImGui::BeginCombo(
										"##GlobalScript",
										project_settings.global_script_name.c_str())) {
									for (auto &script : script_files) {
										if (ImGui::Selectable(
												script.c_str(),
												script == project_settings.global_script_name)) {
											project_settings.global_script_name = script;
										}
									}
									ImGui::EndCombo();
								}
							}

							ImGui::Separator();

							ImGui::TextUnformatted("Global Memory Reserve (KB)");
							ImGui::InputInt("##GlobalMem", &project_settings.global_mem_alloc_size,
											1, 1024);
							ImGui::TextUnformatted("Scene Memory Reserve (KB)");
							ImGui::InputInt("##LocalMem", &project_settings.scene_mem_alloc_size, 1,
											1024);

							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Modules")) {
							if (ImGui::Checkbox("Audio", &project_settings.modules.audio)) {
								if (!project_settings.modules.audio)
									project_settings.modules.audio_mixer = false;
							}

							if (!project_settings.modules.audio)
								ImGui::BeginDisabled();
							ImGui::Checkbox("Audio Mixer", &project_settings.modules.audio_mixer);
							if (!project_settings.modules.audio)
								ImGui::EndDisabled();

							ImGui::Checkbox("Console", &project_settings.modules.console);
							ImGui::Checkbox("Controller", &project_settings.modules.controller);
							ImGui::Checkbox("Debug Is Viewer",
											&project_settings.modules.debug_is_viewer);
							ImGui::Checkbox("Debug USB", &project_settings.modules.debug_usb);
							ImGui::Checkbox("Display", &project_settings.modules.display);
							ImGui::Checkbox("DFS", &project_settings.modules.dfs);
							ImGui::Checkbox("RDP", &project_settings.modules.rdp);
							ImGui::Checkbox("Timer", &project_settings.modules.timer);

							ImGui::EndTabItem();
						}

						if (project_settings.modules.audio) {
							if (ImGui::BeginTabItem("Audio")) {
								ImGui::InputInt("Frequency", &project_settings.audio.frequency);
								ImGui::InputInt("Buffers", &project_settings.audio.buffers);

								ImGui::EndTabItem();
							}
						}

						if (project_settings.modules.audio_mixer) {
							if (ImGui::BeginTabItem("Mixer")) {
								ImGui::InputInt("Channels", &project_settings.audio_mixer.channels);

								ImGui::EndTabItem();
							}
						}

						static int antialias_current = project_settings.display.antialias;
						static int bit_depth_current = project_settings.display.bit_depth;
						static int gamma_current = project_settings.display.gamma;
						static int resolution_current = project_settings.display.resolution;

						const char *antialias_items[] = {"ANTIALIAS_OFF", "ANTIALIAS_RESAMPLE",
														 "ANTIALIAS_RESAMPLE_FETCH_NEEDED",
														 "ANTIALIAS_RESAMPLE_FETCH_ALWAYS"};
						const char *bit_depth_items[] = {"DEPTH_16_BPP", "DEPTH_32_BPP"};
						const char *gamma_items[] = {"GAMMA_NONE", "GAMMA_CORRECT",
													 "GAMMA_CORRECT_DITHER"};
						const char *resolution_items[] = {
							"RESOLUTION_320x240", "RESOLUTION_640x480", "RESOLUTION_256x240",
							"RESOLUTION_512x480", "RESOLUTION_512x240", "RESOLUTION_640x240"};

						if (project_settings.modules.display) {
							if (ImGui::BeginTabItem("Display")) {
								ImGui::Combo("Antialias", &antialias_current, antialias_items, 4);
								ImGui::Combo("Bit Depth", &bit_depth_current, bit_depth_items, 2);
								ImGui::SliderInt("Buffers",
												 &state.project_settings_screen.display_buffers, 1, 3);
								ImGui::Combo("Gamma", &gamma_current, gamma_items, 3);
								ImGui::Combo("Resolution", &resolution_current, resolution_items,
											 6);

								ImGui::EndTabItem();
							}
						}

						ImGui::Separator();
						ImGui::Spacing();
						if (ImGui::Button("Save")) {
							strcpy(state.project_settings_screen.display_antialias,
								   antialias_items[antialias_current]);
							strcpy(state.project_settings_screen.display_bit_depth,
								   bit_depth_items[bit_depth_current]);
							strcpy(state.project_settings_screen.display_gamma,
								   gamma_items[gamma_current]);
							strcpy(state.project_settings_screen.display_resolution,
								   resolution_items[resolution_current]);

							state.project_settings_screen.ToProjectSettings(project_settings);

							project_settings.SaveToDisk();

							SDL_SetWindowTitle(app.window,
											   ("NGine - " + project_settings.project_name + " - " +
												project_settings.project_directory)
												   .c_str());

							console.AddLog("Saved Project Settings.");
						}
					}
					ImGui::EndTabBar();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Engine")) {
				ImGui::TextUnformatted("Emulator Path (?)");
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip(
						"example: '/path/to/cen64'.\nWe will run 'path path/to/rom_file.z64'.");
				}
				ImGui::InputText("##EmuPath", state.emulator_path, 255);

				if (ImGui::Button("Save")) {
					engine_settings.SetEmulatorPath(state.emulator_path);
				}

				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
	ImGui::End();

	return true;
}

void reload_scripts() {
	script_files.clear();

	std::filesystem::path script_folder = project_settings.project_directory + "/.ngine/scripts";

	if (!std::filesystem::exists(script_folder)) {
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iter(script_folder);
	for (auto &file_entry : dir_iter) {
		if (file_entry.is_regular_file()) {
			std::string filepath(file_entry.path());
			if (filepath.ends_with(".script.json")) {
				nlohmann::json json;

				std::ifstream filestream(file_entry.path());
				filestream >> json;
				filestream.close();

				script_files.emplace_back(json["name"]);
			}
		}
	}
}

void render_image_import_windows() {
	if (!state.dropped_image_files.empty() || !state.dropped_sound_files.empty()) {
		int id = 1;
		if (ImGui::Begin("Import Assets")) {
			if (ImGui::BeginTabBar("ImportAssets")) {
				for (int i = 0; i < state.dropped_image_files.size(); ++i) {
					ImGui::PushID(id);
					if (ImGui::BeginTabItem("Image")) {
						ImGui::Image((ImTextureID)(intptr_t)state.dropped_image_files[i].image_data,
									 ImVec2(state.dropped_image_files[i].w, state.dropped_image_files[i].h));

						ImGui::Separator();
						ImGui::Spacing();

						ImGui::InputText("Name", state.dropped_image_files[i].name, 50);
						ImGui::InputText("DFS Folder", state.dropped_image_files[i].dfs_folder, 100);
						ImGui::InputInt("H Slices", &state.dropped_image_files[i].h_slices);
						ImGui::InputInt("V Slices", &state.dropped_image_files[i].v_slices);

						ImGui::Separator();
						ImGui::Spacing();
						if (ImGui::Button("Import")) {
							std::string name(state.dropped_image_files[i].name);
							std::string dfs_folder(state.dropped_image_files[i].dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								std::string name_string(name);
								auto find_by_name =
									[&name_string](std::unique_ptr<LibdragonImage> &i) {
										return i->name == name_string;
									};
								if (std::find_if(images.begin(), images.end(), find_by_name) !=
									std::end(images)) {
									console.AddLog(
										"Image with the name already exists. Please choose a "
										"different name.");
								} else {
									auto image = std::make_unique<LibdragonImage>();
									image->name = name;
									image->dfs_folder = dfs_folder;
									image->h_slices = state.dropped_image_files[i].h_slices;
									image->v_slices = state.dropped_image_files[i].v_slices;
									image->image_path = "assets/sprites/" + name + ".png";

									std::filesystem::create_directories(
										project_settings.project_directory + "/assets/sprites");
									std::filesystem::copy_file(state.dropped_image_files[i].image_path,
															   project_settings.project_directory +
																   "/assets/sprites/" + name +
																   ".png");

									image->SaveToDisk(project_settings.project_directory);
									load_image(image);

									state.dropped_image_files.erase(state.dropped_image_files.begin() + i);

									images.push_back(move(image));
									--i;
								}
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							state.dropped_image_files.erase(state.dropped_image_files.begin() + i);
							--i;
						}

						ImGui::EndTabItem();
					}
					ImGui::PopID();
					++id;
				}
				for (int i = 0; i < state.dropped_sound_files.size(); ++i) {
					ImGui::PushID(id);
					if (ImGui::BeginTabItem("WAV Sound")) {
						ImGui::InputText("Name", state.dropped_sound_files[i].name, 50);
						ImGui::InputText("DFS Folder", state.dropped_sound_files[i].dfs_folder, 100);
						ImGui::Checkbox("Loop", &state.dropped_sound_files[i].loop);
						if (state.dropped_sound_files[i].loop) {
							ImGui::InputInt("Loop Offset", &state.dropped_sound_files[i].loop_offset);
						}

						ImGui::Separator();
						ImGui::Spacing();
						if (ImGui::Button("Import")) {
							std::string name(state.dropped_sound_files[i].name);
							std::string dfs_folder(state.dropped_sound_files[i].dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								std::string name_string(name);
								auto find_by_name =
									[&name_string](std::unique_ptr<LibdragonSound> &i) {
										return i->name == name_string;
									};
								if (std::find_if(sounds.begin(), sounds.end(), find_by_name) !=
									std::end(sounds)) {
									console.AddLog(
										"Sound with the name already exists. Please choose a "
										"different name.");
								} else {
									auto image = std::make_unique<LibdragonSound>(SOUND_WAV);
									image->name = name;
									image->dfs_folder = dfs_folder;
									image->sound_path = "assets/sounds/" + name + ".wav";
									image->wav_loop = state.dropped_sound_files[i].loop;
									image->wav_loop_offset = state.dropped_sound_files[i].loop_offset;

									std::filesystem::create_directories(
										project_settings.project_directory + "/assets/sounds");
									std::filesystem::copy_file(state.dropped_sound_files[i].sound_path,
															   project_settings.project_directory +
																   "/assets/sounds/" + name +
																   ".wav");

									image->SaveToDisk(project_settings.project_directory);

									state.dropped_sound_files.erase(state.dropped_sound_files.begin() + i);

									sounds.push_back(move(image));
									--i;
								}
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							state.dropped_sound_files.erase(state.dropped_sound_files.begin() + i);
							--i;
						}

						ImGui::EndTabItem();
					}
					ImGui::PopID();
					++id;
				}
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}
}

void load_images() {
	images.clear();

	std::filesystem::path folder = project_settings.project_directory + "/.ngine/sprites";
	if (!std::filesystem::exists(folder)) {
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iter(folder);
	for (auto &file_entry : dir_iter) {
		if (file_entry.is_regular_file()) {
			std::string filepath(file_entry.path());
			if (filepath.ends_with(".sprite.json")) {
				auto image = std::make_unique<LibdragonImage>();
				image->LoadFromDisk(filepath);

				load_image(image);

				images.push_back(move(image));
			}
		}
	}
}

void load_image(std::unique_ptr<LibdragonImage> &image) {
	std::string path(project_settings.project_directory + "/" + image->image_path);

	image->loaded_image = IMG_LoadTexture(app.renderer, path.c_str());

	int w, h;
	SDL_QueryTexture(image->loaded_image, nullptr, nullptr, &w, &h);

	image->width = w;
	image->height = h;

	const float max_size = 100.f;
	if (w > h) {
		h = (h / (float)w) * max_size;
		w = max_size;
	} else {
		w = (w / (float)h) * max_size;
		h = max_size;
	}

	image->display_width = w;
	image->display_height = h;
}

void load_sounds() {
	sounds.clear();

	std::filesystem::path folder = project_settings.project_directory + "/.ngine/sounds";
	if (!std::filesystem::exists(folder)) {
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iter(folder);
	for (auto &file_entry : dir_iter) {
		if (file_entry.is_regular_file()) {
			std::string filepath(file_entry.path());
			if (filepath.ends_with(".sound.json")) {
				auto sound = std::make_unique<LibdragonSound>(SOUND_UNKNOWN);
				sound->LoadFromDisk(filepath);

				sounds.push_back(move(sound));
			}
		}
	}
}
