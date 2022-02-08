#include "ImportAssets.h"

#include <filesystem>

#include "App.h"
#include "ConsoleApp.h"
#include "imgui.h"

void ImportAssets::RenderImportScreen(App *app) {
	if (!app->state.dropped_image_files.empty() || !app->state.dropped_sound_files.empty()) {
		if (ImGui::Begin("Import Assets")) {
			float window_width = ImGui::GetWindowWidth();
			float window_height = ImGui::GetWindowHeight() - 200;
			if (ImGui::BeginTabBar("ImportAssets")) {
				int id = 1;
				for (size_t i = 0; i < app->state.dropped_image_files.size(); ++i) {
					DroppedImage *image_file = &app->state.dropped_image_files[i];

					ImGui::PushID(id);
					if (ImGui::BeginTabItem("Image")) {
						if (image_file->width_mult > image_file->height_mult) {
							ImGui::Image((ImTextureID)(intptr_t)image_file->image_data,
										 ImVec2((float)window_width,
												(float)image_file->height_mult * window_width));
						} else {
							ImGui::Image((ImTextureID)(intptr_t)image_file->image_data,
										 ImVec2((float)image_file->width_mult * window_height,
												(float)window_height));
						}
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::InputText("Name", image_file->name, 50);
						ImGui::InputText("DFS Folder", image_file->dfs_folder, 100);
						ImGui::InputInt("H Slices", &image_file->h_slices);
						ImGui::InputInt("V Slices", &image_file->v_slices);

						ImGui::Separator();
						ImGui::Spacing();
						if (ImGui::Button("Import")) {
							std::string name(image_file->name);
							std::string dfs_folder(image_file->dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								std::string name_string(name);
								auto find_by_name =
									[&name_string](const std::unique_ptr<LibdragonImage> &i) {
										return i->name == name_string;
									};
								if (std::find_if(app->project.images.begin(),
												 app->project.images.end(),
												 find_by_name) != std::end(app->project.images)) {
									console.AddLog(
										"Image with the name already exists. Please choose a "
										"different name.");
								} else {
									auto image = std::make_unique<LibdragonImage>();
									image->name = name;
									image->dfs_folder = dfs_folder;
									image->h_slices = image_file->h_slices;
									image->v_slices = image_file->v_slices;
									image->image_path = "assets/sprites/" + name + ".png";

									std::filesystem::create_directories(
										app->project.project_settings.project_directory +
										"/assets/sprites");
									std::filesystem::copy_file(
										image_file->image_path,
										app->project.project_settings.project_directory +
											"/assets/sprites/" + name + ".png");

									image->SaveToDisk(
										app->project.project_settings.project_directory);
									image->LoadImage(
										app->project.project_settings.project_directory,
										app->renderer);

									app->state.dropped_image_files.erase(
										app->state.dropped_image_files.begin() + (int)i);

									app->project.images.push_back(move(image));
									--i;
								}
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							app->state.dropped_image_files.erase(
								app->state.dropped_image_files.begin() + (int)i);
							--i;
						}

						ImGui::EndTabItem();
					}
					ImGui::PopID();
					++id;
				}
				for (size_t i = 0; i < app->state.dropped_sound_files.size(); ++i) {
					DroppedSound *sound_file = &app->state.dropped_sound_files[i];
					ImGui::PushID(id);
					if (ImGui::BeginTabItem("Sound")) {
						ImGui::InputText("Name", sound_file->name, 50);
						ImGui::InputText("DFS Folder", sound_file->dfs_folder, 100);

						if (sound_file->type == SOUND_WAV) {
							ImGui::Checkbox("Loop", &sound_file->loop);
							if (sound_file->loop) {
								ImGui::InputInt("Loop Offset", &sound_file->loop_offset);
							}
						} else if (sound_file->type == SOUND_YM) {
							ImGui::Checkbox("Compress", &sound_file->compress);
						}

						ImGui::Separator();
						ImGui::Spacing();
						if (ImGui::Button("Import")) {
							std::string name(sound_file->name);
							std::string dfs_folder(sound_file->dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								std::string name_string(name);
								auto find_by_name =
									[&name_string](const std::unique_ptr<LibdragonSound> &i) {
										return i->name == name_string;
									};
								if (std::find_if(app->project.sounds.begin(),
												 app->project.sounds.end(),
												 find_by_name) != std::end(app->project.sounds)) {
									console.AddLog(
										"Sound with the name already exists. Please choose a "
										"different name.");
								} else {
									auto sound = std::make_unique<LibdragonSound>(sound_file->type);
									sound->name = name;
									sound->dfs_folder = dfs_folder;
									sound->sound_path = "assets/sounds/" + name + sound->GetExtension();
									sound->wav_loop = sound_file->loop;
									sound->wav_loop_offset = sound_file->loop_offset;
									sound->ym_compress = sound_file->compress;

									std::filesystem::create_directories(
										app->project.project_settings.project_directory +
										"/assets/sounds");
									std::filesystem::copy_file(
										sound_file->sound_path,
										app->project.project_settings.project_directory +
											"/assets/sounds/" + name + sound->GetExtension());

									sound->SaveToDisk(
										app->project.project_settings.project_directory);

									app->state.dropped_sound_files.erase(
										app->state.dropped_sound_files.begin() + (int)i);

									app->project.sounds.push_back(move(sound));
									--i;
								}
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							app->state.dropped_sound_files.erase(
								app->state.dropped_sound_files.begin() + (int)i);
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
