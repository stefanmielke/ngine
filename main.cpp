#include <filesystem>
#include <fstream>
#include <iostream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"

#include <SDL2/SDL.h>

#include "ConsoleApp.h"
#include "Emulator.h"
#include "json.hpp"
#include "LibdragonImage.h"
#include "ProjectBuilder.h"
#include "ScriptBuilder.h"
#include "VSCode.h"
#include "settings/EngineSettings.h"
#include "settings/Project.h"
#include "settings/ProjectSettings.h"
#include "settings/ProjectSettingsScreen.h"

const char *default_title = "NGine - N64 Engine Powered by Libdragon";

ConsoleApp console;
Project project;
Scene *current_scene = nullptr;
char scene_name[100];
std::vector<std::string> script_files;
std::vector<LibdragonImage> images;

ProjectSettings project_settings;
EngineSettings engine_settings;

char input_new_project[255];
char input_open_project[255];
char emulator_path[255];
ProjectSettingsScreen project_settings_screen;

bool update_gui(SDL_Window *window);
void reload_scripts();

struct App {
	SDL_Renderer *renderer;
	SDL_Window *window;
} app;

void initSDL(void) {
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	windowFlags = SDL_WINDOW_RESIZABLE;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

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
	memset(input_new_project, 0, 255);
	memset(input_open_project, 0, 255);

	engine_settings.LoadFromDisk();
	strcpy(input_open_project, engine_settings.GetLastOpenedProject().c_str());
	strcpy(emulator_path, engine_settings.GetMupen64Path().c_str());

	std::stringstream output_stream;
	std::cout.rdbuf(output_stream.rdbuf());

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
				default:
					break;
			}
		}

		if (!is_running)
			break;

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame(app.window);
		ImGui::NewFrame();

		is_running = update_gui(app.window);

		// pulling data from output stream if available
		std::string temp_output_string;
		while (std::getline(output_stream, temp_output_string, '\n')) {
			console.AddLog("%s", temp_output_string.c_str());
		}

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
	SDL_Quit();

	return 0;
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

			ProjectBuilder::Build(project_settings, project);
		}
		if (ImGui::BeginMenu("Tasks", project_settings.IsOpen())) {
			if (ImGui::MenuItem("Clean/Build")) {
				console.AddLog("Rebuilding project...");

				ProjectBuilder::Rebuild(project_settings, project);
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
				project_settings.IsOpen() && !engine_settings.GetMupen64Path().empty())) {
			Emulator::Run(engine_settings, project_settings, project);
		}
		ImGui::EndMainMenuBar();
	}

	if (new_project_window_open) {
		ImGui::SetNextWindowSize(ImVec2(300, 80));
		if (ImGui::Begin("New Project", &new_project_window_open)) {
			ImGui::TextUnformatted("Folder");
			ImGui::SameLine();
			ImGui::InputText("##", input_new_project, 255);
			if (ImGui::Button("Create", ImVec2(50, 20))) {
				std::string new_project_folder(input_new_project);

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
			ImGui::InputText("##", input_open_project, 255);
			if (ImGui::Button("Open", ImVec2(50, 20))) {
				console.AddLog("Opening project at '%s'...", input_open_project);

				if (project_settings.IsOpen()) {
					project_settings.CloseProject();
				}

				project_settings.project_directory = input_open_project;

				std::string project_filepath = std::string(input_open_project);
				if (project_settings.LoadFromFile(project_filepath)) {
					project.LoadFromDisk(project_settings.project_directory);

					SDL_SetWindowTitle(window, ("NGine - " + project_settings.project_name + " - " +
												project_settings.project_directory)
												   .c_str());
					open_project_window_open = false;

					project_settings_screen.FromProjectSettings(project_settings);

					engine_settings.SetLastOpenedProject(project_filepath);

					reload_scripts();

					console.AddLog("Project opened.");
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(50, 20))) {
				open_project_window_open = false;
			}
			ImGui::End();
		}
	}

	console.Draw("Output", window, is_output_open);

	const float center_x_size = (float)window_width - 600;
	const float center_y_offset = is_output_open ? 219 : 38;
	ImGui::SetNextWindowSize(ImVec2(center_x_size, (float)window_height - center_y_offset));
	ImGui::SetNextWindowPos(ImVec2(300, 19));
	if (ImGui::Begin("ContentBrowser", nullptr,
					 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
		if (ImGui::BeginTabBar("CenterContentTabs",
							   ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
			if (ImGui::BeginTabItem("Content Browser")) {
				if (project_settings.IsOpen()) {
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Script Browser")) {
				if (project_settings.IsOpen()) {
					static char script_name_input[100] = {};
					ImGui::InputText("Script Name", script_name_input, 100);
					ImGui::SameLine();
					if (ImGui::Button("Create Script File")) {
						ScriptBuilder::CreateScriptFile(project_settings, script_name_input);
						memset(script_name_input, 0, 100);

						reload_scripts();
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
		if (current_scene) {
			if (ImGui::BeginTabBar("Properties", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
				if (ImGui::BeginTabItem("Nodes")) {
					if (ImGui::TreeNodeEx("Root Node")) {
						{
							ImGui::SameLine();
							if (ImGui::Selectable("Props")) {
								// do whatever
							}
						}
						{
							if (ImGui::Selectable("Test Node Press")) {
								// do whatever
							}
						}

						if (ImGui::TreeNode("Test Node")) {
							ImGui::TreePop();
						}
						if (ImGui::TreeNode("Test Node 2")) {
							ImGui::TreePop();
						}
						ImGui::TreePop();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Settings")) {
					ImGui::TextColored(ImColor(100, 100, 255), "Id: %d", current_scene->id);
					ImGui::Spacing();
					ImGui::Separator();

					ImGui::Spacing();
					ImGui::InputText("Name", scene_name, 100);
					ImGui::Spacing();

					ImGui::Separator();
					ImGui::Spacing();
					{
						std::string current_selected(current_scene->script_name);
						ImGui::TextUnformatted("Attached Script");
						if (ImGui::BeginCombo("##AttachedScript", current_selected.c_str())) {
							for (auto &script : script_files) {
								if (ImGui::Selectable(script.c_str(), script == current_selected)) {
									current_scene->script_name = script;
								}
							}
							ImGui::EndCombo();
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Remove")) {
							current_scene->script_name.clear();
						}
					}
					ImGui::Spacing();

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Save")) {
						current_scene->name = scene_name;
						project.SaveToDisk(project_settings.project_directory);
						project_settings.SaveToDisk();
					}
					ImGui::Spacing();

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Delete Scene")) {
						for (int i = 0; i < project.scenes.size(); ++i) {
							if (project.scenes[i].id == current_scene->id) {
								project.scenes.erase(project.scenes.begin() + i);
								std::string filename = project_settings.project_directory +
													   "/.ngine/scenes/" +
													   std::to_string(current_scene->id) +
													   ".scene.json";
								std::filesystem::remove(filename);

								current_scene = nullptr;
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
		if (ImGui::BeginTabBar("Properties")) {
			if (ImGui::BeginTabItem("Scenes")) {
				for (auto &scene : project.scenes) {
					if (ImGui::Selectable(scene.name.c_str())) {
						current_scene = &scene;
						strcpy(scene_name, current_scene->name.c_str());
					}
				}

				if (ImGui::Button("Create New Scene")) {
					project.scenes.emplace_back();
					current_scene = &project.scenes[project.scenes.size() - 1];
					current_scene->id = project_settings.next_scene_id++;
					current_scene->name = std::to_string(project.scenes.size());
					strcpy(scene_name, current_scene->name.c_str());
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Project")) {
				if (project_settings.IsOpen()) {
					ImGui::InputText("Name", project_settings_screen.project_name, 100);
					ImGui::InputText("Rom", project_settings_screen.rom_name, 100);

					ImGui::Separator();

					std::string current_selected("None");
					for (auto &scene : project.scenes) {
						if (scene.id == project_settings.initial_screen_id) {
							current_selected = scene.name;
							break;
						}
					}
					ImGui::TextUnformatted("Initial Screen");
					if (ImGui::BeginCombo("##InitialScreen", current_selected.c_str())) {
						for (auto &scene : project.scenes) {
							if (ImGui::Selectable(scene.name.c_str(),
												  scene.id == project_settings.initial_screen_id)) {
								project_settings.initial_screen_id = scene.id;
							}
						}
						ImGui::EndCombo();
					}

					ImGui::Separator();

					ImGui::TextUnformatted("Global Memory Reserve (KB)");
					ImGui::InputInt("##GlobalMem", &project_settings.global_mem_alloc_size, 1,
									1024);
					ImGui::TextUnformatted("Scene Memory Reserve (KB)");
					ImGui::InputInt("##LocalMem", &project_settings.scene_mem_alloc_size, 1, 1024);

					ImGui::Separator();

					ImGui::TextUnformatted("Modules:");
					ImGui::Checkbox("Console", &project_settings.modules.console);
					ImGui::Checkbox("Controller", &project_settings.modules.controller);
					ImGui::Checkbox("Debug Is Viewer", &project_settings.modules.debug_is_viewer);
					ImGui::Checkbox("Debug USB", &project_settings.modules.debug_usb);
					ImGui::Checkbox("Display", &project_settings.modules.display);
					ImGui::Checkbox("DFS", &project_settings.modules.dfs);
					ImGui::Checkbox("RDP", &project_settings.modules.rdp);
					ImGui::Checkbox("Timer", &project_settings.modules.timer);

					ImGui::Separator();

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
					const char *resolution_items[] = {"RESOLUTION_320x240", "RESOLUTION_640x480",
													  "RESOLUTION_256x240", "RESOLUTION_512x480",
													  "RESOLUTION_512x240", "RESOLUTION_640x240"};

					if (project_settings.modules.display) {
						ImGui::TextUnformatted("Display Settings:");
						ImGui::Combo("Antialias", &antialias_current, antialias_items, 4);
						ImGui::Combo("Bit Depth", &bit_depth_current, bit_depth_items, 2);
						ImGui::SliderInt("Buffers", &project_settings_screen.display_buffers, 1, 3);
						ImGui::Combo("Gamma", &gamma_current, gamma_items, 3);
						ImGui::Combo("Resolution", &resolution_current, resolution_items, 6);

						ImGui::Separator();
					}

					if (ImGui::Button("Save")) {
						strcpy(project_settings_screen.display_antialias,
							   antialias_items[antialias_current]);
						strcpy(project_settings_screen.display_bit_depth,
							   bit_depth_items[bit_depth_current]);
						strcpy(project_settings_screen.display_gamma, gamma_items[gamma_current]);
						strcpy(project_settings_screen.display_resolution,
							   resolution_items[resolution_current]);

						project_settings_screen.ToProjectSettings(project_settings);

						project_settings.SaveToDisk();

						console.AddLog("Saved Project Settings.");
					}
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Engine")) {
				ImGui::TextUnformatted("Emulator Path (?)");
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip(
						"example: '/path/to/cen64'.\nWe will run 'path path/to/rom_file.z64'.");
				}
				ImGui::InputText("##EmuPath", emulator_path, 255);

				if (ImGui::Button("Save")) {
					engine_settings.SetEmulatorPath(emulator_path);
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