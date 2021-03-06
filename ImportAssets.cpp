#include "ImportAssets.h"

#include <filesystem>

#include "App.h"
#include "ConsoleApp.h"
#include "imgui/imgui.h"
#include "imgui/imgui_custom.h"

void ImportAssets::RenderImportScreen(App *app) {
	if (!app->state.dropped_image_files.empty() || !app->state.dropped_sound_files.empty() ||
		!app->state.dropped_general_files.empty() || !app->state.dropped_font_files.empty() ||
		!app->state.dropped_tiled_files.empty() || !app->state.dropped_ldtk_files.empty()) {
		ImGui::SetNextWindowFocus();
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
							float width = window_width - 30;
							ImGui::Image((ImTextureID)(intptr_t)image_file->image_data,
										 ImVec2(width, (float)image_file->height_mult * width));
							ImGui::SamePlace(8);
							ImGui::Image((ImTextureID)(intptr_t)image_file->image_data_overlay,
										 ImVec2(width, (float)image_file->height_mult * width));
						} else {
							ImGui::Image((ImTextureID)(intptr_t)image_file->image_data,
										 ImVec2((float)image_file->width_mult * window_height,
												(float)window_height));
							ImGui::SamePlace(8);
							ImGui::Image((ImTextureID)(intptr_t)image_file->image_data_overlay,
										 ImVec2((float)image_file->width_mult * window_height,
												(float)window_height));
						}
						ImGui::Separator();
						ImGui::Spacing();

						render_badge(get_libdragon_image_type_name(image_file->type).c_str(),
									 ImVec4(.4f, .8f, .4f, 0.7f));
						ImGui::InputText("Name", image_file->name, 50,
										 ImGuiInputTextFlags_CharsFileName);
						bool dfs_valid = input_text_dfs_folder(image_file->dfs_folder, 100);

						bool recreate_grid = false;
						if (ImGui::InputInt("H Slices", &image_file->h_slices)) {
							recreate_grid = true;
						}
						if (ImGui::InputInt("V Slices", &image_file->v_slices)) {
							recreate_grid = true;
						}
						if (recreate_grid) {
							if (SDL_SetRenderTarget(app->renderer, image_file->image_data_overlay) <
								0) {
								console.AddLog("Error setting render target: %s", SDL_GetError());
							}

							SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 0);
							SDL_RenderClear(app->renderer);

							SDL_SetRenderDrawColor(app->renderer, 0, 255, 0, 255);

							int h_slices = image_file->h_slices - 1;
							if (h_slices > 0) {
								float steps = ((float)image_file->w * 2) /
											  (float)image_file->h_slices;
								for (int h = 1; h <= h_slices; ++h) {
									SDL_RenderDrawLine(app->renderer, steps * h, 0, steps * h,
													   (image_file->h * 2));
								}
							}
							int v_slices = image_file->v_slices - 1;
							if (v_slices > 0) {
								float steps = ((float)image_file->h * 2) /
											  (float)image_file->v_slices;
								for (int v = 1; v <= v_slices; ++v) {
									SDL_RenderDrawLine(app->renderer, 0, steps * v,
													   (image_file->w * 2), steps * v);
								}
							}

							SDL_Rect rect = {0, 0, image_file->w * 2, image_file->h * 2};
							SDL_RenderDrawRect(app->renderer, &rect);

							SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);

							SDL_SetRenderTarget(app->renderer, nullptr);
						}

						ImGui::Separator();
						ImGui::Spacing();

						ImGui::BeginDisabled(!dfs_valid);
						if (ImGui::Button("Import")) {
							std::string name(image_file->name);
							std::string dfs_folder(image_file->dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								std::string extension = get_libdragon_image_type_extension(
									image_file->type);

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
									image->type = image_file->type;
									image->dfs_folder = dfs_folder;
									image->h_slices = image_file->h_slices;
									image->v_slices = image_file->v_slices;
									image->image_path = "assets/sprites/" + name + extension;

									std::filesystem::create_directories(
										app->project.project_settings.project_directory +
										"/assets/sprites");
									std::filesystem::copy_file(
										image_file->image_path,
										app->project.project_settings.project_directory +
											"/assets/sprites/" + name + extension);

									image->SaveToDisk(
										app->project.project_settings.project_directory);
									image->LoadImage(
										app->project.project_settings.project_directory,
										app->renderer);

									SDL_DestroyTexture(image_file->image_data);
									SDL_DestroyTexture(image_file->image_data_overlay);

									app->state.dropped_image_files.erase(
										app->state.dropped_image_files.begin() + (int)i);

									app->project.images.push_back(move(image));
									app->project.ReloadAssets();
									--i;
								}
							}
						}
						ImGui::EndDisabled();

						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							SDL_DestroyTexture(image_file->image_data);
							SDL_DestroyTexture(image_file->image_data_overlay);

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
						ImGui::InputText("Name", sound_file->name, 50,
										 ImGuiInputTextFlags_CharsFileName);
						bool dfs_valid = input_text_dfs_folder(sound_file->dfs_folder, 100);

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

						ImGui::BeginDisabled(!dfs_valid);
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
									sound->sound_path = "assets/sounds/" + name +
														sound->GetExtension();
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
									app->project.ReloadAssets();
									--i;
								}
							}
						}
						ImGui::EndDisabled();

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
				for (size_t i = 0; i < app->state.dropped_general_files.size(); ++i) {
					DroppedGeneralFile *general_file = &app->state.dropped_general_files[i];
					ImGui::PushID(id);
					if (ImGui::BeginTabItem("General")) {
						ImGui::BeginDisabled();
						ImGui::InputText("Type", general_file->extension, 50);
						ImGui::EndDisabled();

						ImGui::InputText("Name", general_file->name, 50,
										 ImGuiInputTextFlags_CharsFileName);
						bool dfs_valid = input_text_dfs_folder(general_file->dfs_folder, 100);
						ImGui::Checkbox("Copy to Filesystem", &general_file->copy_to_filesystem);

						ImGui::Separator();
						ImGui::Spacing();

						ImGui::BeginDisabled(!dfs_valid);
						if (ImGui::Button("Import")) {
							std::string name(general_file->name);
							std::string dfs_folder(general_file->dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								if (std::filesystem::exists(
										app->project.project_settings.project_directory +
										"/assets/general/" + name + general_file->extension)) {
									console.AddLog(
										"File with the name already exists. Please choose a "
										"different name.");
								} else {
									auto file = std::make_unique<LibdragonFile>();
									file->name = name;
									file->dfs_folder = dfs_folder;
									file->dfs_folder = dfs_folder;
									file->copy_to_filesystem = general_file->copy_to_filesystem;
									file->file_path = "assets/general/" + name +
													  general_file->extension;
									file->file_type = general_file->extension;

									std::filesystem::create_directories(
										app->project.project_settings.project_directory +
										"/assets/general");
									std::filesystem::copy_file(
										general_file->file_path,
										app->project.project_settings.project_directory +
											"/assets/general/" + name + general_file->extension);

									file->SaveToDisk(
										app->project.project_settings.project_directory);

									app->state.dropped_general_files.erase(
										app->state.dropped_general_files.begin() + (int)i);

									app->project.general_files.push_back(move(file));
									app->project.ReloadAssets();

									--i;
								}
							}
						}
						ImGui::EndDisabled();

						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							app->state.dropped_general_files.erase(
								app->state.dropped_general_files.begin() + (int)i);
							--i;
						}

						ImGui::EndTabItem();
					}
					ImGui::PopID();
					++id;
				}
				for (size_t i = 0; i < app->state.dropped_font_files.size(); ++i) {
					DroppedFont *font_file = &app->state.dropped_font_files[i];

					ImGui::PushID(id);
					if (ImGui::BeginTabItem("Font")) {
						if (font_file->width_mult > font_file->height_mult) {
							ImGui::Image((ImTextureID)(intptr_t)font_file->font_data,
										 ImVec2((float)window_width,
												(float)font_file->height_mult * window_width));
						} else {
							ImGui::Image((ImTextureID)(intptr_t)font_file->font_data,
										 ImVec2((float)font_file->width_mult * window_height,
												(float)window_height));
						}
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::InputText("Name", font_file->name, 50,
										 ImGuiInputTextFlags_CharsFileName);
						bool dfs_valid = input_text_dfs_folder(font_file->dfs_folder, 100);
						ImGui::InputInt("Font Size", &font_file->font_size);

						ImGui::Separator();
						ImGui::Spacing();

						ImGui::BeginDisabled(!dfs_valid);
						if (ImGui::Button("Import")) {
							std::string name(font_file->name);
							std::string dfs_folder(font_file->dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								std::string name_string(name);
								auto find_by_name =
									[&name_string](const std::unique_ptr<LibdragonFont> &i) {
										return i->name == name_string;
									};
								if (std::find_if(app->project.fonts.begin(),
												 app->project.fonts.end(),
												 find_by_name) != std::end(app->project.fonts)) {
									console.AddLog(
										"Font with the name already exists. Please choose a "
										"different name.");
								} else {
									auto font = std::make_unique<LibdragonFont>();
									font->name = name;
									font->dfs_folder = dfs_folder;
									font->font_size = font_file->font_size;
									font->font_path = "assets/fonts/" + name + ".ttf";

									std::filesystem::create_directories(
										app->project.project_settings.project_directory +
										"/assets/fonts");
									std::filesystem::copy_file(
										font_file->font_path,
										app->project.project_settings.project_directory +
											"/assets/fonts/" + name + ".ttf");

									font->SaveToDisk(
										app->project.project_settings.project_directory);
									font->LoadImage(app->project.project_settings.project_directory,
													app->renderer);

									SDL_DestroyTexture(font_file->font_data);

									app->state.dropped_font_files.erase(
										app->state.dropped_font_files.begin() + (int)i);

									app->project.fonts.push_back(move(font));
									app->project.ReloadAssets();
									--i;
								}
							}
						}
						ImGui::EndDisabled();

						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							SDL_DestroyTexture(font_file->font_data);

							app->state.dropped_font_files.erase(
								app->state.dropped_font_files.begin() + (int)i);
							--i;
						}

						ImGui::EndTabItem();
					}
					ImGui::PopID();
					++id;
				}
				for (size_t i = 0; i < app->state.dropped_tiled_files.size(); ++i) {
					DroppedTiledMap *map_file = &app->state.dropped_tiled_files[i];
					ImGui::PushID(id);
					if (ImGui::BeginTabItem("Map")) {
						render_badge("tiled", ImVec4(.4f, .1f, .1f, 1.f));

						ImGui::InputText("Name", map_file->name, 50,
										 ImGuiInputTextFlags_CharsFileName);
						bool dfs_valid = input_text_dfs_folder(map_file->dfs_folder, 100);
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::TextWrapped("Layers:");
						for (size_t layer_i = 0; layer_i < map_file->layers.size(); ++layer_i) {
							ImGui::TextWrapped("- %s", map_file->layers[layer_i].name.c_str());
						}

						ImGui::Separator();
						ImGui::Spacing();

						ImGui::BeginDisabled(!dfs_valid);
						if (ImGui::Button("Import")) {
							std::string name(map_file->name);
							std::string dfs_folder(map_file->dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								if (std::filesystem::exists(
										app->project.project_settings.project_directory +
										"/assets/tiled_maps/" + name + +".tmx")) {
									console.AddLog(
										"Map with the name already exists. Please choose a "
										"different name.");
								} else {
									auto file = std::make_unique<LibdragonTiledMap>();
									file->name = name;
									file->dfs_folder = dfs_folder;
									file->file_path = "assets/tiled_maps/" + name + +".tmx";
									file->layers = map_file->layers;

									std::filesystem::create_directories(
										app->project.project_settings.project_directory +
										"/assets/tiled_maps");
									std::filesystem::copy_file(
										map_file->file_path,
										app->project.project_settings.project_directory +
											"/assets/tiled_maps/" + name + ".tmx");

									file->SaveToDisk(
										app->project.project_settings.project_directory);

									app->state.dropped_tiled_files.erase(
										app->state.dropped_tiled_files.begin() + (int)i);

									app->project.tiled_maps.push_back(move(file));
									app->project.ReloadAssets();

									--i;
								}
							}
						}
						ImGui::EndDisabled();

						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							app->state.dropped_tiled_files.erase(
								app->state.dropped_tiled_files.begin() + (int)i);
							--i;
						}

						ImGui::EndTabItem();
					}
					ImGui::PopID();
					++id;
				}
				for (size_t i = 0; i < app->state.dropped_ldtk_files.size(); ++i) {
					DroppedLDtkMap *map_file = &app->state.dropped_ldtk_files[i];
					ImGui::PushID(id);
					if (ImGui::BeginTabItem("Map")) {
						render_badge("ldtk", ImVec4(.4f, .1f, .1f, 1.f));

						ImGui::InputText("Name", map_file->name, 50,
										 ImGuiInputTextFlags_CharsFileName);
						bool dfs_valid = input_text_dfs_folder(map_file->dfs_folder, 100);
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::TextWrapped("Layers:");
						for (size_t layer_i = 0; layer_i < map_file->layers.size(); ++layer_i) {
							ImGui::TextWrapped("- %s", map_file->layers[layer_i].name.c_str());
						}

						ImGui::Separator();
						ImGui::Spacing();

						ImGui::BeginDisabled(!dfs_valid);
						if (ImGui::Button("Import")) {
							std::string name(map_file->name);
							std::string dfs_folder(map_file->dfs_folder);

							if (name.empty() || dfs_folder.empty()) {
								console.AddLog(
									"[error] Please fill both 'name' and 'dfs folder' fields");
							} else {
								if (std::filesystem::exists(
										app->project.project_settings.project_directory +
										"/assets/ldtk_maps/" + name + +".ldtk")) {
									console.AddLog(
										"Map with the name already exists. Please choose a "
										"different name.");
								} else {
									auto file = std::make_unique<LibdragonLDtkMap>();
									file->name = name;
									file->dfs_folder = dfs_folder;
									file->file_path = "assets/ldtk_maps/" + name + +".ldtk";
									file->layers = map_file->layers;

									std::filesystem::create_directories(
										app->project.project_settings.project_directory +
										"/assets/ldtk_maps");
									std::filesystem::copy_file(
										map_file->file_path,
										app->project.project_settings.project_directory +
											"/assets/ldtk_maps/" + name + ".ldtk");

									file->SaveToDisk(
										app->project.project_settings.project_directory);

									app->state.dropped_ldtk_files.erase(
										app->state.dropped_ldtk_files.begin() + (int)i);

									app->project.ldtk_maps.push_back(move(file));
									app->project.ReloadAssets();

									--i;
								}
							}
						}
						ImGui::EndDisabled();

						ImGui::SameLine();
						if (ImGui::Button("Cancel")) {
							app->state.dropped_ldtk_files.erase(
								app->state.dropped_ldtk_files.begin() + (int)i);
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
