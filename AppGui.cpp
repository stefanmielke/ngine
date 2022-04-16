#include "AppGui.h"

#include <cmath>
#include <filesystem>
#include <SDL2/SDL.h>

#include "imgui.h"
#include "imgui_custom.h"
#include "imgui/ImGuiFileDialog/ImGuiFileDialog.h"

#include "App.h"
#include "CodeEditor.h"
#include "ConsoleApp.h"
#include "Emulator.h"
#include "ImportAssets.h"
#include "Libdragon.h"
#include "ProjectBuilder.h"
#include "ScriptBuilder.h"
#include "ThreadCommand.h"

static int window_width, window_height;
static bool is_output_open;

static bool display_sprites = true, display_sounds = true, display_files = true;
static char assets_name_filter[100];

const float details_window_size = 172;

void open_url(const char *url);

enum AssetsDisplayType { ADT_LIST, ADT_GRID };
static AssetsDisplayType asset_display_type = ADT_LIST;

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

void AppGui::ProcessImportFile(App &app, std::string file_path) {
	if (file_path.ends_with(".png") || file_path.ends_with(".bmp")) {
		LibdragonImageType type;
		if (file_path.ends_with(".png"))
			type = IMAGE_PNG;
		else if (file_path.ends_with(".bmp"))
			type = IMAGE_BMP;

		DroppedImage dropped_image(file_path.c_str(), type);

		std::filesystem::path filepath(file_path);
		std::string filename = filepath.filename().replace_extension().string();
		std::replace(filename.begin(), filename.end(), ' ', '_');

		strcpy(dropped_image.name, filename.c_str());

		dropped_image.image_data = IMG_LoadTexture(app.renderer, file_path.c_str());

		int w, h;
		SDL_QueryTexture(dropped_image.image_data, nullptr, nullptr, &w, &h);

		const float max_size = 300.f;
		if (w > h) {
			dropped_image.height_mult = (float)h / (float)w;
			h = (int)(dropped_image.height_mult * max_size);
			w = (int)max_size;
		} else {
			dropped_image.width_mult = (float)w / (float)h;
			w = (int)(dropped_image.width_mult * max_size);
			h = (int)max_size;
		}

		dropped_image.w = w;
		dropped_image.h = h;

		app.state.dropped_image_files.push_back(dropped_image);

		ImGui::SetWindowFocus("Import Assets");
	} else if (file_path.ends_with(".wav") || file_path.ends_with(".xm") ||
			   file_path.ends_with(".ym")) {
		LibdragonSoundType type;
		if (file_path.ends_with(".wav"))
			type = SOUND_WAV;
		else if (file_path.ends_with(".xm"))
			type = SOUND_XM;
		else
			type = SOUND_YM;

		DroppedSound dropped_sound(file_path.c_str(), type);
		std::filesystem::path filepath(file_path);
		std::string filename = filepath.filename().replace_extension().string();
		std::replace(filename.begin(), filename.end(), ' ', '_');

		strcpy(dropped_sound.name, filename.c_str());

		app.state.dropped_sound_files.push_back(dropped_sound);

		ImGui::SetWindowFocus("Import Assets");
	} else {
		DroppedGeneralFile dropped_file(file_path.c_str());

		std::filesystem::path filepath(file_path);
		std::string filename = filepath.filename().replace_extension().string();
		std::replace(filename.begin(), filename.end(), ' ', '_');

		strcpy(dropped_file.name, filename.c_str());
		strcpy(dropped_file.extension, filepath.extension().string().c_str());

		app.state.dropped_general_files.push_back(dropped_file);

		ImGui::SetWindowFocus("Import Assets");
	}
}

void AppGui::RenderMenuBar(App &app) {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Project")) {
				ImGuiFileDialog::Instance()->OpenDialog("NewProjectDlgKey", "Choose Folder",
														nullptr,
														app.engine_settings.GetLastOpenedProject());
			}
			if (ImGui::MenuItem("Open Project")) {
				ImGuiFileDialog::Instance()->OpenDialog("OpenProjectDlgKey", "Choose Folder",
														nullptr,
														app.engine_settings.GetLastOpenedProject());
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
		if (ImGui::BeginMenu("Tasks", app.project.project_settings.IsOpen())) {
			if (ImGui::MenuItem("Regen Static Files")) {
				console.AddLog("Regenerating static files...");

				ProjectBuilder::GenerateStaticFiles(app.project.project_settings.project_directory);

				console.AddLog("Files regenerated.");
			}
			if (ImGui::MenuItem("Disassembly ROM")) {
				console.AddLog("Building Project...");
				ProjectBuilder::Build(&app);
				Libdragon::Disasm(app.project.project_settings.project_directory,
								  app.engine_settings.GetLibdragonExeLocation());

				console.AddLog("Assembly output to 'rom.asm' file.");
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			ImGui::MenuItem(app.engine_version.version_string.c_str(), nullptr, false, false);
			if (ImGui::MenuItem(app.engine_settings.GetLibdragonVersion().c_str(), nullptr, false,
								app.engine_settings.GetLibdragonVersion().starts_with("Update"))) {
				open_url("https://github.com/anacierdem/libdragon-docker/releases/latest");
			}
			ImGui::Separator();
			ImGui::MenuItem("Development Resources", nullptr, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("N64Brew Wiki")) {
				open_url("https://n64brew.dev/wiki/Main_Page");
			}
			if (ImGui::MenuItem("N64Brew Discord")) {
				open_url("https://discord.gg/WqFgNWf");
			}
			if (ImGui::MenuItem("Awesome N64 Development List")) {
				open_url("https://n64.dev/");
			}
			if (ImGui::MenuItem("N64 Dev Manual - Libultra")) {
				open_url("https://ultra64.ca/resources/documentation/");
			}
			ImGui::Separator();
			ImGui::MenuItem("Engine Resources", nullptr, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("Ngine Wiki")) {
				open_url("https://github.com/stefanmielke/ngine/wiki");
			}
			if (ImGui::MenuItem("Libdragon")) {
				open_url("https://github.com/DragonMinded/libdragon");
			}
			if (ImGui::MenuItem("Libdragon CLI")) {
				open_url("https://github.com/anacierdem/libdragon-docker");
			}
			if (ImGui::MenuItem("Libdragon Extensions")) {
				open_url("https://github.com/stefanmielke/libdragon-extensions");
			}
			if (ImGui::MenuItem("SDL2")) {
				open_url("https://www.libsdl.org/index.php");
			}
			if (ImGui::MenuItem("SDL2 Image")) {
				open_url("https://www.libsdl.org/projects/SDL_image/");
			}
			if (ImGui::MenuItem("Docker Install")) {
				open_url("https://www.docker.com/get-started");
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void AppGui::RenderNewProjectWindow(App &app) {
	ImGui::SetNextWindowSize(ImVec2(680, 330), ImGuiCond_Once);
	if (ImGuiFileDialog::Instance()->Display("NewProjectDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			ProjectBuilder::Create(&app, ImGuiFileDialog::Instance()->GetCurrentPath());
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void AppGui::RenderOpenProjectWindow(App &app) {
	ImGui::SetNextWindowSize(ImVec2(680, 330), ImGuiCond_Once);
	if (ImGuiFileDialog::Instance()->Display("OpenProjectDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			app.OpenProject(ImGuiFileDialog::Instance()->GetCurrentPath());
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void set_up_popup_windows(App &app) {
	if (ImGui::BeginPopup("PopupSpritesBrowserImage")) {
		if (ImGui::Selectable("Edit Settings")) {
			if (app.state.asset_selected.Type() == IMAGE) {
				app.state.asset_editing = app.state.asset_selected;
				app.state.reload_asset_edit = true;
			}
		}
		if (ImGui::Selectable("Copy DFS Path")) {
			if (app.state.asset_selected.Type() == IMAGE) {
				std::string dfs_path((*app.state.asset_selected.Ref().image)->dfs_folder +
									 (*app.state.asset_selected.Ref().image)->name + ".sprite");
				ImGui::SetClipboardText(dfs_path.c_str());
			}
		}
		if (ImGui::Selectable("Delete")) {
			if (app.state.asset_selected.Type() == IMAGE) {
				(*app.state.asset_selected.Ref().image)
					->DeleteFromDisk(app.project.project_settings.project_directory);

				for (size_t i = 0; i < app.project.images.size(); ++i) {
					if (app.project.images[i]->image_path ==
						(*app.state.asset_selected.Ref().image)->image_path) {
						app.project.images.erase(app.project.images.begin() + (int)i);
						break;
					}
				}

				app.state.asset_selected.Reset();
				app.state.asset_editing.Reset();
				app.project.ReloadAssets();
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("PopupSoundsBrowserSound")) {
		if (ImGui::Selectable("Edit Settings")) {
			if (app.state.asset_selected.Type() == SOUND) {
				app.state.asset_editing = app.state.asset_selected;
				app.state.reload_asset_edit = true;
			}
		}
		if (ImGui::Selectable("Copy DFS Path")) {
			if (app.state.asset_selected.Type() == SOUND) {
				std::string dfs_path;
				if ((*app.state.asset_selected.Ref().sound)->type == SOUND_XM ||
					(*app.state.asset_selected.Ref().sound)->type == SOUND_YM) {
					dfs_path.append("rom:");
				}

				dfs_path.append((*app.state.asset_selected.Ref().sound)->dfs_folder +
								(*app.state.asset_selected.Ref().sound)->name +
								(*app.state.asset_selected.Ref().sound)->GetLibdragonExtension());
				ImGui::SetClipboardText(dfs_path.c_str());
			}
		}
		if (ImGui::Selectable("Delete")) {
			if (app.state.asset_selected.Type() == SOUND) {
				(*app.state.asset_selected.Ref().sound)
					->DeleteFromDisk(app.project.project_settings.project_directory);

				for (size_t i = 0; i < app.project.sounds.size(); ++i) {
					if (app.project.sounds[i]->sound_path ==
						(*app.state.asset_selected.Ref().sound)->sound_path) {
						app.project.sounds.erase(app.project.sounds.begin() + (int)i);
						break;
					}
				}

				app.state.asset_selected.Reset();
				app.state.asset_editing.Reset();
				app.project.ReloadAssets();
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("PopupContentsBrowserContent")) {
		if (ImGui::Selectable("Edit Settings")) {
			if (app.state.asset_selected.Type() == GENERAL) {
				app.state.asset_editing = app.state.asset_selected;
				app.state.reload_asset_edit = true;
			}
		}
		if (ImGui::Selectable("Copy DFS Path")) {
			if (app.state.asset_selected.Type() == GENERAL) {
				std::string dfs_path;
				dfs_path.append((*app.state.asset_selected.Ref().file)->dfs_folder +
								(*app.state.asset_selected.Ref().file)->GetFilename());

				ImGui::SetClipboardText(dfs_path.c_str());
			}
		}
		if (ImGui::Selectable("Delete")) {
			if (app.state.asset_selected.Type() == GENERAL) {
				(*app.state.asset_selected.Ref().file)
					->DeleteFromDisk(app.project.project_settings.project_directory);

				for (size_t i = 0; i < app.project.general_files.size(); ++i) {
					if (app.project.general_files[i]->GetFilename() ==
						(*app.state.asset_selected.Ref().file)->GetFilename()) {
						app.project.general_files.erase(app.project.general_files.begin() + (int)i);

						break;
					}
				}

				app.state.asset_selected.Reset();
				app.state.asset_editing.Reset();
				app.project.ReloadAssets();
			}
		}
		ImGui::EndPopup();
	}
}

void render_asset_folder_list(App &app, Asset *folder) {
	set_up_popup_windows(app);

	for (auto &asset : folder->children) {
		switch (asset.GetType()) {
			case FOLDER:
				if (ImGui::TreeNodeEx(asset.GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
					render_asset_folder_list(app, &asset);
					ImGui::TreePop();
				}
				break;
			case IMAGE: {
				if (display_sprites) {
					std::string name = asset.GetName();
					if (name.find(assets_name_filter) != name.npos) {
						if (!app.project.project_settings.modules.display) {
							ImVec2 uv0, uv1;
							app.GetImagePosition("Error_Icon.png", uv0, uv1);
							ImGui::PushID(0);
							if (ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture),
												   ImVec2(9, 9), uv0, uv1)) {
								app.project.project_settings.modules.display = true;
							}
							ImGui::PopID();
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip(
									"Display module is disabled. Click here to enable it.");
							}
							ImGui::SameLine();
						} else if (!app.project.project_settings.modules.rdp) {
							ImVec2 uv0, uv1;
							app.GetImagePosition("Warning_Icon.png", uv0, uv1);
							ImGui::PushID(0);
							if (ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture),
												   ImVec2(9, 9), uv0, uv1)) {
								app.project.project_settings.modules.rdp = true;
							}
							ImGui::PopID();
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip(
									"RDP module is disabled. Click here to enable it.");
							}
							ImGui::SameLine();
						}

						render_badge(GetAssetTypeName(asset.GetType()).c_str(),
									 ImVec4(.4f, .8f, .4f, 0.7f));
						ImGui::SameLine();

						bool selected = app.state.asset_selected.Ref().image &&
										name == (*app.state.asset_selected.Ref().image)->name;
						if (ImGui::Selectable(asset.GetName().c_str(), selected,
											  ImGuiSelectableFlags_AllowDoubleClick)) {
							app.state.asset_selected.Ref(IMAGE, asset.GetAssetReference());
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}
						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
							app.state.asset_selected.Ref(IMAGE, asset.GetAssetReference());
							ImGui::OpenPopup("PopupSpritesBrowserImage");
						}
						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().image)->DrawTooltip();
						}
					}
				}
			} break;
			case UNKNOWN: {
				ImGui::TextColored(ImVec4(.4f, .4f, .4f, 1.f), "%s",
								   GetAssetTypeName(asset.GetType()).c_str());

				ImGui::SameLine();
				if (ImGui::Selectable(asset.GetName().c_str())) {
					// do whatever
				}
			} break;
			case SOUND: {
				if (display_sounds) {
					std::string name = asset.GetName();
					if (name.find(assets_name_filter) != name.npos) {
						if (!app.project.project_settings.modules.audio) {
							ImVec2 uv0, uv1;
							app.GetImagePosition("Error_Icon.png", uv0, uv1);
							ImGui::PushID(1);
							if (ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture),
												   ImVec2(9, 9), uv0, uv1)) {
								app.project.project_settings.modules.audio = true;
							}
							ImGui::PopID();
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip(
									"Audio module is disabled. Click here to enable it.");
							}
							ImGui::SameLine();
						} else if (!app.project.project_settings.modules.audio_mixer) {
							ImVec2 uv0, uv1;
							app.GetImagePosition("Warning_Icon.png", uv0, uv1);
							ImGui::PushID(1);
							if (ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture),
												   ImVec2(9, 9), uv0, uv1)) {
								app.project.project_settings.modules.audio_mixer = true;
							}
							ImGui::PopID();
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip(
									"Audio Mixer module is disabled. Click here to enable it.");
							}
							ImGui::SameLine();
						}

						render_badge(GetAssetTypeName(asset.GetType()).c_str(),
									 ImVec4(.4f, .4f, 1.f, 0.7f));
						ImGui::SameLine();

						bool selected = app.state.asset_selected.Type() == SOUND &&
										name == (*app.state.asset_selected.Ref().sound)->name;
						if (ImGui::Selectable(asset.GetName().c_str(), selected,
											  ImGuiSelectableFlags_AllowDoubleClick)) {
							app.state.asset_selected.Ref(SOUND, asset.GetAssetReference());
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
							app.state.asset_selected.Ref(SOUND, asset.GetAssetReference());
							ImGui::OpenPopup("PopupSoundsBrowserSound");
						}

						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().sound)->DrawTooltip();
						}
					}
				}
			} break;
			case GENERAL: {
				if (display_files) {
					std::string name = (*asset.GetAssetReference().file)->GetFilename();
					if (name.find(assets_name_filter) != name.npos) {
						render_badge(GetAssetTypeName(asset.GetType()).c_str(),
									 ImVec4(1.f, .4f, .4f, 0.7f));
						ImGui::SameLine();

						bool selected = app.state.asset_selected.Type() == GENERAL &&
										name ==
											(*app.state.asset_selected.Ref().file)->GetFilename();
						if (ImGui::Selectable(name.c_str(), selected,
											  ImGuiSelectableFlags_AllowDoubleClick)) {
							app.state.asset_selected.Ref(GENERAL, asset.GetAssetReference());
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
							app.state.asset_selected.Ref(GENERAL, asset.GetAssetReference());
							ImGui::OpenPopup("PopupContentsBrowserContent");
						}

						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().file)->DrawTooltip();
						}
					}
				}
			} break;
		}
	}
}

void render_asset_folder_grid(App &app, Asset *folder) {
	set_up_popup_windows(app);

	ImGui::PushID(folder->GetName().c_str());
	for (auto &asset : folder->children) {
		switch (asset.GetType()) {
			case FOLDER:
				render_asset_folder_grid(app, &asset);
				break;
			case IMAGE: {
				if (display_sprites) {
					std::string name = asset.GetName();
					if (name.find(assets_name_filter) != name.npos) {
						ImGui::TableNextColumn();

						bool selected = app.state.asset_selected.Ref().image &&
										name == (*app.state.asset_selected.Ref().image)->name;

						ImGui::PushID(asset.GetName().c_str());
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						if (app.engine_settings.GetTheme() == THEME_LIGHT) {
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, .1f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, .1f));
						} else {
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, .1f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, .1f));
						}
						if (ImGui::ImageButton(
								(ImTextureID)(intptr_t)((*asset.GetAssetReference().image)
															->loaded_image),
								ImVec2(80, 80))) {
							app.state.asset_selected.Ref(IMAGE, asset.GetAssetReference());
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}
						ImGui::PopStyleColor(3);
						ImGui::PopID();

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}
						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
							app.state.asset_selected.Ref(IMAGE, asset.GetAssetReference());
							ImGui::OpenPopup("PopupSpritesBrowserImage");
						}
						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().image)->DrawTooltip();
						}

						if (selected) {
							ImGui::SameLine(8);
							ImGui::Checkbox("##", &selected);
						}

						if (!app.project.project_settings.modules.display) {
							ImGui::SameLine(73);
							ImVec2 uv0, uv1;
							app.GetImagePosition("Error_Icon.png", uv0, uv1);
							ImGui::PushID(0);
							ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture),
											   ImVec2(15, 15), uv0, uv1);
							ImGui::PopID();
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("Display module is disabled.");
							}
						} else if (!app.project.project_settings.modules.rdp) {
							ImGui::SameLine(73);
							ImVec2 uv0, uv1;
							app.GetImagePosition("Warning_Icon.png", uv0, uv1);
							ImGui::PushID(0);
							ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture),
											   ImVec2(15, 15), uv0, uv1);
							ImGui::PopID();
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("RDP module is disabled.");
							}
						}

						ImGui::TextWrapped("%s", name.c_str());
					}
				}
			} break;
			case UNKNOWN: {
				ImGui::TableNextColumn();
				ImGui::TextColored(ImVec4(.4f, .4f, .4f, 1.f), "%s",
								   GetAssetTypeName(asset.GetType()).c_str());

				ImGui::SameLine();
				if (ImGui::Selectable(asset.GetName().c_str())) {
					// do whatever
				}
			} break;
			case SOUND: {
				if (display_sounds) {
					std::string name = asset.GetName();
					if (name.find(assets_name_filter) != name.npos) {
						ImGui::TableNextColumn();

						bool selected = app.state.asset_selected.Type() == SOUND &&
										name == (*app.state.asset_selected.Ref().sound)->name;

						ImVec2 uv0, uv1;
						app.GetImagePosition("Song.png", uv0, uv1);
						ImGui::PushID(asset.GetName().c_str());
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						if (app.engine_settings.GetTheme() == THEME_LIGHT) {
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, .1f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, .1f));
						} else {
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, .1f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, .1f));
						}
						if (ImGui::ImageButton((ImTextureID)(intptr_t)((app.app_texture)),
											   ImVec2(50, 50), uv0, uv1, 18)) {
							app.state.asset_selected.Ref(SOUND, asset.GetAssetReference());
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}
						ImGui::PopStyleColor(3);
						ImGui::PopID();

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
							app.state.asset_selected.Ref(SOUND, asset.GetAssetReference());
							ImGui::OpenPopup("PopupSoundsBrowserSound");
						}

						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().sound)->DrawTooltip();
						}

						if (selected) {
							ImGui::SameLine(8);
							ImGui::Checkbox("##", &selected);
						}

						if (!app.project.project_settings.modules.audio) {
							ImGui::SameLine(73);
							ImVec2 uv0, uv1;
							app.GetImagePosition("Error_Icon.png", uv0, uv1);
							ImGui::PushID(0);
							ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture),
											   ImVec2(15, 15), uv0, uv1);
							ImGui::PopID();
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("Audio module is disabled.");
							}
						} else if (!app.project.project_settings.modules.audio_mixer) {
							ImGui::SameLine(73);
							ImVec2 uv0, uv1;
							app.GetImagePosition("Warning_Icon.png", uv0, uv1);
							ImGui::PushID(0);
							ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture),
											   ImVec2(15, 15), uv0, uv1);
							ImGui::PopID();
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("Audio Mixer module is disabled.");
							}
						}

						ImGui::TextWrapped("%s", name.c_str());
					}
				}
			} break;
			case GENERAL: {
				if (display_files) {
					std::string name = (*asset.GetAssetReference().file)->GetFilename();
					if (name.find(assets_name_filter) != name.npos) {
						ImGui::TableNextColumn();

						bool selected = app.state.asset_selected.Type() == GENERAL &&
										name ==
											(*app.state.asset_selected.Ref().file)->GetFilename();

						ImVec2 uv0, uv1;
						app.GetImagePosition("File.png", uv0, uv1);
						ImGui::PushID(asset.GetName().c_str());
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						if (app.engine_settings.GetTheme() == THEME_LIGHT) {
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, .1f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, .1f));
						} else {
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, .1f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, .1f));
						}
						if (ImGui::ImageButton((ImTextureID)(intptr_t)((app.app_texture)),
											   ImVec2(50, 50), uv0, uv1, 18)) {
							app.state.asset_selected.Ref(GENERAL, asset.GetAssetReference());
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}
						ImGui::PopStyleColor(3);
						ImGui::PopID();

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							app.state.asset_editing = app.state.asset_selected;
							app.state.reload_asset_edit = true;
						}

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
							app.state.asset_selected.Ref(GENERAL, asset.GetAssetReference());
							ImGui::OpenPopup("PopupContentsBrowserContent");
						}

						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().file)->DrawTooltip();
						}

						if (selected) {
							ImGui::SameLine(8);
							ImGui::Checkbox("##", &selected);
						}

						ImGui::TextWrapped("%s", name.c_str());
					}
				}
			} break;
		}
	}
	ImGui::PopID();
}

void render_asset_details_window(App &app) {
	ImVec2 position = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();

	const float center_y_offset = is_output_open ? 200 : 19;
	position.y = (float)window_height - details_window_size - center_y_offset;
	size.y = details_window_size;

	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(position);
	switch (app.state.asset_editing.Type()) {
		case UNKNOWN:
		case FOLDER:
			break;
		case IMAGE: {
			static char image_edit_name[50];
			static char image_edit_dfs_folder[100];
			static int image_edit_h_slices = 0;
			static int image_edit_v_slices = 0;
			if (app.state.reload_asset_edit) {
				app.state.reload_asset_edit = false;

				strcpy(image_edit_name, (*app.state.asset_editing.Ref().image)->name.c_str());
				strcpy(image_edit_dfs_folder,
					   (*app.state.asset_editing.Ref().image)->dfs_folder.c_str());
				image_edit_h_slices = (*app.state.asset_editing.Ref().image)->h_slices;
				image_edit_v_slices = (*app.state.asset_editing.Ref().image)->v_slices;
			}
			if (ImGui::Begin("Details", nullptr,
							 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				ImGui::InputText("Name", image_edit_name, 50, ImGuiInputTextFlags_CharsFilePath);
				ImGui::InputText("DFS Folder", image_edit_dfs_folder, 100,
								 ImGuiInputTextFlags_CharsFilePath);
				ImGui::InputInt("H Slices", &image_edit_h_slices);
				ImGui::InputInt("V Slices", &image_edit_v_slices);

				ImGui::Separator();
				ImGui::Spacing();

				if (ImGui::Button("Save")) {
					bool will_save = true;
					if ((*app.state.asset_editing.Ref().image)->name != image_edit_name) {
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
							std::string extension = get_libdragon_image_type_extension(
								(*app.state.asset_editing.Ref().image)->type);

							std::filesystem::copy_file(
								app.project.project_settings.project_directory + "/" +
									(*app.state.asset_editing.Ref().image)->image_path,
								app.project.project_settings.project_directory +
									"/assets/sprites/" + image_edit_name + extension);
							(*app.state.asset_editing.Ref().image)
								->DeleteFromDisk(app.project.project_settings.project_directory);
						}
					}

					if (will_save) {
						std::string extension = get_libdragon_image_type_extension(
							(*app.state.asset_editing.Ref().image)->type);

						(*app.state.asset_editing.Ref().image)->name = image_edit_name;
						(*app.state.asset_editing.Ref().image)->dfs_folder = image_edit_dfs_folder;
						(*app.state.asset_editing.Ref().image)->h_slices = image_edit_h_slices;
						(*app.state.asset_editing.Ref().image)->v_slices = image_edit_v_slices;
						(*app.state.asset_editing.Ref().image)
							->image_path = "assets/sprites/" +
										   (*app.state.asset_editing.Ref().image)->name + extension;

						(*app.state.asset_editing.Ref().image)
							->SaveToDisk(app.project.project_settings.project_directory);

						std::sort(app.project.images.begin(), app.project.images.end(),
								  libdragon_image_comparison);

						app.project.ReloadAssets();
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					app.state.asset_editing.Reset();
					app.state.asset_selected.Reset();
				}
			}
			ImGui::End();
		} break;
		case SOUND: {
			static char sound_edit_name[50];
			static char sound_edit_dfs_folder[100];
			if (app.state.reload_asset_edit) {
				app.state.reload_asset_edit = false;

				strcpy(sound_edit_name, (*app.state.asset_editing.Ref().sound)->name.c_str());
				strcpy(sound_edit_dfs_folder,
					   (*app.state.asset_editing.Ref().sound)->dfs_folder.c_str());

				if ((*app.state.asset_editing.Ref().sound)->type == SOUND_WAV) {
					std::string sound_path = app.project.project_settings.project_directory + "/" +
											 (*app.state.asset_editing.Ref().sound)->sound_path;

					if (app.audio_state == SS_PLAYING || app.audio_state == SS_PAUSED) {
						Mix_HaltChannel(-1);
						app.audio_state = SS_STOPPED;
					}
					if (app.audio_sample) {
						Mix_FreeChunk(app.audio_sample);
					}

					app.audio_sample = Mix_LoadWAV(sound_path.c_str());
					if (!app.audio_sample) {
						console.AddLog("[error] Unable to load wave file: %s\n",
									   sound_path.c_str());
					}
				}
			}
			if (ImGui::Begin("Details", nullptr,
							 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				// audio preview
				if (app.state.asset_editing.Ref().sound) {
					if ((*app.state.asset_editing.Ref().sound)->type == SOUND_WAV) {
						if (ImGui::Button(app.audio_state == SS_STOPPED ? "Play" : "Restart")) {
							switch (app.audio_state) {
								case SS_STOPPED:
									Mix_PlayChannel(0, app.audio_sample, 0);
									break;
								case SS_PAUSED:
								case SS_PLAYING:
									Mix_HaltChannel(0);
									Mix_PlayChannel(0, app.audio_sample, 0);
									break;
							}
							app.audio_state = SS_PLAYING;
						}
						ImGui::SameLine();
						ImGui::BeginDisabled(app.audio_state == SS_STOPPED);
						if (ImGui::Button(app.audio_state == SS_PAUSED ? "Resume" : "Pause")) {
							if (app.audio_state == SS_PAUSED) {
								Mix_Resume(0);
								app.audio_state = SS_PLAYING;
							} else {
								Mix_Pause(0);
								app.audio_state = SS_PAUSED;
							}
						}
						ImGui::EndDisabled();

						ImGui::SameLine();
						ImGui::BeginDisabled(app.audio_state == SS_STOPPED);
						if (ImGui::Button("Stop")) {
							Mix_HaltChannel(0);
							app.audio_state = SS_STOPPED;
						}
						ImGui::EndDisabled();

						ImGui::SameLine();
						static int volume = MIX_MAX_VOLUME;
						ImGui::SetNextItemWidth(100);
						if (ImGui::SliderInt("Volume", &volume, 0, MIX_MAX_VOLUME)) {
							Mix_Volume(0, volume);
						}
					} else {
						ImGui::TextWrapped("Preview is not supported for this audio type.");
					}

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
				} else if (app.audio_state != SS_STOPPED) {
					Mix_HaltChannel(0);
				}

				ImGui::InputText("Name", sound_edit_name, 50, ImGuiInputTextFlags_CharsFileName);
				ImGui::InputText("DFS Folder", sound_edit_dfs_folder, 100,
								 ImGuiInputTextFlags_CharsFilePath);

				if ((*app.state.asset_editing.Ref().sound)->type == SOUND_WAV) {
					ImGui::Checkbox("Loop", &(*app.state.asset_editing.Ref().sound)->wav_loop);
					if ((*app.state.asset_editing.Ref().sound)->wav_loop) {
						ImGui::SameLine();
						ImGui::SetNextItemWidth(100);
						ImGui::InputInt("Offset",
										&(*app.state.asset_editing.Ref().sound)->wav_loop_offset);
					}
				} else if ((*app.state.asset_editing.Ref().sound)->type == SOUND_YM) {
					ImGui::Checkbox("Compress",
									&(*app.state.asset_editing.Ref().sound)->ym_compress);
				}

				ImGui::Separator();
				ImGui::Spacing();
				if (ImGui::Button("Save")) {
					bool will_save = true;
					if ((*app.state.asset_editing.Ref().sound)->name != sound_edit_name) {
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
									(*app.state.asset_editing.Ref().sound)->sound_path,
								app.project.project_settings.project_directory + "/assets/sounds/" +
									sound_edit_name +
									(*app.state.asset_editing.Ref().sound)->GetExtension());
							(*app.state.asset_editing.Ref().sound)
								->DeleteFromDisk(app.project.project_settings.project_directory);
						}
					}

					if (will_save) {
						(*app.state.asset_editing.Ref().sound)->name = sound_edit_name;
						(*app.state.asset_editing.Ref().sound)->dfs_folder = sound_edit_dfs_folder;
						(*app.state.asset_editing.Ref().sound)
							->sound_path = "assets/sounds/" +
										   (*app.state.asset_editing.Ref().sound)->name +
										   (*app.state.asset_editing.Ref().sound)->GetExtension();

						(*app.state.asset_editing.Ref().sound)
							->SaveToDisk(app.project.project_settings.project_directory);

						std::sort(app.project.sounds.begin(), app.project.sounds.end(),
								  libdragon_sound_comparison);
						app.project.ReloadAssets();
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					app.state.asset_editing.Reset();
					app.state.asset_selected.Reset();
				}
			}
			ImGui::End();
		} break;
		case GENERAL: {
			static char edit_name[50];
			static char edit_dfs_folder[100];
			static bool edit_copy_to_filesystem;

			if (app.state.reload_asset_edit) {
				app.state.reload_asset_edit = false;

				strcpy(edit_name, (*app.state.asset_editing.Ref().file)->name.c_str());
				strcpy(edit_dfs_folder, (*app.state.asset_editing.Ref().file)->dfs_folder.c_str());
				edit_copy_to_filesystem = (*app.state.asset_editing.Ref().file)->copy_to_filesystem;
			}
			if (ImGui::Begin("Details", nullptr,
							 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				ImGui::InputText("Name", edit_name, 50, ImGuiInputTextFlags_CharsFileName);
				ImGui::InputText("DFS Folder", edit_dfs_folder, 100,
								 ImGuiInputTextFlags_CharsFilePath);
				ImGui::Checkbox("Copy to Filesystem", &edit_copy_to_filesystem);

				ImGui::Separator();
				ImGui::Spacing();
				if (ImGui::Button("Save")) {
					bool will_save = true;
					if ((*app.state.asset_editing.Ref().file)->name != edit_name) {
						std::string name_string(edit_name);
						name_string.append((*app.state.asset_editing.Ref().file)->file_type);

						auto find_by_name =
							[&name_string](const std::unique_ptr<LibdragonFile> &i) {
								return i->GetFilename() == name_string;
							};
						if (std::find_if(app.project.general_files.begin(),
										 app.project.general_files.end(),
										 find_by_name) != std::end(app.project.general_files)) {
							console.AddLog(
								"File with the name already exists. Please choose a "
								"different name.");
							will_save = false;
						} else {
							std::filesystem::copy_file(
								app.project.project_settings.project_directory + "/" +
									(*app.state.asset_editing.Ref().file)->file_path,
								app.project.project_settings.project_directory +
									"/assets/general/" + name_string);
							(*app.state.asset_editing.Ref().file)
								->DeleteFromDisk(app.project.project_settings.project_directory);
						}
					}

					if (will_save) {
						(*app.state.asset_editing.Ref().file)->name = edit_name;
						(*app.state.asset_editing.Ref().file)->dfs_folder = edit_dfs_folder;
						(*app.state.asset_editing.Ref().file)
							->copy_to_filesystem = edit_copy_to_filesystem;
						(*app.state.asset_editing.Ref().file)
							->file_path = "assets/general/" +
										  (*app.state.asset_editing.Ref().file)->GetFilename();

						(*app.state.asset_editing.Ref().file)
							->SaveToDisk(app.project.project_settings.project_directory);

						std::sort(app.project.general_files.begin(),
								  app.project.general_files.end(), libdragon_file_comparison);
						app.project.ReloadAssets();
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					app.state.asset_editing.Reset();
					app.state.asset_selected.Reset();
				}
			}
			ImGui::End();
		} break;
	}
}

void render_asset_import_window(App &app) {
	ImGui::SetNextWindowSize(ImVec2(680, 330), ImGuiCond_Once);
	if (ImGuiFileDialog::Instance()->Display("ImportAssetsDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			auto selections = ImGuiFileDialog::Instance()->GetSelection();
			for (auto &selection : selections) {
				AppGui::ProcessImportFile(app, selection.second.c_str());
			}
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void AppGui::RenderContentBrowserNew(App &app) {
	render_asset_import_window(app);

	if (ImGui::Button("Import Assets")) {
		ImGuiFileDialog::Instance()->OpenDialog("ImportAssetsDlgKey", "Choose Files", ".*", ".", 0);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("You can also Drag & Drop files anywhere to import.");
	}
	ImGui::SameLine();
	if (ImGui::Button("Refresh Assets")) {
		app.project.ReloadImages(app.renderer);
		app.project.ReloadSounds();
		app.project.ReloadGeneralFiles();

		app.project.ReloadAssets();
	}
	ImGui::Separator();

	ImGui::InputTextWithHint("##Name", "Filter Assets", assets_name_filter, 100);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::BeginCombo("##", "Filters", ImGuiComboFlags_NoArrowButton)) {
		ImGui::Checkbox("Sprites", &display_sprites);
		ImGui::Checkbox("Sounds", &display_sounds);
		ImGui::Checkbox("Files", &display_files);
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::TextUnformatted("|");
	ImGui::SameLine();

	ImVec2 uv0, uv1;
	app.GetImagePosition("View_List.png", uv0, uv1);
	ImGui::PushID(0);
	if (ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture), ImVec2(15, 15), uv0, uv1)) {
		asset_display_type = ADT_LIST;
	}
	ImGui::PopID();
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("View assets as a list");
	}

	ImGui::SameLine();
	app.GetImagePosition("View_Grid.png", uv0, uv1);
	ImGui::PushID(1);
	if (ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture), ImVec2(15, 15), uv0, uv1)) {
		asset_display_type = ADT_GRID;
	}
	ImGui::PopID();
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("View assets as a grid");
	}

	if (!app.project.project_settings.modules.dfs) {
		ImGui::TextWrapped("DFS MODULE IS NOT LOADED. CONTENT WILL NOT BE USABLE IN THE GAME.");
	}

	const ImVec4 sep_color(.1f, .1f, .1f, 1);
	ImGui::PushStyleColor(ImGuiCol_Separator, sep_color);
	ImGui::Separator();
	ImGui::PopStyleColor();

	if (asset_display_type == ADT_LIST) {
		if (ImGui::TreeNodeEx("Assets", ImGuiTreeNodeFlags_DefaultOpen)) {
			render_asset_folder_list(app, app.project.assets);
			ImGui::TreePop();
		}
	} else if (asset_display_type == ADT_GRID) {
		const int item_size = 100;
		int items_per_line = std::floor(ImGui::GetWindowWidth() / (float)item_size);
		if (items_per_line < 1)
			items_per_line = 1;

		if (ImGui::BeginTable("Assets", items_per_line)) {
			ImGui::TableNextRow();
			render_asset_folder_grid(app, app.project.assets);
			ImGui::EndTable();
		}
	}

	render_asset_details_window(app);
}

void AppGui::RenderContentBrowser(App &app) {
	const float center_x_size = (float)window_width - 620;
	float center_y_offset = is_output_open ? 219 : 38;

	if (app.state.asset_editing.Type() > UNKNOWN) {
		center_y_offset += details_window_size;
	}
	ImGui::SetNextWindowSize(ImVec2(center_x_size, (float)window_height - center_y_offset));
	ImGui::SetNextWindowPos(ImVec2(300, 19));
	if (ImGui::Begin("ContentBrowser", nullptr,
					 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
		if (app.project.project_settings.IsOpen()) {
			if (ImGui::BeginTabBar("CenterContentTabs",
								   ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
				if (ImGui::BeginTabItem("Content Browser")) {
					RenderContentBrowserNew(app);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Scripts")) {
					static char script_name_input[100] = {};
					ImGui::SetNextItemWidth(300);
					bool create_script = ImGui::InputTextWithHint(
						"##ScriptName", "script_name", script_name_input, 100,
						ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsFileName |
							ImGuiInputTextFlags_AutoSelectAll);
					if (create_script) {
						ImGui::SetKeyboardFocusHere(-1);
					}

					ImGui::SameLine();
					if (ImGui::Button("Create Script") || create_script) {
						std::string script_name(script_name_input);
						if (!script_name.empty()) {
							if (ScriptBuilder::CreateScriptFile(app.project.project_settings,
																script_name_input)) {
								memset(script_name_input, 0, 100);

								app.project.ReloadScripts(&app);
							}
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Reload Scripts")) {
						app.project.ReloadScripts(&app);
					}
					ImGui::Separator();

					if (ImGui::BeginPopup("PopupScriptsBrowserScript")) {
						if (ImGui::Selectable("Edit")) {
							if (app.state.selected_script) {
								std::string path = (*app.state.selected_script)->GetFilePath(&app);
								CodeEditor::OpenPath(&app, path);
							}
						}
						if (ImGui::Selectable("Edit Settings")) {
							if (app.state.selected_script) {
								app.state.script_editing = app.state.selected_script;
								app.state.reload_script_edit = true;
							}
						}
						if (ImGui::Selectable("Delete")) {
							if (app.state.selected_script) {
								(*app.state.selected_script)->DeleteFromDisk(&app);

								for (size_t i = 0; i < app.project.script_files.size(); ++i) {
									if (app.project.script_files[i] == *app.state.selected_script) {
										app.project.script_files.erase(
											app.project.script_files.begin() + (int)i);
									}
								}
								app.state.selected_script = nullptr;
							}
						}
						ImGui::EndPopup();
					}

					int cur_i = 0;
					for (auto &script : app.project.script_files) {
						bool selected = app.state.selected_script &&
										script->name == (*app.state.selected_script)->name;
						if (ImGui::Selectable(script->name.c_str(), selected,
											  ImGuiSelectableFlags_AllowDoubleClick)) {
							app.state.selected_script = &script;
						}

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							std::string path = script->GetFilePath(&app);
							CodeEditor::OpenPath(&app, path);
						}

						if (ImGui::IsItemHovered() &&
							ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
							app.state.selected_script = &script;
							ImGui::OpenPopup("PopupScriptsBrowserScript");
						}

						if (ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("%s", script->text.c_str());
							ImGui::EndTooltip();
						}
						++cur_i;
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}
}

void AppGui::RenderSceneWindow(App &app) {
	const float prop_y_size = is_output_open ? 219 : 38;
	ImGui::SetNextWindowSize(ImVec2(300, (float)window_height - prop_y_size));
	ImGui::SetNextWindowPos(ImVec2(0, 19));
	if (ImGui::Begin("Scene", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_NoTitleBar)) {
		if (app.project.project_settings.IsOpen()) {
			{
				ImVec2 button_uv0;
				ImVec2 button_uv1;
				const ImVec2 button_size(19, 20);

				ImGui::PushID(0);
				app.GetImagePosition("Save.png", button_uv0, button_uv1);
				if (ImGui::ImageButton((ImTextureID)(intptr_t)app.app_texture, button_size,
									   button_uv0, button_uv1, -1) ||
					((ImGui::IsKeyDown(ImGuiKey_RightCtrl) ||
					  ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) &&
					 ImGui::IsKeyPressed(ImGuiKey_S, false))) {
					console.AddLog("Saving Project...");

					app.project.SaveToDisk(app.project.project_settings.project_directory);
					app.project.project_settings.SaveToDisk();

					console.AddLog("Project saved.");
				}
				ImGui::PopID();
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Save All [Ctrl+S]");
				}

				ImGui::SameLine();
				ImGui::PushID(5);
				app.GetImagePosition("Editor.png", button_uv0, button_uv1);
				if (ImGui::ImageButton((ImTextureID)(intptr_t)app.app_texture, button_size,
									   button_uv0, button_uv1, -1) ||
					ImGui::IsKeyPressed(ImGuiKey_F9, false)) {
					console.AddLog("Opening project in Editor...");

					CodeEditor::OpenPath(&app, app.project.project_settings.project_directory);
				}
				ImGui::PopID();
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Open in Editor [F9]");
				}

				ImGui::SameLine();
				ImGui::PushID(1);
				app.GetImagePosition("Build.png", button_uv0, button_uv1);
				if (ImGui::ImageButton((ImTextureID)(intptr_t)app.app_texture, button_size,
									   button_uv0, button_uv1, -1) ||
					ImGui::IsKeyPressed(ImGuiKey_F6, false)) {
					console.AddLog("Building Project...");

					ProjectBuilder::Build(&app);
				}
				ImGui::PopID();
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Build [F6]");
				}

				ImGui::SameLine();
				ImGui::PushID(3);
				app.GetImagePosition("Clean_Build.png", button_uv0, button_uv1);
				if (ImGui::ImageButton((ImTextureID)(intptr_t)app.app_texture, button_size,
									   button_uv0, button_uv1, -1) ||
					ImGui::IsKeyPressed(ImGuiKey_F7, false)) {
					console.AddLog("Rebuilding Project...");

					ProjectBuilder::Rebuild(&app);
				}
				ImGui::PopID();
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Clean and Build [F7]");
				}

				ImGui::SameLine();
				ImGui::PushID(2);
				app.GetImagePosition("Run.png", button_uv0, button_uv1);
				if (ImGui::ImageButton((ImTextureID)(intptr_t)app.app_texture, button_size,
									   button_uv0, button_uv1, -1) ||
					((ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
					  ImGui::IsKeyDown(ImGuiKey_RightShift)) &&
					 ImGui::IsKeyPressed(ImGuiKey_F5, false))) {
					Emulator::Run(&app);
				}
				ImGui::PopID();
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Run [Shift+F5]");
				}

				ImGui::SameLine();
				ImGui::PushID(4);
				app.GetImagePosition("Build_Run.png", button_uv0, button_uv1);
				if (ImGui::ImageButton((ImTextureID)(intptr_t)app.app_texture, button_size,
									   button_uv0, button_uv1, -1) ||
					(!ImGui::IsKeyDown(ImGuiKey_LeftShift) &&
					 !ImGui::IsKeyDown(ImGuiKey_RightShift) &&
					 ImGui::IsKeyPressed(ImGuiKey_F5, false))) {
					console.AddLog("Building Project...");

					ProjectBuilder::Build(&app);
					Emulator::Run(&app);
				}
				ImGui::PopID();
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Build and Run [F5]");
				}

				ImGui::Separator();
			}

			if (app.project.project_settings.modules.scene_manager) {
				if (ImGui::BeginTabBar("#ScenesTabBar", ImGuiTabBarFlags_AutoSelectNewTabs)) {
					if (ImGui::BeginTabItem("Scenes")) {
						for (auto &scene : app.project.scenes) {
							bool is_initial_scene = (scene.id ==
													 app.project.project_settings.initial_scene_id);
							if (ImGui::Selectable(scene.name.c_str(),
												  app.state.current_scene &&
													  scene.id == app.state.current_scene->id)) {
								app.state.current_scene = &scene;
								strcpy(app.state.scene_name, app.state.current_scene->name.c_str());
							}
							if (is_initial_scene) {
								ImGui::SameLine(300 - 40);
								render_badge("start", ImVec4(.4f, .8f, .4f, 1.f));
							}
							if (is_initial_scene && ImGui::IsItemHovered()) {
								ImGui::SetTooltip("Initial Scene");
							}
						}

						ImGui::Spacing();
						if (ImGui::Button("New Scene")) {
							app.project.scenes.emplace_back();
							auto scene = &app.project.scenes[app.project.scenes.size() - 1];
							scene->id = app.project.project_settings.next_scene_id++;
							scene->name = std::to_string(app.project.scenes.size());
						}

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
						{
							std::string current_selected("None");
							for (auto &scene : app.project.scenes) {
								if (scene.id == app.project.project_settings.initial_scene_id) {
									current_selected = scene.name;
									break;
								}
							}
							ImGui::TextUnformatted("Initial Scene");
							if (ImGui::BeginCombo("##InitialScene", current_selected.c_str())) {
								for (auto &scene : app.project.scenes) {
									if (ImGui::Selectable(
											scene.name.c_str(),
											scene.id ==
												app.project.project_settings.initial_scene_id)) {
										app.project.project_settings.initial_scene_id = scene.id;
										app.project.project_settings.SaveToDisk();
									}
								}
								ImGui::EndCombo();
							}
						}

						ImGui::EndTabItem();
					}

					if (app.state.current_scene &&
						app.project.project_settings.modules.scene_manager) {
						if (ImGui::BeginTabItem(app.state.current_scene->name.c_str())) {
							if (ImGui::BeginTabBar("Properties",
												   ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
								//				if (ImGui::BeginTabItem("Nodes")) {
								//					if (ImGui::TreeNodeEx("Root Node")) {
								//						{
								//							ImGui::SameLine();
								//							if (ImGui::Selectable("Props")) {
								//								// do whatever
								//							}
								//						}
								//						{
								//							if (ImGui::Selectable("Test Node
								// Press"))
								//{
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
									ImGui::InputText("Name", app.state.scene_name, 100,
													 ImGuiInputTextFlags_CharsFileName);

									ImGui::Spacing();
									ImGui::TextUnformatted("Background Fill Color");
									ImGui::ColorPicker3("##FillColor",
														app.state.current_scene->fill_color);

									ImGui::Spacing();
									ImGui::Separator();
									ImGui::Spacing();
									{
										std::string current_selected(
											app.state.current_scene->script_name);
										ImGui::TextUnformatted("Attached Script");
										if (ImGui::BeginCombo("##AttachedScript",
															  current_selected.c_str())) {
											for (auto &script : app.project.script_files) {
												if (ImGui::Selectable(
														script->name.c_str(),
														script->name == current_selected)) {
													app.state.current_scene
														->script_name = script->name;
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
										app.project.SaveToDisk(
											app.project.project_settings.project_directory);
										app.project.project_settings.SaveToDisk();
									}
									ImGui::SameLine();
									if (ImGui::Button("Cancel")) {
										app.state.current_scene = nullptr;
									}
									ImGui::Spacing();

									ImGui::Separator();
									ImGui::Spacing();
									if (ImGui::Button("Delete Scene")) {
										for (size_t i = 0; i < app.project.scenes.size(); ++i) {
											if (app.project.scenes[i].id ==
												app.state.current_scene->id) {
												app.project.scenes.erase(
													app.project.scenes.begin() + (int)i);
												std::string filename =
													app.project.project_settings.project_directory +
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
							ImGui::EndTabItem();
						}
					}

					ImGui::EndTabBar();
				}
			} else {
				ImGui::TextWrapped("Please enable 'Scene Manager' module to use this feature.");
			}
		}
	}
	ImGui::End();
}

void AppGui::RenderSettingsWindow(App &app) {
	const float prop_x_size = 320;
	const float prop_y_size = is_output_open ? 219 : 38;
	ImGui::SetNextWindowSize(ImVec2(prop_x_size, (float)window_height - prop_y_size));
	ImGui::SetNextWindowPos(ImVec2((float)window_width - prop_x_size, 19));
	if (ImGui::Begin("General Settings", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_NoTitleBar)) {
		if (ImGui::BeginTabBar("Properties", ImGuiTabBarFlags_AutoSelectNewTabs)) {
			if (app.state.script_editing) {
				static char edit_name[50];

				if (app.state.reload_script_edit) {
					app.state.reload_script_edit = false;

					strcpy(edit_name, (*app.state.script_editing)->name.c_str());
				}
				if (ImGui::BeginTabItem("Script Settings")) {
					ImGui::InputText("Name", edit_name, 50, ImGuiInputTextFlags_CharsFileName);

					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Save")) {
						if ((*app.state.script_editing)->name != edit_name) {
							std::string name_string(edit_name);

							auto find_by_name =
								[&name_string](const std::unique_ptr<LibdragonScript> &i) {
									return i->name == name_string;
								};
							if (std::find_if(app.project.script_files.begin(),
											 app.project.script_files.end(),
											 find_by_name) != std::end(app.project.script_files)) {
								console.AddLog(
									"Script with the name already exists. Please choose a "
									"different name.");
							} else {
								(*app.state.script_editing)->RenameAs(&app, name_string);

								std::sort(app.project.script_files.begin(),
										  app.project.script_files.end(),
										  libdragon_script_comparison);
							}
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Cancel")) {
						app.state.script_editing = nullptr;
					}

					ImGui::EndTabItem();
				}
			}
			if (app.project.project_settings.IsOpen() && ImGui::BeginTabItem("Project")) {
				if (app.project.project_settings.IsOpen()) {
					if (ImGui::BeginTabBar("ProjectAllSettings")) {
						if (ImGui::BeginTabItem("General")) {
							ImGui::InputText("Name", app.state.project_settings_screen.project_name,
											 100);
							ImGui::InputText("Rom", app.state.project_settings_screen.rom_name,
											 100);

							const char *save_type_items[] = {"none",	 "eeprom4k", "eeprom16",
															 "sram256k", "sram768k", "sram1m",
															 "flashram"};
							ImGui::Combo("Save Type", &app.state.project_settings_screen.save_type,
										 save_type_items, 7);

							ImGui::Checkbox("Region Free",
											&app.state.project_settings_screen.region_free);

							ImGui::Separator();
							{
								ImGui::TextUnformatted("Custom Content Pipeline");
								if (ImGui::Button("Edit Custom Content Build Script")) {
									std::string path_to_pipeline(
										app.project.project_settings.project_directory +
										"/.ngine/pipeline/");
									if (!std::filesystem::exists(path_to_pipeline)) {
										std::filesystem::create_directories(path_to_pipeline);
									}

									std::string path_to_script(path_to_pipeline +
															   "content_pipeline_end.term");
									if (!std::filesystem::exists(path_to_script)) {
										std::ofstream filestream(path_to_script);
										filestream.close();
									}

									if (!CodeEditor::OpenPath(&app, path_to_script)) {
										console.AddLog("You can edit the file at %s.",
													   path_to_script.c_str());
									}
								}
								if (ImGui::Button("Edit Custom Makefile")) {
									std::string path_to_makefile(
										app.project.project_settings.project_directory +
										"/Makefile_custom.mk");

									if (!CodeEditor::OpenPath(&app, path_to_makefile)) {
										console.AddLog("You can edit the file at %s.",
													   path_to_makefile.c_str());
									}
								}
							}

							ImGui::Separator();
							{
								ImGui::TextUnformatted("Global Script");
								if (ImGui::BeginCombo(
										"##GlobalScript",
										app.project.project_settings.global_script_name.c_str())) {
									for (auto &script : app.project.script_files) {
										if (ImGui::Selectable(
												script->name.c_str(),
												script->name == app.project.project_settings
																	.global_script_name)) {
											app.project.project_settings
												.global_script_name = script->name;
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

						if (ImGui::BeginTabItem("Libdragon")) {
							{
								ImGui::Spacing();
								ImGui::BeginDisabled();
								ImGui::TextUnformatted("Libdragon");
								ImGui::EndDisabled();
								ImGui::Separator();

								if (ImGui::Button("Update Libdragon")) {
									console.AddLog("Running 'libdragon update'...");
									Libdragon::Update(
										app.project.project_settings.project_directory,
										app.engine_settings.GetLibdragonExeLocation());
								}
								ImGui::SameLine();
								ImGui::BeginDisabled();
								ImGui::TextUnformatted("- libdragon update");
								ImGui::EndDisabled();

								if (ImGui::Button("Re-Build Libdragon")) {
									console.AddLog("Running 'libdragon install'...");
									Libdragon::Install(
										app.project.project_settings.project_directory,
										app.engine_settings.GetLibdragonExeLocation());
								}
								ImGui::SameLine();
								ImGui::BeginDisabled();
								ImGui::TextUnformatted("- libdragon install");
								ImGui::EndDisabled();

								//								ImGui::TextUnformatted("Repository
								// URL"); 								char repo_buf[255] = "\0";
								// ImGui::InputText("###Repo", repo_buf, 255);
								// ImGui::SameLine(); ImGui::Button("Update");
								//
								ImGui::Spacing();
								ImGui::TextUnformatted("Branch");
								ImGui::SameLine();
								if (ImGui::Button("Reset")) {
									strcpy(app.state.project_settings_screen.libdragon_branch,
										   "trunk\0");
								}

								ImGui::InputText("###Branch",
												 app.state.project_settings_screen.libdragon_branch,
												 50);
								ImGui::SameLine();
								if (ImGui::Button("Update")) {
									std::string dir(app.project.project_settings.project_directory);
									dir.append("/libdragon");

									char cmd[255];
									snprintf(cmd, 255, "git checkout %s",
											 app.state.project_settings_screen.libdragon_branch);
									ThreadCommand::QueueCommand(cmd);
								}
							}
							{
								//								ImGui::Spacing();
								//								ImGui::BeginDisabled();
								//								ImGui::TextUnformatted("Libdragon
								// Extensions"); ImGui::EndDisabled(); ImGui::Separator();

								//								ImGui::TextUnformatted("Repository
								// URL"); 								char repo_buf[255] = "\0";
								// ImGui::InputText("###Repo", repo_buf, 255);
								// ImGui::SameLine(); ImGui::Button("Update");
								//
								//								ImGui::TextUnformatted("Branch");
								//								char branch_buf[50] = "\0";
								//								ImGui::InputText("###Branch",
								// branch_buf, 50); ImGui::SameLine();
								// ImGui::Button("Update");
							}
							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Modules")) {
							ImGui::TextWrapped(
								"IF YOU CHANGE MODULES, CONSIDER RUNNING THE CLEAN/BUILD TASK.");
							ImGui::Separator();

							ImGui::BeginDisabled();
							ImGui::TextWrapped("Libdragon");
							ImGui::EndDisabled();
							ImGui::Separator();

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

							if (ImGui::Checkbox("Timer",
												&app.project.project_settings.modules.timer)) {
								if (!app.project.project_settings.modules.timer)
									app.project.project_settings.modules.rtc = false;
							}

							ImGui::BeginDisabled(!app.project.project_settings.modules.timer);
							ImGui::Checkbox("Real-Time Clock (RTC)",
											&app.project.project_settings.modules.rtc);
							ImGui::EndDisabled();

							ImGui::Spacing();
							ImGui::BeginDisabled();
							ImGui::TextWrapped("Libdragon Extensions");
							ImGui::EndDisabled();
							ImGui::Separator();

							ImGui::Checkbox("Memory Pool",
											&app.project.project_settings.modules.memory_pool);
							ImGui::Checkbox("Scene Manager",
											&app.project.project_settings.modules.scene_manager);

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

				ImGui::TextUnformatted("Libdragon Exe Path (?)");
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip(
						"We will run the libdragon-docker exe from this path.\n\nYou can use PATH "
						"vars by leaving only the exe name.");
				}
				ImGui::InputTextWithHint("##LibdragonExePath",
										 "/path/to/libdragon/folder/libdragon",
										 app.state.libdragon_exe_path, 255);

				{
					ImGui::TextUnformatted("Theme");
					static int selected_theme = (int)app.engine_settings.GetTheme();
					if (ImGui::Combo("##Theme", &selected_theme,
									 "Dark\0Light\0Classic\0Dark Gray\0")) {
						ChangeTheme(app, (Theme)selected_theme);
					}
				}

				ImGui::Spacing();
				if (ImGui::Button("Save")) {
					app.engine_settings.SetEmulatorPath(app.state.emulator_path);
					app.engine_settings.SetEditorLocation(app.state.editor_path);
					app.engine_settings.SetLibdragonExeLocation(app.state.libdragon_exe_path);
				}

				ImGui::Separator();
				ImGui::Spacing();
				ImGui::TextWrapped("Edit Engine Settings File:");
				ImGui::SameLine();
				if (ImGui::Button("Open File")) {
					CodeEditor::OpenPath(&app, app.engine_settings.GetEngineSettingsFilepath());
				}
				ImGui::SameLine();
				if (ImGui::Button("Reload File")) {
					app.engine_settings.LoadFromDisk();
					app.state.LoadEngineSetings(app.engine_settings);
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
		case THEME_DARKGRAY:
			ImGui::StyleColorsDarkGray();
			break;
	}

	ImGuiStyle *style = &ImGui::GetStyle();
	style->FrameRounding = 3.f;
	style->FrameBorderSize = 1.f;
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