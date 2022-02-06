#include "ImportAssets.h"

#include <filesystem>

#include "App.h"
#include "ConsoleApp.h"
#include "imgui.h"

void ImportAssets::RenderImportScreen(App *app) {
	if (!app->state.dropped_image_files.empty() || !app->state.dropped_sound_files.empty()) {
		int id = 1;
		if (ImGui::Begin("Import Assets")) {
			if (ImGui::BeginTabBar("ImportAssets")) {
				for (int i = 0; i < app->state.dropped_image_files.size(); ++i) {
					ImGui::PushID(id);
					if (ImGui::BeginTabItem("Image")) {
						ImGui::Image((ImTextureID)(intptr_t)app->state.dropped_image_files[i].image_data,
									 ImVec2(app->state.dropped_image_files[i].w, app->state.dropped_image_files[i].h));

						ImGui::Separator();
						ImGui::Spacing();

						ImGui::InputText("Name", app->state.dropped_image_files[i].name, 50);
						ImGui::InputText("DFS Folder", app->state.dropped_image_files[i].dfs_folder, 100);
						ImGui::InputInt("H Slices", &app->state.dropped_image_files[i].h_slices);
						ImGui::InputInt("V Slices", &app->state.dropped_image_files[i].v_slices);

						ImGui::Separator();
						ImGui::Spacing();
						if (ImGui::Button("Import")) {
							std::string name(app->state.dropped_image_files[i].name);
							std::string dfs_folder(app->state.dropped_image_files[i].dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								std::string name_string(name);
								auto find_by_name =
									[&name_string](std::unique_ptr<LibdragonImage> &i) {
										return i->name == name_string;
									};
								if (std::find_if(app->project.images.begin(), app->project.images.end(), find_by_name) !=
									std::end(app->project.images)) {
									console.AddLog(
										"Image with the name already exists. Please choose a "
										"different name.");
								} else {
									auto image = std::make_unique<LibdragonImage>();
									image->name = name;
									image->dfs_folder = dfs_folder;
									image->h_slices = app->state.dropped_image_files[i].h_slices;
									image->v_slices = app->state.dropped_image_files[i].v_slices;
									image->image_path = "assets/sprites/" + name + ".png";

									std::filesystem::create_directories(
										app->project.project_settings.project_directory + "/assets/sprites");
									std::filesystem::copy_file(app->state.dropped_image_files[i].image_path,
															   app->project.project_settings.project_directory +
																   "/assets/sprites/" + name +
																   ".png");

									image->SaveToDisk(app->project.project_settings.project_directory);
									image->LoadImage(app->project.project_settings.project_directory, app->renderer);

									app->state.dropped_image_files.erase(app->state.dropped_image_files.begin() + i);

									app->project.images.push_back(move(image));
									--i;
								}
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							app->state.dropped_image_files.erase(app->state.dropped_image_files.begin() + i);
							--i;
						}

						ImGui::EndTabItem();
					}
					ImGui::PopID();
					++id;
				}
				for (int i = 0; i < app->state.dropped_sound_files.size(); ++i) {
					ImGui::PushID(id);
					if (ImGui::BeginTabItem("WAV Sound")) {
						ImGui::InputText("Name", app->state.dropped_sound_files[i].name, 50);
						ImGui::InputText("DFS Folder", app->state.dropped_sound_files[i].dfs_folder, 100);
						ImGui::Checkbox("Loop", &app->state.dropped_sound_files[i].loop);
						if (app->state.dropped_sound_files[i].loop) {
							ImGui::InputInt("Loop Offset", &app->state.dropped_sound_files[i].loop_offset);
						}

						ImGui::Separator();
						ImGui::Spacing();
						if (ImGui::Button("Import")) {
							std::string name(app->state.dropped_sound_files[i].name);
							std::string dfs_folder(app->state.dropped_sound_files[i].dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								std::string name_string(name);
								auto find_by_name =
									[&name_string](std::unique_ptr<LibdragonSound> &i) {
										return i->name == name_string;
									};
								if (std::find_if(app->project.sounds.begin(), app->project.sounds.end(), find_by_name) !=
									std::end(app->project.sounds)) {
									console.AddLog(
										"Sound with the name already exists. Please choose a "
										"different name.");
								} else {
									auto image = std::make_unique<LibdragonSound>(SOUND_WAV);
									image->name = name;
									image->dfs_folder = dfs_folder;
									image->sound_path = "assets/sounds/" + name + ".wav";
									image->wav_loop = app->state.dropped_sound_files[i].loop;
									image->wav_loop_offset = app->state.dropped_sound_files[i].loop_offset;

									std::filesystem::create_directories(
										app->project.project_settings.project_directory + "/assets/sounds");
									std::filesystem::copy_file(app->state.dropped_sound_files[i].sound_path,
															   app->project.project_settings.project_directory +
																   "/assets/sounds/" + name +
																   ".wav");

									image->SaveToDisk(app->project.project_settings.project_directory);

									app->state.dropped_sound_files.erase(app->state.dropped_sound_files.begin() + i);

									app->project.sounds.push_back(move(image));
									--i;
								}
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							app->state.dropped_sound_files.erase(app->state.dropped_sound_files.begin() + i);
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
