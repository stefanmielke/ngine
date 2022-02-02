#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include "imgui/imgui.h"
#include "imgui-SFML/imgui-SFML.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "ConsoleApp.h"
#include "generated/generated.h"
#include "ProjectSettings.h"

const char *default_title = "NGine - N64 Engine Powered by Libdragon";

ConsoleApp console;
ProjectSettings project_settings;

int main() {
	std::string project_folder;

	std::string temp_output_string;
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

		ImGui::SFML::Update(window, deltaClock.restart());

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
				if (ImGui::MenuItem("Close Project")) {
					console.AddLog("Closing project...");

					project_folder.clear();
					window.setTitle(default_title);

					console.AddLog("Project closed.");
				}
				if (ImGui::MenuItem("Exit")) {
					window.close();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (new_project_window_open) {
			ImGui::SetNextWindowSize(ImVec2(300, 80));
			if (ImGui::Begin("New Project", &new_project_window_open)) {
				char input[255];
				ImGui::TextUnformatted("Folder");
				ImGui::SameLine();
				ImGui::InputText("##", input, 255);
				if (ImGui::Button("Create", ImVec2(50, 20))) {
					std::string new_project_folder(input);

					// create folder if it doesn't exist
					const std::filesystem::path project_path(new_project_folder);
					if (!std::filesystem::exists(project_path)) {
						console.AddLog("Creating project folder...");
						std::filesystem::create_directories(project_path);
					}

					// create initial project file
					console.AddLog("Creating project settings file...");
					std::string project_settings_filename(new_project_folder +
														  "/ngine.project.json");

					ProjectSettings default_project_settings;
					default_project_settings.SaveToFile(project_settings_filename);

					console.AddLog("Running 'libdragon init' at '%s'...", input);
					console.AddLog("Check output on the console.");
					char command[500];
					snprintf(command, 500, "cd %s\nlibdragon init", new_project_folder.c_str());
					std::thread(system, command).detach();

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
				char input[255];
				ImGui::TextUnformatted("Folder");
				ImGui::SameLine();
				ImGui::InputText("##", input, 255);
				if (ImGui::Button("Open", ImVec2(50, 20))) {
					console.AddLog("Opening project at '%s'...", input);

					project_folder = input;
					window.setTitle("NGine - " + project_folder);

					std::string project_filepath = project_folder + "/ngine.project.json";
					project_settings.LoadFromFile(project_filepath);

					open_project_window_open = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(50, 20))) {
					open_project_window_open = false;
				}
				ImGui::End();
			}
		}

		// pulling data from output stream if available
		while (std::getline(output_stream, temp_output_string, '\n')) {
			console.AddLog("%s", temp_output_string.c_str());
		}

		console.Draw("Output", window);

		window.clear();

		// render SFML stuff here

		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}