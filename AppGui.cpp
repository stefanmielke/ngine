#include "AppGui.h"

#include <filesystem>

#include "imgui.h"

#include "App.h"
#include "CodeEditor.h"
#include "ConsoleApp.h"
#include "Emulator.h"
#include "ImportAssets.h"
#include "ProjectBuilder.h"
#include "ScriptBuilder.h"
#include "ThreadCommand.h"

static bool new_project_window_open = false;
static bool open_project_window_open = false;

static int window_width, window_height;
static bool is_output_open;

void open_url(const char *url);

void AppGui::Update(App &app) {
	is_output_open = true;

	SDL_GetWindowSize(app.window, &window_width, &window_height);

	RenderMenuBar(app);

	RenderNewProjectWindow(app);
	RenderOpenProjectWindow(app);

	ImportAssets::RenderImportScreen(&app);

	console.Draw("Output", app.window, is_output_open);

	RenderContentBrowser(app);

	RenderSceneWindow(app);

	RenderSettingsWindow(app);
}

void AppGui::RenderMenuBar(App &app) {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Project")) {
				new_project_window_open = true;
			}
			if (ImGui::MenuItem("Open Project")) {
				open_project_window_open = true;
			}
			if (ImGui::MenuItem("Close Project", nullptr, false,
								app.project.project_settings.IsOpen())) {
				app.CloseProject();
			}
			if (ImGui::MenuItem("Exit")) {
				app.is_running = false;
			}
			ImGui::EndMenu();
		}
		ImGui::MenuItem("|", nullptr, false, false);
		if (ImGui::MenuItem("Save All", nullptr, false, app.project.project_settings.IsOpen())) {
			console.AddLog("Saving Project...");

			app.project.SaveToDisk(app.project.project_settings.project_directory);
			app.project.project_settings.SaveToDisk();

			console.AddLog("Project saved.");
		}
		ImGui::MenuItem("|", nullptr, false, false);
		if (ImGui::MenuItem("Build", nullptr, false, app.project.project_settings.IsOpen())) {
			console.AddLog("Building Project...");

			ProjectBuilder::Build(app.project);
		}
		ImGui::MenuItem("|", nullptr, false, false);
		if (ImGui::BeginMenu("Tasks", app.project.project_settings.IsOpen())) {
			if (ImGui::MenuItem("Clean/Build")) {
				console.AddLog("Rebuilding Project...");

				ProjectBuilder::Rebuild(app.project);
			}
			if (ImGui::MenuItem("Regen Static Files")) {
				console.AddLog("Regenerating static files...");

				ProjectBuilder::GenerateStaticFiles(app.project.project_settings.project_directory);

				console.AddLog("Files regenerated.");
			}
			ImGui::EndMenu();
		}
		ImGui::MenuItem("|", nullptr, false, false);
		if (ImGui::MenuItem("Open in Editor", nullptr, false,
							app.project.project_settings.IsOpen())) {
			console.AddLog("Opening project in Editor...");

			CodeEditor::OpenPath(&app, app.project.project_settings.project_directory);
		}
		ImGui::MenuItem("|", nullptr, false, false);
		if (ImGui::MenuItem("Run", nullptr, false,
							app.project.project_settings.IsOpen() &&
								!app.engine_settings.GetEmulatorPath().empty())) {
			Emulator::Run(&app);
		}
		ImGui::MenuItem("|", nullptr, false, false);
		if (ImGui::BeginMenu("Help")) {
			ImGui::MenuItem("Version 1.0.0", nullptr, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("Ngine Wiki")){
				open_url("https://github.com/stefanmielke/ngine/projects?type=beta");
			}
			if (ImGui::MenuItem("Libdragon")){
				open_url("https://github.com/DragonMinded/libdragon");
			}
			if (ImGui::MenuItem("Libdragon CLI")){
				open_url("https://github.com/anacierdem/libdragon-docker");
			}
			if (ImGui::MenuItem("Libdragon Extensions")){
				open_url("https://github.com/stefanmielke/libdragon-extensions");
			}
			if (ImGui::MenuItem("SDL2")){
				open_url("https://www.libsdl.org/index.php");
			}
			if (ImGui::MenuItem("Docker Install")){
				open_url("https://www.docker.com/get-started");
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void AppGui::RenderNewProjectWindow(App &app) {
	if (new_project_window_open) {
		ImGui::SetNextWindowSize(ImVec2(300, 80));
		if (ImGui::Begin("New Project", &new_project_window_open)) {
			ImGui::TextUnformatted("Folder");
			ImGui::SameLine();
			bool create_project;
			create_project = ImGui::InputTextWithHint("##", "/path/to/project/folder",
													  app.state.input_new_project, 255,
													  ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::Button("Create", ImVec2(50, 20)) || create_project) {
				std::string new_project_folder(app.state.input_new_project);

				ProjectBuilder::Create(&app, new_project_folder);

				new_project_window_open = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(50, 20))) {
				new_project_window_open = false;
			}
			ImGui::End();
		}
	}
}

void AppGui::RenderOpenProjectWindow(App &app) {
	if (open_project_window_open) {
		ImGui::SetNextWindowSize(ImVec2(300, 80));
		if (ImGui::Begin("Open Project", &open_project_window_open)) {
			ImGui::TextUnformatted("Folder");
			ImGui::SameLine();
			bool open_project;
			open_project = ImGui::InputTextWithHint("##", "/path/to/project/folder",
													app.state.input_open_project, 255,
													ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::Button("Open", ImVec2(50, 20)) || open_project) {
				if (app.OpenProject(app.state.input_open_project)) {
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
}

void AppGui::RenderContentBrowser(App &app) {
	const float center_x_size = (float)window_width - 600;
	const float center_y_offset = is_output_open ? 219 : 38;
	ImGui::SetNextWindowSize(ImVec2(center_x_size, (float)window_height - center_y_offset));
	ImGui::SetNextWindowPos(ImVec2(300, 19));
	if (ImGui::Begin("ContentBrowser", nullptr,
					 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
		if (ImGui::BeginTabBar("CenterContentTabs",
							   ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
			if (ImGui::BeginTabItem("Sprites Browser")) {
				if (app.project.project_settings.IsOpen()) {
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						ImGui::OpenPopup("PopupSpritesBrowser");
					}

					if (ImGui::BeginPopup("PopupSpritesBrowser")) {
						if (ImGui::Selectable("Refresh")) {
							app.project.ReloadImages(app.renderer);
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
							if (app.state.selected_image) {
								app.state.image_editing = app.state.selected_image;
								app.state.reload_image_edit = true;
								app.state.selected_image = nullptr;
							}
						}
						if (ImGui::Selectable("Copy DFS Path")) {
							if (app.state.selected_image) {
								std::string dfs_path((*app.state.selected_image)->dfs_folder +
													 (*app.state.selected_image)->name + ".sprite");
								ImGui::SetClipboardText(dfs_path.c_str());
								app.state.selected_image = nullptr;
							}
						}
						if (ImGui::Selectable("Delete")) {
							if (app.state.selected_image) {
								(*app.state.selected_image)
									->DeleteFromDisk(
										app.project.project_settings.project_directory);

								for (size_t i = 0; i < app.project.images.size(); ++i) {
									if (app.project.images[i]->image_path ==
										(*app.state.selected_image)->image_path) {
										app.project.images.erase(app.project.images.begin() +
																 (int)i);
										break;
									}
								}
								app.state.selected_image = nullptr;
							}
						}
						ImGui::EndPopup();
					}

					ImGui::TextWrapped("Drag & Drop files anywhere to import.");
					ImGui::Separator();

					if (!app.project.project_settings.modules.dfs) {
						ImGui::TextWrapped(
							"DFS MODULE IS NOT LOADED. CONTENT WILL NOT BE USABLE IN THE GAME.");
						ImGui::Separator();
					}
					if (!app.project.project_settings.modules.display) {
						ImGui::TextWrapped(
							"DISPLAY MODULE IS NOT LOADED. SPRITES WILL NOT BE USABLE IN THE "
							"GAME.");
						ImGui::Separator();
					}
					if (!app.project.project_settings.modules.rdp) {
						ImGui::TextWrapped(
							"RDP MODULE IS NOT LOADED. HARDWARE RENDERING (rdp_* functions) WILL "
							"NOT BE USABLE IN THE GAME.");
						ImGui::Separator();
					}

					for (auto &image : app.project.images) {
						if (ImGui::ImageButton((ImTextureID)(intptr_t)image->loaded_image,
											   ImVec2(item_size, item_size))) {
							app.state.selected_image = &image;
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
										 ImVec2((float)image->width, (float)image->height));
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
				if (app.project.project_settings.IsOpen()) {
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						ImGui::OpenPopup("PopupSoundsBrowser");
					}

					if (ImGui::BeginPopup("PopupSoundsBrowser")) {
						if (ImGui::Selectable("Refresh")) {
							app.project.ReloadSounds();
						}
						ImGui::EndPopup();
					}

					int cur_i = 0;
					if (ImGui::BeginPopup("PopupSoundsBrowserSound")) {
						if (ImGui::Selectable("Edit Settings")) {
							if (app.state.selected_sound) {
								app.state.sound_editing = app.state.selected_sound;
								app.state.reload_sound_edit = true;
								app.state.selected_sound = nullptr;
							}
						}
						if (ImGui::Selectable("Copy DFS Path")) {
							if (app.state.selected_sound) {
								std::string dfs_path;
								if ((*app.state.selected_sound)->type == SOUND_XM ||
									(*app.state.selected_sound)->type == SOUND_YM) {
									dfs_path.append("rom:");
								}

								dfs_path.append(
									(*app.state.selected_sound)->dfs_folder +
									(*app.state.selected_sound)->name +
									(*app.state.selected_sound)->GetLibdragonExtension());
								ImGui::SetClipboardText(dfs_path.c_str());
								app.state.selected_sound = nullptr;
							}
						}
						if (ImGui::Selectable("Delete")) {
							if (app.state.selected_sound) {
								(*app.state.selected_sound)
									->DeleteFromDisk(
										app.project.project_settings.project_directory);

								for (size_t i = 0; i < app.project.sounds.size(); ++i) {
									if (app.project.sounds[i]->sound_path ==
										(*app.state.selected_sound)->sound_path) {
										app.project.sounds.erase(app.project.sounds.begin() +
																 (int)i);
										break;
									}
								}
								app.state.selected_sound = nullptr;
							}
						}
						ImGui::EndPopup();
					}

					ImGui::TextWrapped("Drag & Drop files anywhere to import.");
					ImGui::Separator();

					if (!app.project.project_settings.modules.dfs) {
						ImGui::TextWrapped(
							"DFS MODULE IS NOT LOADED. CONTENT WILL NOT BE USABLE IN THE GAME.");
						ImGui::Separator();
					}
					if (!app.project.project_settings.modules.audio) {
						ImGui::TextWrapped(
							"AUDIO MODULE IS NOT LOADED. AUDIO WILL NOT BE USABLE IN THE GAME.");
						ImGui::Separator();
					}
					if (!app.project.project_settings.modules.audio_mixer) {
						ImGui::TextWrapped(
							"AUDIO MIXER MODULE IS NOT LOADED. SOME AUDIO WILL NOT BE USABLE IN "
							"THE GAME.");
						ImGui::Separator();
					}

					for (auto &sound : app.project.sounds) {
						if (ImGui::Selectable(sound->name.c_str())) {
							app.state.selected_sound = &sound;
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
				if (app.project.project_settings.IsOpen()) {
					static char script_name_input[100] = {};
					bool create_script;
					create_script = ImGui::InputTextWithHint("##ScriptName", "script_name",
															 script_name_input, 100,
															 ImGuiInputTextFlags_EnterReturnsTrue);
					ImGui::SameLine();
					if (ImGui::Button("Create Script") || create_script) {
						std::string script_name(script_name_input);
						if (!script_name.empty()) {
							if (ScriptBuilder::CreateScriptFile(app.project.project_settings,
																script_name_input)) {
								memset(script_name_input, 0, 100);

								app.project.ReloadScripts();
							}
						}
					}
					ImGui::Separator();
					for (auto &script_name : app.project.script_files) {
						ImGui::TextUnformatted(script_name.c_str());
						ImGui::SameLine();
						ImGui::PushID(script_name.c_str());
						if (ImGui::SmallButton("Edit")) {
							std::string path = app.project.project_settings.project_directory +
											   "/src/scripts/" + script_name + ".script.c";
							CodeEditor::OpenPath(&app, path);
						}
						ImGui::PopID();
						ImGui::SameLine();
						ImGui::PushID((script_name + "D").c_str());
						if (ImGui::SmallButton("Delete")) {
							ScriptBuilder::DeleteScriptFile(app.project, script_name.c_str());

							for (size_t i = 0; i < app.project.script_files.size(); ++i) {
								if (app.project.script_files[i] == script_name) {
									app.project.script_files.erase(
										app.project.script_files.begin() + (int)i);
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
}

void AppGui::RenderSceneWindow(App &app) {
	const float prop_y_size = is_output_open ? 219 : 38;
	ImGui::SetNextWindowSize(ImVec2(300, (float)window_height - prop_y_size));
	ImGui::SetNextWindowPos(ImVec2(0, 19));
	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		if (app.state.current_scene && app.project.project_settings.modules.scene_manager) {
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
					ImGui::TextColored(ImColor(100, 100, 255), "Id: %d",
									   app.state.current_scene->id);
					ImGui::Spacing();
					ImGui::Separator();

					ImGui::Spacing();
					ImGui::InputText("Name", app.state.scene_name, 100);

					ImGui::Spacing();
					ImGui::TextUnformatted("Background Fill Color");
					ImGui::ColorPicker3("##FillColor", app.state.current_scene->fill_color);

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
					{
						std::string current_selected(app.state.current_scene->script_name);
						ImGui::TextUnformatted("Attached Script");
						if (ImGui::BeginCombo("##AttachedScript", current_selected.c_str())) {
							for (auto &script : app.project.script_files) {
								if (ImGui::Selectable(script.c_str(), script == current_selected)) {
									app.state.current_scene->script_name = script;
								}
							}
							ImGui::EndCombo();
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Remove")) {
							app.state.current_scene->script_name.clear();
						}
					}
					ImGui::Spacing();

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Save")) {
						app.state.current_scene->name = app.state.scene_name;
						app.project.SaveToDisk(app.project.project_settings.project_directory);
						app.project.project_settings.SaveToDisk();
					}
					ImGui::Spacing();

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Delete Scene")) {
						for (size_t i = 0; i < app.project.scenes.size(); ++i) {
							if (app.project.scenes[i].id == app.state.current_scene->id) {
								app.project.scenes.erase(app.project.scenes.begin() + (int)i);
								std::string filename = app.project.project_settings
														   .project_directory +
													   "/.ngine/scenes/" +
													   std::to_string(app.state.current_scene->id) +
													   ".scene.json";
								std::filesystem::remove(filename);

								app.state.current_scene = nullptr;
								break;
							}
						}
					}

					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		} else if (app.project.project_settings.IsOpen() &&
				   app.project.project_settings.modules.scene_manager) {
			ImGui::TextWrapped("Load a scene on the right side under 'Scenes' tab.");
		} else if (app.project.project_settings.IsOpen() &&
				   !app.project.project_settings.modules.scene_manager) {
			ImGui::TextWrapped("Please enable 'Scene Manager' module to use this feature.");
		}
	}
	ImGui::End();
}

void AppGui::RenderSettingsWindow(App &app) {
	const float prop_x_size = 300;
	const float prop_y_size = is_output_open ? 219 : 38;
	ImGui::SetNextWindowSize(ImVec2(prop_x_size, (float)window_height - prop_y_size));
	ImGui::SetNextWindowPos(ImVec2((float)window_width - prop_x_size, 19));
	if (ImGui::Begin("General Settings", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_NoTitleBar)) {
		if (ImGui::BeginTabBar("Properties", ImGuiTabBarFlags_AutoSelectNewTabs)) {
			if (app.state.image_editing) {
				static char image_edit_name[50];
				static char image_edit_dfs_folder[100];
				static int image_edit_h_slices = 0;
				static int image_edit_v_slices = 0;
				if (app.state.reload_image_edit) {
					app.state.reload_image_edit = false;

					strcpy(image_edit_name, (*app.state.image_editing)->name.c_str());
					strcpy(image_edit_dfs_folder, (*app.state.image_editing)->dfs_folder.c_str());
					image_edit_h_slices = (*app.state.image_editing)->h_slices;
					image_edit_v_slices = (*app.state.image_editing)->v_slices;
				}
				if (ImGui::BeginTabItem("Image Settings")) {
					ImGui::Image((ImTextureID)(intptr_t)(*app.state.image_editing)->loaded_image,
								 ImVec2((float)(*app.state.image_editing)->display_width * 2.f,
										(float)(*app.state.image_editing)->display_height * 2.f));
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
						if ((*app.state.image_editing)->name != image_edit_name) {
							std::string name_string(image_edit_name);
							auto find_by_name =
								[&name_string](const std::unique_ptr<LibdragonImage> &i) {
									return i->name == name_string;
								};
							if (std::find_if(app.project.images.begin(), app.project.images.end(),
											 find_by_name) != std::end(app.project.images)) {
								console.AddLog(
									"Image with the name already exists. Please choose a "
									"different name.");
								will_save = false;
							} else {
								std::filesystem::copy_file(
									app.project.project_settings.project_directory + "/" +
										(*app.state.image_editing)->image_path,
									app.project.project_settings.project_directory +
										"/assets/sprites/" + image_edit_name + ".png");
								(*app.state.image_editing)
									->DeleteFromDisk(
										app.project.project_settings.project_directory);
							}
						}

						if (will_save) {
							(*app.state.image_editing)->name = image_edit_name;
							(*app.state.image_editing)->dfs_folder = image_edit_dfs_folder;
							(*app.state.image_editing)->h_slices = image_edit_h_slices;
							(*app.state.image_editing)->v_slices = image_edit_v_slices;
							(*app.state.image_editing)
								->image_path = "assets/sprites/" +
											   (*app.state.image_editing)->name + ".png";

							(*app.state.image_editing)
								->SaveToDisk(app.project.project_settings.project_directory);

							app.state.image_editing = nullptr;
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel")) {
						app.state.image_editing = nullptr;
					}

					ImGui::EndTabItem();
				}
			}
			if (app.state.sound_editing) {
				static char sound_edit_name[50];
				static char sound_edit_dfs_folder[100];
				if (app.state.reload_sound_edit) {
					app.state.reload_sound_edit = false;

					strcpy(sound_edit_name, (*app.state.sound_editing)->name.c_str());
					strcpy(sound_edit_dfs_folder, (*app.state.sound_editing)->dfs_folder.c_str());
				}
				if (ImGui::BeginTabItem("Sound Settings")) {
					ImGui::InputText("Name", sound_edit_name, 50);
					ImGui::InputText("DFS Folder", sound_edit_dfs_folder, 100);

					if ((*app.state.sound_editing)->type == SOUND_WAV) {
						ImGui::Checkbox("Loop", &(*app.state.sound_editing)->wav_loop);
						if ((*app.state.sound_editing)->wav_loop) {
							ImGui::InputInt("Loop Offset",
											&(*app.state.sound_editing)->wav_loop_offset);
						}
					} else if ((*app.state.sound_editing)->type == SOUND_YM) {
						ImGui::Checkbox("Compress", &(*app.state.sound_editing)->ym_compress);
					}

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Save")) {
						bool will_save = true;
						if ((*app.state.sound_editing)->name != sound_edit_name) {
							std::string name_string(sound_edit_name);
							auto find_by_name =
								[&name_string](const std::unique_ptr<LibdragonSound> &i) {
									return i->name == name_string;
								};
							if (std::find_if(app.project.sounds.begin(), app.project.sounds.end(),
											 find_by_name) != std::end(app.project.sounds)) {
								console.AddLog(
									"Sound with the name already exists. Please choose a "
									"different name.");
								will_save = false;
							} else {
								std::filesystem::copy_file(
									app.project.project_settings.project_directory + "/" +
										(*app.state.sound_editing)->sound_path,
									app.project.project_settings.project_directory +
										"/assets/sounds/" + sound_edit_name +
										(*app.state.sound_editing)->GetExtension());
								(*app.state.sound_editing)
									->DeleteFromDisk(
										app.project.project_settings.project_directory);
							}
						}

						if (will_save) {
							(*app.state.sound_editing)->name = sound_edit_name;
							(*app.state.sound_editing)->dfs_folder = sound_edit_dfs_folder;
							(*app.state.sound_editing)
								->sound_path = "assets/sounds/" + (*app.state.sound_editing)->name +
											   (*app.state.sound_editing)->GetExtension();

							(*app.state.sound_editing)
								->SaveToDisk(app.project.project_settings.project_directory);

							app.state.sound_editing = nullptr;
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel")) {
						app.state.sound_editing = nullptr;
					}

					ImGui::EndTabItem();
				}
			}
			if (ImGui::BeginTabItem("Scenes")) {
				if (app.project.project_settings.IsOpen() &&
					app.project.project_settings.modules.scene_manager) {
					for (auto &scene : app.project.scenes) {
						if (ImGui::Selectable(scene.name.c_str(),
											  app.state.current_scene &&
												  scene.id == app.state.current_scene->id)) {
							app.state.current_scene = &scene;
							strcpy(app.state.scene_name, app.state.current_scene->name.c_str());
						}
					}

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Create New Scene")) {
						app.project.scenes.emplace_back();
						app.state.current_scene = &app.project
													   .scenes[app.project.scenes.size() - 1];
						app.state.current_scene->id = app.project.project_settings
														  .next_scene_id++;
						app.state.current_scene->name = std::to_string(
							app.project.scenes.size());
						strcpy(app.state.scene_name, app.state.current_scene->name.c_str());
					}
				} else if (app.project.project_settings.IsOpen()) {
					ImGui::TextWrapped("Please enable 'Scene Manager' module to use this feature.");
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Project")) {
				if (app.project.project_settings.IsOpen()) {
					if (ImGui::BeginTabBar("ProjectAllSettings")) {
						if (ImGui::BeginTabItem("General")) {
							ImGui::InputText("Name", app.state.project_settings_screen.project_name,
											 100);
							ImGui::InputText("Rom", app.state.project_settings_screen.rom_name,
											 100);

							ImGui::Separator();
							{
								ImGui::TextUnformatted("Global Script");
								if (ImGui::BeginCombo(
										"##GlobalScript",
										app.project.project_settings.global_script_name.c_str())) {
									for (auto &script : app.project.script_files) {
										if (ImGui::Selectable(script.c_str(),
															  script == app.project.project_settings
																			.global_script_name)) {
											app.project.project_settings
												.global_script_name = script;
										}
									}
									ImGui::EndCombo();
								}
								ImGui::SameLine();
								if (ImGui::Button("Remove")) {
									app.project.project_settings.global_script_name = "";
								}
							}

							ImGui::Separator();
							{
								ImGui::BeginDisabled(
									!app.project.project_settings.modules.scene_manager);

								std::string current_selected("None");
								for (auto &scene : app.project.scenes) {
									if (scene.id ==
										app.project.project_settings.initial_screen_id) {
										current_selected = scene.name;
										break;
									}
								}
								ImGui::TextUnformatted("Initial Screen");
								if (ImGui::BeginCombo("##InitialScreen",
													  current_selected.c_str())) {
									for (auto &scene : app.project.scenes) {
										if (ImGui::Selectable(
												scene.name.c_str(),
												scene.id == app.project.project_settings
																.initial_screen_id)) {
											app.project.project_settings
												.initial_screen_id = scene.id;
										}
									}
									ImGui::EndCombo();
								}
								ImGui::EndDisabled();
							}

							ImGui::Separator();

							ImGui::BeginDisabled(!app.project.project_settings.modules.memory_pool);
							ImGui::TextUnformatted("Global Memory Reserve (KB)");
							ImGui::InputInt("##GlobalMem",
											&app.project.project_settings.global_mem_alloc_size, 1,
											1024);
							ImGui::TextUnformatted("Scene Memory Reserve (KB)");
							ImGui::InputInt("##LocalMem",
											&app.project.project_settings.scene_mem_alloc_size, 1,
											1024);
							ImGui::EndDisabled();

							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Modules")) {
							if (ImGui::Checkbox("Audio",
												&app.project.project_settings.modules.audio)) {
								if (!app.project.project_settings.modules.audio)
									app.project.project_settings.modules.audio_mixer = false;
							}
							ImGui::BeginDisabled(!app.project.project_settings.modules.audio);
							ImGui::Checkbox("Audio Mixer",
											&app.project.project_settings.modules.audio_mixer);
							ImGui::EndDisabled();

							if (ImGui::Checkbox("Display",
												&app.project.project_settings.modules.display)) {
								if (!app.project.project_settings.modules.display)
									app.project.project_settings.modules.rdp = false;
							}

							ImGui::BeginDisabled(!app.project.project_settings.modules.display);
							ImGui::Checkbox("RDP", &app.project.project_settings.modules.rdp);
							ImGui::EndDisabled();

							ImGui::Checkbox("Console",
											&app.project.project_settings.modules.console);
							ImGui::Checkbox("Controller",
											&app.project.project_settings.modules.controller);
							ImGui::Checkbox("Debug Is Viewer",
											&app.project.project_settings.modules.debug_is_viewer);
							ImGui::Checkbox("Debug USB",
											&app.project.project_settings.modules.debug_usb);
							ImGui::Checkbox("DFS", &app.project.project_settings.modules.dfs);
							ImGui::Checkbox("Memory Pool",
											&app.project.project_settings.modules.memory_pool);
							ImGui::Checkbox("Scene Manager",
											&app.project.project_settings.modules.scene_manager);
							ImGui::Checkbox("Timer", &app.project.project_settings.modules.timer);

							ImGui::EndTabItem();
						}

						if (app.project.project_settings.modules.audio) {
							if (ImGui::BeginTabItem("Audio")) {
								ImGui::InputInt("Frequency",
												&app.project.project_settings.audio.frequency);
								ImGui::InputInt("Buffers",
												&app.project.project_settings.audio.buffers);

								ImGui::BeginDisabled(
									!app.project.project_settings.modules.audio_mixer);
								ImGui::Separator();
								ImGui::Spacing();
								ImGui::TextUnformatted("Mixer Settings:");
								ImGui::Spacing();
								ImGui::InputInt("Channels",
												&app.project.project_settings.audio_mixer.channels);
								ImGui::EndDisabled();

								ImGui::EndTabItem();
							}
						}

						static int antialias_current = app.project.project_settings.display
														   .antialias;
						static int bit_depth_current = app.project.project_settings.display
														   .bit_depth;
						static int gamma_current = app.project.project_settings.display.gamma;
						static int resolution_current = app.project.project_settings.display
															.resolution;

						const char *antialias_items[] = {"ANTIALIAS_OFF", "ANTIALIAS_RESAMPLE",
														 "ANTIALIAS_RESAMPLE_FETCH_NEEDED",
														 "ANTIALIAS_RESAMPLE_FETCH_ALWAYS"};
						const char *bit_depth_items[] = {"DEPTH_16_BPP", "DEPTH_32_BPP"};
						const char *gamma_items[] = {"GAMMA_NONE", "GAMMA_CORRECT",
													 "GAMMA_CORRECT_DITHER"};
						const char *resolution_items[] = {
							"RESOLUTION_320x240", "RESOLUTION_640x480", "RESOLUTION_256x240",
							"RESOLUTION_512x480", "RESOLUTION_512x240", "RESOLUTION_640x240"};

						if (app.project.project_settings.modules.display) {
							if (ImGui::BeginTabItem("Display")) {
								ImGui::Combo("Antialias", &antialias_current, antialias_items, 4);
								ImGui::Combo("Bit Depth", &bit_depth_current, bit_depth_items, 2);
								ImGui::SliderInt("Buffers",
												 &app.state.project_settings_screen.display_buffers,
												 1, 3);
								ImGui::Combo("Gamma", &gamma_current, gamma_items, 3);
								ImGui::Combo("Resolution", &resolution_current, resolution_items,
											 6);

								ImGui::EndTabItem();
							}
						}

						ImGui::Separator();
						ImGui::Spacing();
						if (ImGui::Button("Save")) {
							strcpy(app.state.project_settings_screen.display_antialias,
								   antialias_items[antialias_current]);
							strcpy(app.state.project_settings_screen.display_bit_depth,
								   bit_depth_items[bit_depth_current]);
							strcpy(app.state.project_settings_screen.display_gamma,
								   gamma_items[gamma_current]);
							strcpy(app.state.project_settings_screen.display_resolution,
								   resolution_items[resolution_current]);

							app.state.project_settings_screen.ToProjectSettings(
								app.project.project_settings);

							app.project.project_settings.SaveToDisk();

							SDL_SetWindowTitle(
								app.window,
								("NGine - " + app.project.project_settings.project_name + " - " +
								 app.project.project_settings.project_directory)
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
				ImGui::InputText("##EmuPath", app.state.emulator_path, 255);

				ImGui::TextUnformatted("Editor Path (?)");
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip(
						"example: '/path/to/editor'.\nWe will run 'path "
						"path/to/file/or/directory'.");
				}
				ImGui::InputTextWithHint("##EditorPath", "use 'code' for VSCode",
										 app.state.editor_path, 255);

				{
					ImGui::TextUnformatted("Theme");
					static int selected_theme = (int)app.engine_settings.GetTheme();
					if (ImGui::Combo("##Theme", &selected_theme, "Dark\0Light\0Classic\0")) {
						ChangeTheme(app, (Theme)selected_theme);
					}
				}

				ImGui::Spacing();
				if (ImGui::Button("Save")) {
					app.engine_settings.SetEmulatorPath(app.state.emulator_path);
					app.engine_settings.SetEditorLocation(app.state.editor_path);
				}

				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void AppGui::ChangeTheme(App &app, Theme theme) {
	app.engine_settings.SetTheme(theme);

	switch (theme) {
		case THEME_DARK:
			ImGui::StyleColorsDark();
			break;
		case THEME_LIGHT:
			ImGui::StyleColorsLight();
			break;
		case THEME_CLASSIC:
			ImGui::StyleColorsClassic();
			break;
	}
}

void open_url(const char *url) {
#ifdef __LINUX__
	std::string command("xdg-open ");
#else
	std::string command("start ");
#endif
	command.append(url);

	ThreadCommand::RunCommandDetached(command);
}