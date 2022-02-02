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
#include "ProjectSettings.h"
#include "VSCode.h"

const char *default_title = "NGine - N64 Engine Powered by Libdragon";

ConsoleApp console;
ProjectSettings project_settings;

struct ProjectSettingsScreen {
	char project_name[100];
	char rom_name[100];

	char display_antialias[100];
	char display_bit_depth[100];
	int display_buffers;
	char display_gamma[100];
	char display_resolution[100];
};

char input_new_project[255];
char input_open_project[255];
ProjectSettingsScreen project_settings_screen;

void update_gui(sf::RenderWindow &window, sf::Time time);

int main() {
	memset(input_new_project, 0, 255);
	memset(input_open_project, 0, 255);

	std::stringstream output_stream;
	std::cout.rdbuf(output_stream.rdbuf());

	sf::RenderWindow window(sf::VideoMode(1024, 768), default_title);
	window.setFramerateLimit(60);
	if (!ImGui::SFML::Init(window))
		return -1;

	sf::Clock deltaClock;
	while (window.isOpen()) {
		sf::Event event;
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
		if (ImGui::MenuItem("Build", nullptr, false, project_settings.IsOpen())) {
			console.AddLog("Building project...");

			ProjectBuilder::Build(project_settings);
		}
		if (ImGui::MenuItem("Clean/Build", nullptr, false, project_settings.IsOpen())) {
			console.AddLog("Rebuilding project...");

			ProjectBuilder::Rebuild(project_settings);
		}
		if (ImGui::MenuItem("Open in VSCode", nullptr, false, project_settings.IsOpen())) {
			console.AddLog("Opening project in VSCode...");

			VSCode::OpenFolder(project_settings.project_directory);
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
					window.setTitle("NGine - " + project_settings.project_directory);
					open_project_window_open = false;

					strcpy(project_settings_screen.project_name,
						   project_settings.project_name.c_str());
					strcpy(project_settings_screen.rom_name, project_settings.rom_name.c_str());
					strcpy(project_settings_screen.display_antialias,
						   project_settings.display.GetAntialias());
					strcpy(project_settings_screen.display_bit_depth,
						   project_settings.display.GetBitDepth());
					strcpy(project_settings_screen.display_gamma,
						   project_settings.display.GetGamma());
					strcpy(project_settings_screen.display_resolution,
						   project_settings.display.GetResolution());
					project_settings_screen.display_buffers = project_settings.display.buffers;

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

	if (project_settings.IsOpen()) {
		const int prop_x_size = 300;
		const int prop_y_size = is_output_open ? 219 : 19;
		ImGui::SetNextWindowSize(ImVec2(prop_x_size, window.getSize().y - prop_y_size));
		ImGui::SetNextWindowPos(ImVec2(window.getSize().x - prop_x_size, 19));
		if (ImGui::Begin("Properties", nullptr,
						 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
			ImGui::TextUnformatted("Project Settings:");
			ImGui::InputText("Name", project_settings_screen.project_name, 100);
			ImGui::InputText("Rom", project_settings_screen.rom_name, 100);

			ImGui::Separator();
			ImGui::TextUnformatted("Display Settings:");

			const char *antialias_items[] = { "ANTIALIAS_OFF", "ANTIALIAS_RESAMPLE", "ANTIALIAS_RESAMPLE_FETCH_NEEDED", "ANTIALIAS_RESAMPLE_FETCH_ALWAYS" };
			static int antialias_current;
			ImGui::Combo("Antialias", &antialias_current, antialias_items, 4);

			const char *bit_depth_items[] = { "DEPTH_16_BPP", "DEPTH_32_BPP" };
			static int bit_depth_current;
			ImGui::Combo("Bit Depth", &bit_depth_current, bit_depth_items, 2);

			ImGui::SliderInt("Buffers", &project_settings_screen.display_buffers, 1, 3);

			const char *gamma_items[] = { "GAMMA_NONE", "GAMMA_CORRECT", "GAMMA_CORRECT_DITHER" };
			static int gamma_current;
			ImGui::Combo("Gamma", &gamma_current, gamma_items, 3);

			const char *resolution_items[] = { "RESOLUTION_320x240", "RESOLUTION_640x480", "RESOLUTION_256x240", "RESOLUTION_512x480", "RESOLUTION_512x240", "RESOLUTION_640x240" };
			static int resolution_current;
			ImGui::Combo("Resolution", &resolution_current, resolution_items, 6);

			ImGui::Separator();
			if (ImGui::Button("Save")) {
				project_settings.project_name = project_settings_screen.project_name;
				project_settings.rom_name = project_settings_screen.rom_name;

				project_settings.display.buffers = project_settings_screen.display_buffers;
				project_settings.display.SetAntialias(antialias_items[antialias_current]);
				project_settings.display.SetBitDepth(bit_depth_items[bit_depth_current]);
				project_settings.display.SetGamma(gamma_items[gamma_current]);
				project_settings.display.SetResolution(resolution_items[resolution_current]);

				std::string project_filepath = project_settings.project_directory + "/ngine.project.json";
				project_settings.SaveToFile(project_filepath);
			}
		}
		ImGui::End();
	}
}