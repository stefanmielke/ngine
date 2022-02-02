#include <filesystem>
#include <iostream>

#include "imgui/imgui.h"
#include "imgui-SFML/imgui-SFML.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "ConsoleApp.h"
#include "Libdragon.h"
#include "ProjectBuilder.h"
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

ProjectSettings project_settings;
EngineSettings engine_settings;

char input_new_project[255];
char input_open_project[255];
char mupen64_path[255];
ProjectSettingsScreen project_settings_screen;

void update_gui(sf::RenderWindow &window, sf::Time time);

int main() {
	memset(input_new_project, 0, 255);
	memset(input_open_project, 0, 255);

	engine_settings.LoadFromDisk();
	strcpy(input_open_project, engine_settings.GetLastOpenedProject().c_str());
	strcpy(mupen64_path, engine_settings.GetMupen64Path().c_str());

	std::stringstream output_stream;
	std::cout.rdbuf(output_stream.rdbuf());

	sf::RenderWindow window(sf::VideoMode(1024, 768), default_title);
	window.setFramerateLimit(60);
	if (!ImGui::SFML::Init(window))
		return -1;

	sf::Clock deltaClock;
	sf::Event event{};
	while (window.isOpen()) {
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(window, event);

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		update_gui(window, deltaClock.restart());

		// pulling data from output stream if available
		std::string temp_output_string;
		while (std::getline(output_stream, temp_output_string, '\n')) {
			console.AddLog("%s", temp_output_string.c_str());
		}

		window.clear();

		// render SFML stuff here

		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}

void update_gui(sf::RenderWindow &window, sf::Time time) {
	ImGui::SFML::Update(window, time);

	bool is_output_open = true;

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
				window.setTitle(default_title);

				console.AddLog("Project closed.");
			}
			if (ImGui::MenuItem("Exit")) {
				window.close();
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

			VSCode::OpenFolder(project_settings.project_directory);
		}
		if (ImGui::MenuItem(
				"Run in Mupen64", nullptr, false,
				project_settings.IsOpen() && !engine_settings.GetMupen64Path().empty())) {
			console.AddLog("Opening rom in Mupen64...");

			char cmd[255];
			snprintf(cmd, 255, "cd %s\n%s *.z64", project_settings.project_directory.c_str(),
					 engine_settings.GetMupen64Path().c_str());
			system(cmd);
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

					window.setTitle("NGine - " + project_settings.project_directory);
					open_project_window_open = false;

					project_settings_screen.FromProjectSettings(project_settings);

					engine_settings.SetLastOpenedProject(project_filepath);

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

	const int prop_y_size = is_output_open ? 219 : 38;
	ImGui::SetNextWindowSize(ImVec2(300, window.getSize().y - prop_y_size));
	ImGui::SetNextWindowPos(ImVec2(0, 19));
	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		if (current_scene) {
			if (ImGui::BeginTabBar("Properties")) {
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
					ImGui::InputText("Name", scene_name, 100);

					ImGui::Separator();

					if (ImGui::Button("Save")) {
						current_scene->name = scene_name;
						project.SaveToDisk(project_settings.project_directory);
						project_settings.SaveToDisk();
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

	const int prop_x_size = 300;
	ImGui::SetNextWindowSize(ImVec2(prop_x_size, window.getSize().y - prop_y_size));
	ImGui::SetNextWindowPos(ImVec2(window.getSize().x - prop_x_size, 19));
	if (ImGui::Begin("General Settings", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_NoTitleBar)) {
		if (ImGui::BeginTabBar("Properties")) {
			if (ImGui::BeginTabItem("Node")) {
				ImGui::EndTabItem();
			}
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
				ImGui::InputText("Mupen64 Path", mupen64_path, 255);

				if (ImGui::Button("Save")) {
					engine_settings.SetMupen64Path(mupen64_path);
				}

				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}