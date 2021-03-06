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
#include "LibdragonLDtkMap.h"
#include "ProjectBuilder.h"
#include "ScriptBuilder.h"
#include "ThreadCommand.h"

static int window_width, window_height;
static bool is_output_open = true;

static bool display_sprites = true, display_sounds = true, display_files = true,
			display_fonts = true, display_maps = true;
static char assets_name_filter[100];

const float details_window_size = 172;

enum AssetsDisplayType { ADT_LIST, ADT_GRID };
static AssetsDisplayType asset_display_type = ADT_LIST;

static bool help_window_unfloader_active = false;

void AppGui::Update(App &app) {
	SDL_GetWindowSize(app.window, &window_width, &window_height);

	RenderMenuBar(app);

	RenderNewProjectWindow(app);
	RenderOpenProjectWindow(app);

	ImportAssets::RenderImportScreen(&app);

	console.Draw("Output", app.window, is_output_open);

	if (app.project.project_settings.IsOpen()) {
		RenderContentBrowser(app);
		RenderSceneWindow(app);
	} else {
		RenderStarterWindow(app);
	}

	RenderSettingsWindow(app);
}

void AppGui::RenderStarterWindow(App &app) {
	const float center_x_size = (float)window_width - 320;
	float center_y_offset = is_output_open ? 219 : 38;

	ImGui::SetNextWindowSize(ImVec2(center_x_size, (float)window_height - center_y_offset));
	ImGui::SetNextWindowPos(ImVec2(0, 19));

	if (ImGui::Begin("Start Page", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.0f, .8f, .0f, 1.f));
		ImGui::TextWrapped("Welcome to NGine");
		ImGui::PopStyleColor();
		if (app.engine_version.is_pre_release) {
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, .0f, .0f, 1.f));
			ImGui::TextWrapped(
				"This is a pre-release version. There might be bugs, and if you find any, don't "
				"hesitate to open a bug on GitHub (and don't forget to add the version number).");
			ImGui::PopStyleColor();
			ImGui::Spacing();
			ImGui::Spacing();
		}

		ImGui::Separator();
		ImGui::Spacing();

		ImGui::TextWrapped("Click on 'File' to create or to open a project.");
		ImGui::Spacing();
		ImGui::TextWrapped("Click on 'Help' to learn more about the engine or its libraries.");
		ImGui::Spacing();
		ImGui::TextWrapped("Click on 'About' to check the version of the tools it uses.");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::TextWrapped(
			"If this is your first time here, take a look on the right sidebar and take your "
			"time configuring the emulator and editor you want to use. You can also change the "
			"theme if you prefer a light one.");

		bool has_libftd_installed = false;
		std::filesystem::path libftd_location;
#ifdef __LINUX__
		libftd_location = "/usr/local/lib/libftd2xx.so";
#else
		libftd_location = getenv("WINDIR") + std::string(R"(\System32\drivers\ftdibus.sys)");
#endif
		has_libftd_installed = std::filesystem::exists(libftd_location);
		bool is_docker_ok = app.engine_settings.GetDockerVersion().starts_with("docker");
		bool is_libdragon_ok = app.engine_settings.GetLibdragonVersion().starts_with("libdragon");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::TextWrapped("Checks:");

		if (is_libdragon_ok) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.0f, .8f, .0f, 1.f));
			ImGui::TextWrapped("Libdragon was found and is running. Version: %s",
							   app.engine_settings.GetLibdragonVersion().c_str());
			ImGui::PopStyleColor();
		} else {
			ImGui::Spacing();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 1, 1));
			ImGui::TextWrapped(
				" Libdragon-cli seems to be configured incorrectly. Set the correct path on the "
				"engine settings on the right panel.");
			ImGui::PopStyleColor();
		}
		if (is_docker_ok) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.0f, .8f, .0f, 1.f));
			ImGui::TextWrapped("Docker was found and is running. Version: %s",
							   app.engine_settings.GetDockerVersion().c_str());
			ImGui::PopStyleColor();
		} else {
			ImGui::Spacing();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.4f, 0.4f, 1.f));
			if (app.engine_settings.GetDockerVersion().starts_with("Docker is not started")) {
#ifdef WIN64
				if (link_button("Docker service is stopped. Please start Docker for Windows "
								"manually or click here.")) {
					ThreadCommand::RunCommandDetached(
						R"(""%PROGRAMFILES%\Docker\Docker\Docker Desktop.exe"")");
				}
#else
				ImGui::TextWrapped("Docker service is stopped. Please start the Docker service.");
#endif
			} else {
				link_button(
					"Docker was not found on your system. Click here to download Docker for your "
					"platform",
					"https://www.docker.com/get-started");
			}
			ImGui::PopStyleColor();
		}
		if (has_libftd_installed) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.0f, .8f, .0f, 1.f));
			ImGui::TextWrapped("UNFLoader drivers were found at '%s'.",
							   libftd_location.parent_path().string().c_str());
			ImGui::PopStyleColor();
		} else {
			ImGui::Spacing();

			if (link_button("To run on console, check out the UNFLoader guide clicking here.")) {
				help_window_unfloader_active = true;
			}
		}
	}
	ImGui::End();
}

void AppGui::ProcessImportFile(App &app, std::string file_path) {
	if (file_path.ends_with(".png") || file_path.ends_with(".bmp") || file_path.ends_with(".jpg") ||
		file_path.ends_with(".pcx") || file_path.ends_with(".tga") ||
		file_path.ends_with(".jpeg")) {
		LibdragonImageType type;
		if (file_path.ends_with(".png"))
			type = IMAGE_PNG;
		else if (file_path.ends_with(".bmp"))
			type = IMAGE_BMP;
		else if (file_path.ends_with(".jpg"))
			type = IMAGE_JPG;
		else if (file_path.ends_with(".jpeg"))
			type = IMAGE_JPEG;
		else if (file_path.ends_with(".pcx"))
			type = IMAGE_PCX;
		else if (file_path.ends_with(".tga"))
			type = IMAGE_TGA;

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

		dropped_image.image_data_overlay = SDL_CreateTexture(
			app.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w * 2, h * 2);
		SDL_SetTextureBlendMode(dropped_image.image_data_overlay, SDL_BLENDMODE_BLEND);

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
	} else if (file_path.ends_with(".ttf")) {
		DroppedFont dropped_font(file_path.c_str());

		std::filesystem::path filepath(file_path);
		std::string filename = filepath.filename().replace_extension().string();
		std::replace(filename.begin(), filename.end(), ' ', '_');

		strcpy(dropped_font.name, filename.c_str());

		dropped_font.font_data = LibdragonFont::LoadTextureFromFont(file_path.c_str(), 16,
																	app.renderer);

		int w, h;
		SDL_QueryTexture(dropped_font.font_data, nullptr, nullptr, &w, &h);

		const float max_size = 300.f;
		if (w > h) {
			dropped_font.height_mult = (float)h / (float)w;
			h = (int)(dropped_font.height_mult * max_size);
			w = (int)max_size;
		} else {
			dropped_font.width_mult = (float)w / (float)h;
			w = (int)(dropped_font.width_mult * max_size);
			h = (int)max_size;
		}

		dropped_font.w = w;
		dropped_font.h = h;

		app.state.dropped_font_files.push_back(dropped_font);

		ImGui::SetWindowFocus("Import Assets");
	} else if (file_path.ends_with(".tmx")) {
		DroppedTiledMap dropped_map(file_path.c_str());

		std::filesystem::path filepath(file_path);
		std::string filename = filepath.filename().replace_extension().string();
		std::replace(filename.begin(), filename.end(), ' ', '_');

		strcpy(dropped_map.name, filename.c_str());

		dropped_map.layers = LibdragonTiledMap::LoadLayers(filepath.string());

		app.state.dropped_tiled_files.push_back(dropped_map);

		ImGui::SetWindowFocus("Import Assets");
	} else if (file_path.ends_with(".ldtk")) {
		DroppedLDtkMap dropped_map(file_path.c_str());

		std::filesystem::path filepath(file_path);
		std::string filename = filepath.filename().replace_extension().string();
		std::replace(filename.begin(), filename.end(), ' ', '_');

		strcpy(dropped_map.name, filename.c_str());

		dropped_map.layers = LibdragonLDtkMap::LoadLayers(filepath.string());

		app.state.dropped_ldtk_files.push_back(dropped_map);

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

void render_help_window_unfloader() {
	if (!help_window_unfloader_active)
		return;

	ImGui::SetNextWindowFocus();
	ImGui::SetNextWindowSize(ImVec2(400, 190));
	if (ImGui::Begin("Help - UNFLoader", &help_window_unfloader_active,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		ImGui::TextWrapped("Help for UNFLoader.");
		ImGui::Separator();

		link_button("More information here",
					"https://github.com/buu342/N64-UNFLoader/tree/master/UNFLoader");
		ImGui::Separator();

		ImGui::TextWrapped("Requirements:");
		ImGui::Spacing();
#ifdef WIN64
		ImGui::TextWrapped(
			"- Windows XP or higher.\n- The Windows version of the FDTI driver. If you are on "
			"Windows XP, be sure you download the XP driver and not the first one.");
		link_button("Download driver here.", "https://ftdichip.com/drivers/d2xx-drivers/");
#else
		ImGui::TextWrapped(
			"- Ubuntu (Haven't tested with others).\n- The relevant FTDI driver for your processor "
			"architecture (Check the README inside the downloaded tar for install instructions).");
		link_button("Download driver here.", "https://ftdichip.com/drivers/d2xx-drivers/");
#endif
	}
	ImGui::End();
}

void AppGui::RenderMenuBar(App &app) {
	render_help_window_unfloader();

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
				Libdragon::Disasm(&app);
				ThreadCommand::QueueCommand("echo ! Assembly output to 'rom.asm' file.");
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			ImGui::MenuItem("Guides", nullptr, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("UNFLoader Help")) {
				help_window_unfloader_active = true;
			}
			ImGui::Separator();

			ImGui::MenuItem("Development Resources", nullptr, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("N64Brew Wiki")) {
				App::OpenUrl("https://n64brew.dev/wiki/Main_Page");
			}
			if (ImGui::MenuItem("N64Brew Discord")) {
				App::OpenUrl("https://discord.gg/WqFgNWf");
			}
			if (ImGui::MenuItem("Awesome N64 Development List")) {
				App::OpenUrl("https://n64.dev/");
			}
			if (ImGui::MenuItem("Libdragon Modules Reference")) {
				App::OpenUrl("https://libdragon.dev/ref/modules.html");
			}
			if (ImGui::MenuItem("N64 Dev Manual - Libultra")) {
				App::OpenUrl("https://ultra64.ca/resources/documentation/");
			}
			ImGui::Separator();

			ImGui::MenuItem("Engine Resources", nullptr, false, false);
			ImGui::Separator();
			if (ImGui::MenuItem("Ngine Wiki")) {
				App::OpenUrl("https://github.com/stefanmielke/ngine/wiki");
			}
			if (ImGui::MenuItem("Libdragon")) {
				App::OpenUrl("https://github.com/DragonMinded/libdragon");
			}
			if (ImGui::MenuItem("Libdragon CLI")) {
				App::OpenUrl("https://github.com/anacierdem/libdragon-docker");
			}
			if (ImGui::MenuItem("Libdragon Extensions")) {
				App::OpenUrl("https://github.com/stefanmielke/libdragon-extensions");
			}
			if (ImGui::MenuItem("SDL2")) {
				App::OpenUrl("https://www.libsdl.org/index.php");
			}
			if (ImGui::MenuItem("SDL2 Image")) {
				App::OpenUrl("https://www.libsdl.org/projects/SDL_image/");
			}
			if (ImGui::MenuItem("SDL2 Mixer")) {
				App::OpenUrl("https://www.libsdl.org/projects/SDL_mixer/");
			}
			if (ImGui::MenuItem("SDL2 TTF")) {
				App::OpenUrl("https://github.com/libsdl-org/SDL_ttf");
			}
			if (ImGui::MenuItem("Docker Install")) {
				App::OpenUrl("https://www.docker.com/get-started");
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("About")) {
			ImGui::MenuItem(app.engine_version.version_string.c_str(), nullptr, false, false);
			if (ImGui::MenuItem(app.engine_settings.GetLibdragonVersion().c_str(), nullptr, false,
								app.engine_settings.GetLibdragonVersion().starts_with("Update"))) {
				App::OpenUrl("https://github.com/anacierdem/libdragon-docker/releases/latest");
			}
			if (ImGui::MenuItem(app.engine_settings.GetDockerVersion().c_str(), nullptr, false,
								!app.engine_settings.GetDockerVersion().starts_with("docker"))) {
				if (app.engine_settings.GetDockerVersion().starts_with("Docker is not started")) {
#ifdef WIN64
					ThreadCommand::RunCommandDetached(
						R"(""%PROGRAMFILES%\Docker\Docker\Docker Desktop.exe"")");
#endif
				} else {
					App::OpenUrl("https://www.docker.com/get-started");
				}
			}
			ImGui::EndMenu();
		}
#ifdef DEBUG
		ImGui::MenuItem("Stats:", nullptr, false, false);

		ImGuiIO *io = &ImGui::GetIO();
		char fps[50];
		snprintf(fps, 50, "%.3f (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
		ImGui::MenuItem(fps, nullptr, false, false);
#endif
		ImGui::EndMainMenuBar();
	}
}

void AppGui::RenderNewProjectWindow(App &app) {
	ImGui::SetNextWindowSize(ImVec2(680, 330), ImGuiCond_Once);
	if (ImGuiFileDialog::Instance()->IsOpened("NewProjectDlgKey")) {
		ImGui::SetNextWindowFocus();
	}

	if (ImGuiFileDialog::Instance()->Display("NewProjectDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			app.project.Close(&app);
			ProjectBuilder::Create(&app, ImGuiFileDialog::Instance()->GetCurrentPath());
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void AppGui::RenderOpenProjectWindow(App &app) {
	ImGui::SetNextWindowSize(ImVec2(680, 330), ImGuiCond_Once);
	if (ImGuiFileDialog::Instance()->IsOpened("OpenProjectDlgKey")) {
		ImGui::SetNextWindowFocus();
		if (ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
			ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false)) {
			app.OpenProject(ImGuiFileDialog::Instance()->GetCurrentPath());
			ImGuiFileDialog::Instance()->Close();
		}
	}
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
				app.state.asset_selected.marked_to_delete = true;
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
				app.state.asset_selected.marked_to_delete = true;
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
				app.state.asset_selected.marked_to_delete = true;
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("PopupFontsBrowserFont")) {
		if (ImGui::Selectable("Edit Settings")) {
			if (app.state.asset_selected.Type() == FONT) {
				app.state.asset_editing = app.state.asset_selected;
				app.state.reload_asset_edit = true;
			}
		}
		if (ImGui::Selectable("Copy DFS Path")) {
			if (app.state.asset_selected.Type() == FONT) {
				std::string dfs_path((*app.state.asset_selected.Ref().font)->dfs_folder +
									 (*app.state.asset_selected.Ref().font)->name + ".font");
				ImGui::SetClipboardText(dfs_path.c_str());
			}
		}
		if (ImGui::Selectable("Delete")) {
			if (app.state.asset_selected.Type() == FONT) {
				app.state.asset_selected.marked_to_delete = true;
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("PopupMapsBrowserTiled")) {
		if (ImGui::Selectable("Edit Settings")) {
			if (app.state.asset_selected.Type() == TILED_MAP) {
				app.state.asset_editing = app.state.asset_selected;
				app.state.reload_asset_edit = true;
			}
		}
		if (app.state.asset_selected.Type() == TILED_MAP) {
			for (auto &layer : (*app.state.asset_selected.Ref().tiled)->layers) {
				ImGui::PushID(layer.name.c_str());
				std::string dfs_label = "Copy '" + layer.name + "' DFS Path";
				if (ImGui::Selectable(dfs_label.c_str())) {
					if (app.state.asset_selected.Type() == TILED_MAP) {
						std::string dfs_path;
						dfs_path.append((*app.state.asset_selected.Ref().tiled)->dfs_folder +
										(*app.state.asset_selected.Ref().tiled)->name + "/" +
										layer.name + ".map");

						ImGui::SetClipboardText(dfs_path.c_str());
					}
				}
				ImGui::PopID();
			}
		}
		if (ImGui::Selectable("Delete")) {
			if (app.state.asset_selected.Type() == TILED_MAP) {
				app.state.asset_selected.marked_to_delete = true;
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("PopupMapsBrowserLDtk")) {
		if (ImGui::Selectable("Edit Settings")) {
			if (app.state.asset_selected.Type() == LDTK_MAP) {
				app.state.asset_editing = app.state.asset_selected;
				app.state.reload_asset_edit = true;
			}
		}
		if (app.state.asset_selected.Type() == LDTK_MAP) {
			for (auto &layer : (*app.state.asset_selected.Ref().ldtk)->layers) {
				ImGui::PushID(layer.name.c_str());
				std::string dfs_label = "Copy '" + layer.name + "' DFS Path";
				if (ImGui::Selectable(dfs_label.c_str())) {
					if (app.state.asset_selected.Type() == LDTK_MAP) {
						std::string dfs_path;
						dfs_path.append((*app.state.asset_selected.Ref().ldtk)->dfs_folder +
										(*app.state.asset_selected.Ref().ldtk)->name + "/" +
										layer.name + ".map");

						ImGui::SetClipboardText(dfs_path.c_str());
					}
				}
				ImGui::PopID();
			}
		}
		if (ImGui::Selectable("Delete")) {
			if (app.state.asset_selected.Type() == LDTK_MAP) {
				app.state.asset_selected.marked_to_delete = true;
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
				if (display_sprites && asset.GetAssetReference().image) {
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
				if (display_sounds && asset.GetAssetReference().sound) {
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
				if (display_files && asset.GetAssetReference().file) {
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
			case FONT: {
				if (display_fonts && asset.GetAssetReference().font) {
					std::string name = asset.GetName();
					if (name.find(assets_name_filter) != name.npos) {
						render_badge(GetAssetTypeName(asset.GetType()).c_str(),
									 ImVec4(.8f, .8f, .4f, 0.7f));
						ImGui::SameLine();

						bool selected = app.state.asset_selected.Ref().font &&
										name == (*app.state.asset_selected.Ref().font)->name;
						if (ImGui::Selectable(asset.GetName().c_str(), selected,
											  ImGuiSelectableFlags_AllowDoubleClick)) {
							app.state.asset_selected.Ref(FONT, asset.GetAssetReference());
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
							app.state.asset_selected.Ref(FONT, asset.GetAssetReference());
							ImGui::OpenPopup("PopupFontsBrowserFont");
						}
						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().font)->DrawTooltip();
						}
					}
				}
			} break;
			case TILED_MAP: {
				if (display_maps && asset.GetAssetReference().tiled) {
					std::string name = (*asset.GetAssetReference().tiled)->name;
					if (name.find(assets_name_filter) != name.npos) {
						render_badge(GetAssetTypeName(asset.GetType()).c_str(),
									 ImVec4(.4f, .1f, .1f, 0.7f));
						ImGui::SameLine();

						bool selected = app.state.asset_selected.Type() == TILED_MAP &&
										name == (*app.state.asset_selected.Ref().tiled)->name;
						if (ImGui::Selectable(name.c_str(), selected,
											  ImGuiSelectableFlags_AllowDoubleClick)) {
							app.state.asset_selected.Ref(TILED_MAP, asset.GetAssetReference());
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
							app.state.asset_selected.Ref(TILED_MAP, asset.GetAssetReference());
							ImGui::OpenPopup("PopupMapsBrowserTiled");
						}

						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().tiled)->DrawTooltip();
						}
					}
				}
			} break;
			case LDTK_MAP: {
				if (display_maps && asset.GetAssetReference().ldtk) {
					std::string name = (*asset.GetAssetReference().ldtk)->name;
					if (name.find(assets_name_filter) != name.npos) {
						render_badge(GetAssetTypeName(asset.GetType()).c_str(),
									 ImVec4(.4f, .1f, .1f, 0.7f));
						ImGui::SameLine();

						bool selected = app.state.asset_selected.Type() == LDTK_MAP &&
										name == (*app.state.asset_selected.Ref().ldtk)->name;
						if (ImGui::Selectable(name.c_str(), selected,
											  ImGuiSelectableFlags_AllowDoubleClick)) {
							app.state.asset_selected.Ref(LDTK_MAP, asset.GetAssetReference());
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
							app.state.asset_selected.Ref(LDTK_MAP, asset.GetAssetReference());
							ImGui::OpenPopup("PopupMapsBrowserLDtk");
						}

						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().ldtk)->DrawTooltip();
						}
					}
				}
			} break;
		}
	}
}

void render_asset_folder_grid(App &app, Asset *folder) {
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

						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.5f, .5f, .5f, 1));
						ImGui::TextWrapped("%s", asset.GetFolder().c_str());
						ImGui::PopStyleColor();
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
							app.GetImagePosition("Warning_Icon.png", uv0, uv1);
							ImGui::PushID(0);
							ImGui::ImageButton((ImTextureID)(intptr_t)(app.app_texture),
											   ImVec2(15, 15), uv0, uv1);
							ImGui::PopID();
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("Audio Mixer module is disabled.");
							}
						}

						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.5f, .5f, .5f, 1));
						ImGui::TextWrapped("%s", asset.GetFolder().c_str());
						ImGui::PopStyleColor();
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

						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.5f, .5f, .5f, 1));
						ImGui::TextWrapped("%s", asset.GetFolder().c_str());
						ImGui::PopStyleColor();
						ImGui::TextWrapped("%s", name.c_str());
					}
				}
			} break;
			case FONT: {
				if (display_fonts) {
					std::string name = asset.GetName();
					if (name.find(assets_name_filter) != name.npos) {
						ImGui::TableNextColumn();

						bool selected = app.state.asset_selected.Ref().font &&
										name == (*app.state.asset_selected.Ref().font)->name;

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
								(ImTextureID)(intptr_t)((*asset.GetAssetReference().font)
															->loaded_image),
								ImVec2(80, 80))) {
							app.state.asset_selected.Ref(FONT, asset.GetAssetReference());
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
							app.state.asset_selected.Ref(FONT, asset.GetAssetReference());
							ImGui::OpenPopup("PopupFontsBrowserFont");
						}
						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().font)->DrawTooltip();
						}

						if (selected) {
							ImGui::SameLine(8);
							ImGui::Checkbox("##", &selected);
						}

						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.5f, .5f, .5f, 1));
						ImGui::TextWrapped("%s", asset.GetFolder().c_str());
						ImGui::PopStyleColor();
						ImGui::TextWrapped("%s", name.c_str());
					}
				}
			} break;
			case TILED_MAP: {
				if (display_maps) {
					std::string name = (*asset.GetAssetReference().tiled)->name;
					if (name.find(assets_name_filter) != name.npos) {
						ImGui::TableNextColumn();

						bool selected = app.state.asset_selected.Type() == TILED_MAP &&
										name == (*app.state.asset_selected.Ref().tiled)->name;

						ImVec2 uv0, uv1;
						app.GetImagePosition("Map.png", uv0, uv1);
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
							app.state.asset_selected.Ref(TILED_MAP, asset.GetAssetReference());
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
							app.state.asset_selected.Ref(TILED_MAP, asset.GetAssetReference());
							ImGui::OpenPopup("PopupMapsBrowserTiled");
						}

						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().tiled)->DrawTooltip();
						}

						if (selected) {
							ImGui::SameLine(8);
							ImGui::Checkbox("##", &selected);
						}

						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.5f, .5f, .5f, 1));
						ImGui::TextWrapped("%s", asset.GetFolder().c_str());
						ImGui::PopStyleColor();
						ImGui::TextWrapped("%s", name.c_str());
					}
				}
			} break;
			case LDTK_MAP: {
				if (display_maps) {
					std::string name = (*asset.GetAssetReference().ldtk)->name;
					if (name.find(assets_name_filter) != name.npos) {
						ImGui::TableNextColumn();

						bool selected = app.state.asset_selected.Type() == LDTK_MAP &&
										name == (*app.state.asset_selected.Ref().ldtk)->name;

						ImVec2 uv0, uv1;
						app.GetImagePosition("Map.png", uv0, uv1);
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
							app.state.asset_selected.Ref(LDTK_MAP, asset.GetAssetReference());
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
							app.state.asset_selected.Ref(LDTK_MAP, asset.GetAssetReference());
							ImGui::OpenPopup("PopupMapsBrowserLDtk");
						}

						if (ImGui::IsItemHovered()) {
							(*asset.GetAssetReference().ldtk)->DrawTooltip();
						}

						if (selected) {
							ImGui::SameLine(8);
							ImGui::Checkbox("##", &selected);
						}

						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(.5f, .5f, .5f, 1));
						ImGui::TextWrapped("%s", asset.GetFolder().c_str());
						ImGui::PopStyleColor();
						ImGui::TextWrapped("%s", name.c_str());
					}
				}
			} break;
		}
	}
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
		case FONT: {
			static char image_edit_name[50];
			static char image_edit_dfs_folder[100];
			static int image_edit_font_size = 0;
			if (app.state.reload_asset_edit) {
				app.state.reload_asset_edit = false;

				strcpy(image_edit_name, (*app.state.asset_editing.Ref().font)->name.c_str());
				strcpy(image_edit_dfs_folder,
					   (*app.state.asset_editing.Ref().font)->dfs_folder.c_str());
				image_edit_font_size = (*app.state.asset_editing.Ref().font)->font_size;
			}
			if (ImGui::Begin("Details", nullptr,
							 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				ImGui::InputText("Name", image_edit_name, 50, ImGuiInputTextFlags_CharsFileName);
				bool dfs_valid = input_text_dfs_folder(image_edit_dfs_folder, 100);
				ImGui::InputInt("Font Size", &image_edit_font_size);

				ImGui::Separator();
				ImGui::Spacing();

				ImGui::BeginDisabled(!dfs_valid);
				if (ImGui::Button("Save")) {
					bool will_save = true;
					if ((*app.state.asset_editing.Ref().font)->name != image_edit_name) {
						std::string name_string(image_edit_name);
						auto find_by_name =
							[&name_string](const std::unique_ptr<LibdragonFont> &i) {
								return i->name == name_string;
							};
						if (std::find_if(app.project.fonts.begin(), app.project.fonts.end(),
										 find_by_name) != std::end(app.project.fonts)) {
							console.AddLog(
								"Font with the name already exists. Please choose a "
								"different name.");
							will_save = false;
						} else {
							std::filesystem::copy_file(
								app.project.project_settings.project_directory + "/" +
									(*app.state.asset_editing.Ref().font)->font_path,
								app.project.project_settings.project_directory + "/assets/fonts/" +
									image_edit_name + ".ttf");
							(*app.state.asset_editing.Ref().font)
								->DeleteFromDisk(app.project.project_settings.project_directory);
						}
					}

					if (will_save) {
						(*app.state.asset_editing.Ref().font)->name = image_edit_name;
						(*app.state.asset_editing.Ref().font)->dfs_folder = image_edit_dfs_folder;
						(*app.state.asset_editing.Ref().font)->font_size = image_edit_font_size;
						(*app.state.asset_editing.Ref().font)
							->font_path = "assets/fonts/" +
										  (*app.state.asset_editing.Ref().font)->name + ".ttf";

						(*app.state.asset_editing.Ref().font)
							->SaveToDisk(app.project.project_settings.project_directory);

						(*app.state.asset_editing.Ref().font)
							->LoadImage(app.project.project_settings.project_directory,
										app.renderer);

						app.project.ReloadAssets();
					}
				}
				ImGui::EndDisabled();

				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					app.state.asset_editing.Reset();
					app.state.asset_selected.Reset();
				}
			}
			ImGui::End();
		} break;
		case IMAGE: {
			auto image = app.state.asset_editing.Ref().image;

			static char image_edit_name[50];
			static char image_edit_dfs_folder[100];
			static int image_edit_h_slices = 0;
			static int image_edit_v_slices = 0;
			if (app.state.reload_asset_edit) {
				app.state.reload_asset_edit = false;

				strcpy(image_edit_name, (*image)->name.c_str());
				strcpy(image_edit_dfs_folder, (*image)->dfs_folder.c_str());
				image_edit_h_slices = (*image)->h_slices;
				image_edit_v_slices = (*image)->v_slices;

				(*image)->RecreateOverlay(app.renderer, image_edit_h_slices, image_edit_v_slices);
			}
			if (ImGui::Begin("Details", nullptr,
							 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				if (ImGui::BeginTable("Assets", 2)) {
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::Image((ImTextureID)(intptr_t)((*image)->loaded_image),
								 ImVec2((*image)->display_width, (*image)->display_height));
					if (ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Image((ImTextureID)(intptr_t)((*image)->loaded_image),
									 ImVec2((*image)->width, (*image)->height));
						ImGui::SamePlace(8);
						ImGui::Image((ImTextureID)(intptr_t)(*image)->loaded_image_overlay,
									 ImVec2((*image)->width, (*image)->height));
						ImGui::EndTooltip();
					}
					ImGui::SamePlace(8);
					ImGui::Image((ImTextureID)(intptr_t)(*image)->loaded_image_overlay,
								 ImVec2((*image)->display_width, (*image)->display_height));

					ImGui::TableNextColumn();
					ImGui::InputText("Name", image_edit_name, 50,
									 ImGuiInputTextFlags_CharsFileName);
					bool dfs_valid = input_text_dfs_folder(image_edit_dfs_folder, 100);

					bool recreate_grid = false;
					if (ImGui::InputInt("H Slices", &image_edit_h_slices)) {
						recreate_grid = true;
					}
					if (ImGui::InputInt("V Slices", &image_edit_v_slices)) {
						recreate_grid = true;
					}
					if (recreate_grid) {
						(*image)->RecreateOverlay(app.renderer, image_edit_h_slices,
												  image_edit_v_slices);
					}

					ImGui::Separator();
					ImGui::Spacing();

					ImGui::BeginDisabled(!dfs_valid);
					if (ImGui::Button("Save")) {
						bool will_save = true;
						if ((*image)->name != image_edit_name) {
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
									(*image)->type);

								std::filesystem::copy_file(
									app.project.project_settings.project_directory + "/" +
										(*image)->image_path,
									app.project.project_settings.project_directory +
										"/assets/sprites/" + image_edit_name + extension);
								(*image)->DeleteFromDisk(
									app.project.project_settings.project_directory);
							}
						}

						if (will_save) {
							std::string extension = get_libdragon_image_type_extension(
								(*app.state.asset_editing.Ref().image)->type);

							(*image)->name = image_edit_name;
							(*image)->dfs_folder = image_edit_dfs_folder;
							(*image)->h_slices = image_edit_h_slices;
							(*image)->v_slices = image_edit_v_slices;
							(*image)->image_path = "assets/sprites/" + (*image)->name + extension;

							(*image)->SaveToDisk(app.project.project_settings.project_directory);

							app.project.ReloadAssets();
						}
					}
					ImGui::EndDisabled();

					ImGui::SameLine();
					if (ImGui::Button("Cancel")) {
						app.state.asset_editing.Reset();
						app.state.asset_selected.Reset();
					}
					ImGui::EndTable();
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
				bool dfs_valid = input_text_dfs_folder(sound_edit_dfs_folder, 100);

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

				ImGui::BeginDisabled(!dfs_valid);
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

						app.project.ReloadAssets();
					}
				}
				ImGui::EndDisabled();

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
				bool dfs_valid = input_text_dfs_folder(edit_dfs_folder, 100);

				ImGui::Checkbox("Copy to Filesystem", &edit_copy_to_filesystem);

				ImGui::Separator();
				ImGui::Spacing();

				ImGui::BeginDisabled(!dfs_valid);
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

						app.project.ReloadAssets();
					}
				}
				ImGui::EndDisabled();

				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					app.state.asset_editing.Reset();
					app.state.asset_selected.Reset();
				}
			}
			ImGui::End();
		} break;
		case TILED_MAP: {
			static char edit_name[50];
			static char edit_dfs_folder[100];

			if (app.state.reload_asset_edit) {
				app.state.reload_asset_edit = false;

				strcpy(edit_name, (*app.state.asset_editing.Ref().tiled)->name.c_str());
				strcpy(edit_dfs_folder, (*app.state.asset_editing.Ref().tiled)->dfs_folder.c_str());
			}
			if (ImGui::Begin("Details", nullptr,
							 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				ImGui::InputText("Name", edit_name, 50, ImGuiInputTextFlags_CharsFileName);
				bool dfs_valid = input_text_dfs_folder(edit_dfs_folder, 100);

				ImGui::Separator();
				ImGui::Spacing();

				ImGui::BeginDisabled(!dfs_valid);
				if (ImGui::Button("Save")) {
					bool will_save = true;
					if ((*app.state.asset_editing.Ref().tiled)->name != edit_name) {
						std::string name_string(edit_name);

						auto find_by_name =
							[&name_string](const std::unique_ptr<LibdragonTiledMap> &i) {
								return i->name == name_string;
							};
						if (std::find_if(app.project.tiled_maps.begin(),
										 app.project.tiled_maps.end(),
										 find_by_name) != std::end(app.project.tiled_maps)) {
							console.AddLog(
								"Tiled Map with the name already exists. Please choose a "
								"different name.");
							will_save = false;
						} else {
							std::filesystem::copy_file(
								app.project.project_settings.project_directory + "/" +
									(*app.state.asset_editing.Ref().tiled)->file_path,
								app.project.project_settings.project_directory +
									"/assets/tiled_maps/" + name_string + ".tmx");
							(*app.state.asset_editing.Ref().tiled)
								->DeleteFromDisk(app.project.project_settings.project_directory);
						}
					}

					if (will_save) {
						(*app.state.asset_editing.Ref().tiled)->name = edit_name;
						(*app.state.asset_editing.Ref().tiled)->dfs_folder = edit_dfs_folder;
						(*app.state.asset_editing.Ref().tiled)
							->file_path = "assets/tiled_maps/" +
										  (*app.state.asset_editing.Ref().tiled)->name + ".tmx";

						(*app.state.asset_editing.Ref().tiled)
							->SaveToDisk(app.project.project_settings.project_directory);

						app.project.ReloadAssets();
					}
				}
				ImGui::EndDisabled();

				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					app.state.asset_editing.Reset();
					app.state.asset_selected.Reset();
				}
			}
			ImGui::End();
		} break;
		case LDTK_MAP: {
			static char edit_name[50];
			static char edit_dfs_folder[100];

			if (app.state.reload_asset_edit) {
				app.state.reload_asset_edit = false;

				strcpy(edit_name, (*app.state.asset_editing.Ref().ldtk)->name.c_str());
				strcpy(edit_dfs_folder, (*app.state.asset_editing.Ref().ldtk)->dfs_folder.c_str());
			}
			if (ImGui::Begin("Details", nullptr,
							 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				ImGui::InputText("Name", edit_name, 50, ImGuiInputTextFlags_CharsFileName);
				bool dfs_valid = input_text_dfs_folder(edit_dfs_folder, 100);

				ImGui::Separator();
				ImGui::Spacing();

				ImGui::BeginDisabled(!dfs_valid);
				if (ImGui::Button("Save")) {
					bool will_save = true;
					if ((*app.state.asset_editing.Ref().ldtk)->name != edit_name) {
						std::string name_string(edit_name);

						auto find_by_name =
							[&name_string](const std::unique_ptr<LibdragonLDtkMap> &i) {
								return i->name == name_string;
							};
						if (std::find_if(app.project.ldtk_maps.begin(), app.project.ldtk_maps.end(),
										 find_by_name) != std::end(app.project.ldtk_maps)) {
							console.AddLog(
								"LDtk Map with the name already exists. Please choose a "
								"different name.");
							will_save = false;
						} else {
							std::filesystem::copy_file(
								app.project.project_settings.project_directory + "/" +
									(*app.state.asset_editing.Ref().ldtk)->file_path,
								app.project.project_settings.project_directory +
									"/assets/ldtk_maps/" + name_string + ".ldtk");
							(*app.state.asset_editing.Ref().ldtk)
								->DeleteFromDisk(app.project.project_settings.project_directory);
						}
					}

					if (will_save) {
						(*app.state.asset_editing.Ref().ldtk)->name = edit_name;
						(*app.state.asset_editing.Ref().ldtk)->dfs_folder = edit_dfs_folder;
						(*app.state.asset_editing.Ref().ldtk)
							->file_path = "assets/ldtk_maps/" +
										  (*app.state.asset_editing.Ref().ldtk)->name + ".ldtk";

						(*app.state.asset_editing.Ref().ldtk)
							->SaveToDisk(app.project.project_settings.project_directory);

						app.project.ReloadAssets();
					}
				}
				ImGui::EndDisabled();

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
		ImGuiFileDialog::Instance()->OpenDialog(
			"ImportAssetsDlgKey", "Choose Files", ".*",
			app.project.project_settings.project_directory + "/", 0);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("You can also Drag & Drop files anywhere to import.");
	}
	ImGui::SameLine();
	if (ImGui::Button("Refresh Assets")) {
		app.project.ReloadImages(app.renderer);
		app.project.ReloadSounds();
		app.project.ReloadGeneralFiles();
		app.project.ReloadFonts(&app);
		app.project.ReloadTiledMaps();
		app.project.ReloadLDtkMaps();

		app.project.ReloadAssets();
	}
	ImGui::Separator();

	ImGui::InputTextWithHint("##Name", "Filter Assets", assets_name_filter, 100);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(65);
	if (ImGui::BeginCombo("##", "Filters")) {
		ImGui::Checkbox("Sprites", &display_sprites);
		ImGui::Checkbox("Sounds", &display_sounds);
		ImGui::Checkbox("Files", &display_files);
		ImGui::Checkbox("Fonts", &display_fonts);
		ImGui::Checkbox("Maps", &display_maps);
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

	separator_light();

	if (app.project.assets) {
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
				set_up_popup_windows(app);
				render_asset_folder_grid(app, app.project.assets);
				ImGui::EndTable();
			}
		}

		render_asset_details_window(app);
	}
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

	if (app.state.asset_selected.marked_to_delete) {
		switch (app.state.asset_selected.Type()) {
			case UNKNOWN:
			case FOLDER:
				break;
			case IMAGE: {
				(*app.state.asset_selected.Ref().image)
					->DeleteFromDisk(app.project.project_settings.project_directory);

				for (size_t i = 0; i < app.project.images.size(); ++i) {
					if (app.project.images[i]->image_path ==
						(*app.state.asset_selected.Ref().image)->image_path) {
						if ((*app.state.asset_selected.Ref().image)->loaded_image) {
							SDL_DestroyTexture(
								(*app.state.asset_selected.Ref().image)->loaded_image);
						}

						app.project.images.erase(app.project.images.begin() + (int)i);
						break;
					}
				}
			} break;
			case SOUND: {
				(*app.state.asset_selected.Ref().sound)
					->DeleteFromDisk(app.project.project_settings.project_directory);

				for (size_t i = 0; i < app.project.sounds.size(); ++i) {
					if (app.project.sounds[i]->sound_path ==
						(*app.state.asset_selected.Ref().sound)->sound_path) {
						app.project.sounds.erase(app.project.sounds.begin() + (int)i);
						break;
					}
				}
			} break;
			case GENERAL: {
				(*app.state.asset_selected.Ref().file)
					->DeleteFromDisk(app.project.project_settings.project_directory);

				for (size_t i = 0; i < app.project.general_files.size(); ++i) {
					if (app.project.general_files[i]->GetFilename() ==
						(*app.state.asset_selected.Ref().file)->GetFilename()) {
						app.project.general_files.erase(app.project.general_files.begin() + (int)i);

						break;
					}
				}
			} break;
			case FONT: {
				(*app.state.asset_selected.Ref().font)
					->DeleteFromDisk(app.project.project_settings.project_directory);

				for (size_t i = 0; i < app.project.fonts.size(); ++i) {
					if (app.project.fonts[i]->font_path ==
						(*app.state.asset_selected.Ref().font)->font_path) {
						if ((*app.state.asset_selected.Ref().font)->loaded_image) {
							SDL_DestroyTexture(
								(*app.state.asset_selected.Ref().font)->loaded_image);
						}

						app.project.fonts.erase(app.project.fonts.begin() + (int)i);
						break;
					}
				}
			} break;
			case TILED_MAP: {
				(*app.state.asset_selected.Ref().tiled)
					->DeleteFromDisk(app.project.project_settings.project_directory);

				for (size_t i = 0; i < app.project.tiled_maps.size(); ++i) {
					if (app.project.tiled_maps[i]->name ==
						(*app.state.asset_selected.Ref().tiled)->name) {
						app.project.tiled_maps.erase(app.project.tiled_maps.begin() + (int)i);

						break;
					}
				}
			} break;
			case LDTK_MAP: {
				(*app.state.asset_selected.Ref().ldtk)
					->DeleteFromDisk(app.project.project_settings.project_directory);

				for (size_t i = 0; i < app.project.ldtk_maps.size(); ++i) {
					if (app.project.ldtk_maps[i]->name ==
						(*app.state.asset_selected.Ref().ldtk)->name) {
						app.project.ldtk_maps.erase(app.project.ldtk_maps.begin() + (int)i);

						break;
					}
				}
			} break;
		}

		app.state.asset_selected.Reset();
		app.state.asset_editing.Reset();
		app.project.ReloadAssets();
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

				ImGui::SameLine();
				ImGui::PushID(6);
				app.GetImagePosition("Run_Console.png", button_uv0, button_uv1);
				if (ImGui::ImageButton((ImTextureID)(intptr_t)app.app_texture, button_size,
									   button_uv0, button_uv1, -1) ||
					ImGui::IsKeyPressed(ImGuiKey_F10, false)) {
					std::string rom_path(app.project.project_settings.project_directory + "/" +
										 app.project.project_settings.rom_name + ".z64");
					std::string unfloader_path(app.GetEngineDirectory() + "/bundles/UNFLoader");

					char cmd[500];
#ifdef __LINUX__
					const char *format =
						"gnome-terminal -- bash -c \"sudo rmmod usbserial\nsudo rmmod "
						"ftdi_sio\nsudo %s -d -r %s\"";
#else
					std::replace(rom_path.begin(), rom_path.end(), '/', '\\');
					std::replace(unfloader_path.begin(), unfloader_path.end(), '/', '\\');
					unfloader_path.append(".exe");

					const char *format = R"(""%s" -d -r "%s"")";
#endif

					snprintf(cmd, 500, format, unfloader_path.c_str(), rom_path.c_str());

#ifdef __LINUX__
					ThreadCommand::RunCommandDetached(cmd);
#else
					system(cmd);
#endif
				}
				ImGui::PopID();
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Run Latest Build on Console [F10]");
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
									ImGui::ColorEdit3("Background Fill Color",
													  app.state.current_scene->fill_color,
													  ImGuiColorEditFlags_NoInputs);

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
				if (ImGui::BeginTabBar("ProjectAllSettings")) {
					if (ImGui::BeginTabItem("General")) {
						ImGui::InputText("Name", app.state.project_settings_screen.project_name,
										 100);
						ImGui::InputText("Rom", app.state.project_settings_screen.rom_name, 100);

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
											script->name ==
												app.project.project_settings.global_script_name)) {
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
								Libdragon::Update(&app);
							}
							ImGui::SameLine();
							ImGui::BeginDisabled();
							ImGui::TextUnformatted("- libdragon update");
							ImGui::EndDisabled();

							if (ImGui::Button("Re-Build Libdragon")) {
								console.AddLog("Running 'libdragon install'...");
								Libdragon::Install(&app);
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
								Libdragon::GitCheckout(
									&app, "libdragon",
									app.state.project_settings_screen.libdragon_branch);
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

						if (ImGui::Checkbox("Audio", &app.project.project_settings.modules.audio)) {
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

						ImGui::Checkbox("Console", &app.project.project_settings.modules.console);
						ImGui::Checkbox("Controller",
										&app.project.project_settings.modules.controller);
						ImGui::Checkbox("Debug Is Viewer",
										&app.project.project_settings.modules.debug_is_viewer);
						ImGui::Checkbox("Debug USB",
										&app.project.project_settings.modules.debug_usb);
						ImGui::Checkbox("DFS", &app.project.project_settings.modules.dfs);

						if (ImGui::Checkbox("Timer", &app.project.project_settings.modules.timer)) {
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
						ImGui::Checkbox("Menu", &app.project.project_settings.modules.menu);
						ImGui::Checkbox("Scene Manager",
										&app.project.project_settings.modules.scene_manager);

						ImGui::EndTabItem();
					}

					static int antialias_current = app.project.project_settings.display.antialias;
					static int bit_depth_current = app.project.project_settings.display.bit_depth;
					static int gamma_current = app.project.project_settings.display.gamma;
					static int resolution_current = app.project.project_settings.display.resolution;

					const char *antialias_items[] = {"ANTIALIAS_OFF", "ANTIALIAS_RESAMPLE",
													 "ANTIALIAS_RESAMPLE_FETCH_NEEDED",
													 "ANTIALIAS_RESAMPLE_FETCH_ALWAYS"};
					const char *bit_depth_items[] = {"DEPTH_16_BPP", "DEPTH_32_BPP"};
					const char *gamma_items[] = {"GAMMA_NONE", "GAMMA_CORRECT",
												 "GAMMA_CORRECT_DITHER"};
					const char *resolution_items[] = {"RESOLUTION_320x240", "RESOLUTION_640x480",
													  "RESOLUTION_256x240", "RESOLUTION_512x480",
													  "RESOLUTION_512x240", "RESOLUTION_640x240"};

					if (ImGui::BeginTabItem("Module Settings")) {
						{
							ImGui::BeginDisabled(!app.project.project_settings.modules.display);
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::TextUnformatted("Display");
							separator_light();

							ImGui::Combo("Antialias", &antialias_current, antialias_items, 4);
							ImGui::Combo("Bit Depth", &bit_depth_current, bit_depth_items, 2);
							ImGui::SliderInt("Buffers",
											 &app.state.project_settings_screen.display_buffers, 1,
											 3);
							ImGui::Combo("Gamma", &gamma_current, gamma_items, 3);
							ImGui::Combo("Resolution", &resolution_current, resolution_items, 6);
							ImGui::EndDisabled();
						}
						{
							ImGui::BeginDisabled(!app.project.project_settings.modules.audio);
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::TextUnformatted("Audio");
							separator_light();

							ImGui::InputInt("Frequency",
											&app.project.project_settings.audio.frequency);
							ImGui::InputInt("Buffers", &app.project.project_settings.audio.buffers);
							ImGui::EndDisabled();
						}
						{
							ImGui::BeginDisabled(!app.project.project_settings.modules.audio_mixer);
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::TextUnformatted("Audio Mixer");
							separator_light();

							ImGui::InputInt("Channels",
											&app.project.project_settings.audio_mixer.channels);
							ImGui::EndDisabled();
						}

						{
							ImGui::BeginDisabled(!app.project.project_settings.modules.memory_pool);
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::TextUnformatted("Memory Pool");
							separator_light();

							ImGui::TextUnformatted("Global Memory Reserve (KB)");
							ImGui::InputInt("##GlobalMem",
											&app.project.project_settings.global_mem_alloc_size, 1,
											1024);
							ImGui::TextUnformatted("Scene Memory Reserve (KB)");
							ImGui::InputInt("##LocalMem",
											&app.project.project_settings.scene_mem_alloc_size, 1,
											1024);
							ImGui::EndDisabled();
						}
						{
							ImGui::BeginDisabled(!app.project.project_settings.modules.menu);
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::TextUnformatted("Menu");
							separator_light();

							ImGui::ColorEdit4("Selected Color",
											  app.project.project_settings.menu.text_selected_color,
											  ImGuiColorEditFlags_NoInputs);
							ImGui::ColorEdit4("Enabled Color",
											  app.project.project_settings.menu.text_enabled_color,
											  ImGuiColorEditFlags_NoInputs);
							ImGui::ColorEdit4("Disabled Color",
											  app.project.project_settings.menu.text_disabled_color,
											  ImGuiColorEditFlags_NoInputs);
							ImGui::ColorEdit4(
								"Text Background Color",
								app.project.project_settings.menu.text_background_color,
								ImGuiColorEditFlags_NoInputs);
							ImGui::ColorEdit4(
								"Out of Bounds Color",
								app.project.project_settings.menu.text_out_of_bounds_color,
								ImGuiColorEditFlags_NoInputs);
							ImGui::ColorEdit4(
								"Menu Background Color",
								app.project.project_settings.menu.menu_background_color,
								ImGuiColorEditFlags_NoInputs);
							ImGui::EndDisabled();
						}

						ImGui::EndTabItem();
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

						SDL_SetWindowTitle(app.window,
										   ("NGine - " + app.project.project_settings.project_name +
											" - " + app.project.project_settings.project_directory)
											   .c_str());

						console.AddLog("Saved Project Settings.");
					}
				}
				ImGui::EndTabBar();
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
				ImGui::Checkbox("Use bundled", &app.state.libdragon_use_bundled);
				ImGui::BeginDisabled(app.state.libdragon_use_bundled);
				ImGui::InputTextWithHint("##LibdragonExePath",
										 "/path/to/libdragon/folder/libdragon",
										 app.state.libdragon_exe_path, 255);
				ImGui::EndDisabled();

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
					app.engine_settings.SetLibdragonExeLocation(
						&app, app.state.libdragon_use_bundled, app.state.libdragon_exe_path);
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
					app.engine_settings.LoadFromDisk(&app);
					app.state.LoadEngineSetings(app.engine_settings);
				}
				if (ImGui::Button("Open Engine Folder")) {
					App::OpenUrl(app.GetEngineDirectory().c_str());
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
