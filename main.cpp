#include <filesystem>
#include <iostream>

#include "imgui/imgui.h"
#include "imgui-SFML/imgui-SFML.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "ConsoleApp.h"
#include "ProjectBuilder.h"
#include "ProjectSettings.h"
#include "Libdragon.h"
#include "VSCode.h"

const char *default_title = "NGine - N64 Engine Powered by Libdragon";

ConsoleApp console;
ProjectSettings project_settings;

char input_new_project[255];
char input_open_project[255];

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

	console.Draw("Output", window);
}