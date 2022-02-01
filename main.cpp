#include "imgui/imgui.h"
#include "imgui-SFML/imgui-SFML.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "console_app.h"

ConsoleApp console;

int main() {
	sf::RenderWindow window(sf::VideoMode(1024, 768), "NGine - N64 Engine Powered by Libdragon");
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
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Project")) {
					new_project_window_open = true;
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
			if (ImGui::Begin("New Project")) {
				char input[255];
				ImGui::TextUnformatted("Folder");
				ImGui::SameLine();
				ImGui::InputText("##", input, 255);
				if (ImGui::Button("Create Project", ImVec2(50, 20))) {
					char command[500];
					snprintf(command, 500, "cd %s\nlibdragon init", input);
					system(command);
					new_project_window_open = false;
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(50, 20))) {
					new_project_window_open = false;
				}
				ImGui::End();
			}
		}

		console.Draw("Console", window);

		window.clear();

		// render SFML stuff here

		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}