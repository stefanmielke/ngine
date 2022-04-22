#include "Libdragon.h"

#include "App.h"
#include "ThreadCommand.h"

std::string get_libdragon_exe_location(const App *app) {
	if (app->engine_settings.GetLibdragonUseBundled())
		return "\"" + app->GetEngineDirectory() + "/bundle/libdragon\"";

	return "\"" + app->engine_settings.GetLibdragonExeLocation() + "\"";
}

bool Libdragon::InitSync(const App *app) {
	char command[500];
	snprintf(command, 500, "%s init", get_libdragon_exe_location(app).c_str());
	return ThreadCommand::RunCommand(command) == EXIT_SUCCESS;
}

void Libdragon::Build(const App *app) {
	char command[500];
	snprintf(command, 500, "%s make -j", get_libdragon_exe_location(app).c_str());
	ThreadCommand::QueueCommand(command);

	ThreadCommand::QueueCommand("echo ! Build Completed.");
}

void Libdragon::Clean(const App *app) {
	char command[500];
	snprintf(command, 500, "%s exec make clean", get_libdragon_exe_location(app).c_str());
	ThreadCommand::QueueCommand(command);
}

void Libdragon::CleanSync(const App *app) {
	char command[500];
	snprintf(command, 500, "%s exec make clean", get_libdragon_exe_location(app).c_str());
	ThreadCommand::RunCommand(command);
}

void Libdragon::Update(const App *app) {
	char command[500];
	snprintf(command, 500, "%s update", get_libdragon_exe_location(app).c_str());
	ThreadCommand::QueueCommand(command);
}

void Libdragon::Install(const App *app) {
	char command[500];
	snprintf(command, 500, "%s install", get_libdragon_exe_location(app).c_str());
	ThreadCommand::QueueCommand(command);
}

void Libdragon::Disasm(const App *app) {
	char command[500];
	snprintf(command, 500, "%s disasm > rom.asm", get_libdragon_exe_location(app).c_str());
	ThreadCommand::QueueCommand(command);
}

void Libdragon::Exec(const App *app, const std::string &command) {
	char cmd[500];
	snprintf(cmd, 500, "%s exec %s", get_libdragon_exe_location(app).c_str(), command.c_str());
	ThreadCommand::QueueCommand(cmd);
}

std::string Libdragon::GetVersion(const App *app) {
	char command[500];
	snprintf(command, 500, "%s version", get_libdragon_exe_location(app).c_str());

	std::string result;
	if (ThreadCommand::RunCommand(command, result) != EXIT_SUCCESS) {
		return "Update Libdragon-CLI to see version";
	} else {
		return result;
	}
}

void Libdragon::GitCheckout(const App *app, const std::string &checkout_folder_relative,
							const std::string &branch) {
	char cmd[255];
	snprintf(cmd, 255, "%s exec bash -c 'cd %s; git checkout %s'",
			 get_libdragon_exe_location(app).c_str(), checkout_folder_relative.c_str(),
			 branch.c_str());
	ThreadCommand::QueueCommand(cmd);
}

bool Libdragon::GitSubmoduleAddSync(const App *app, const std::string &submodule_uri,
									const std::string &submodule_folder) {
	char cmd[500];
	snprintf(cmd, 500, "%s exec git submodule add %s %s", get_libdragon_exe_location(app).c_str(),
			 submodule_uri.c_str(), submodule_folder.c_str());

	return ThreadCommand::RunCommand(cmd) == EXIT_SUCCESS;
}

unsigned int Libdragon::GetColor3(const float color[3], BitDepth bit_depth) {
	unsigned int fill_color;
	if (bit_depth == DEPTH_16_BPP) {
		auto sr = (unsigned char)(255 * color[0]);
		auto sg = (unsigned char)(255 * color[1]);
		auto sb = (unsigned char)(255 * color[2]);

		int r = sr >> 3;
		int g = sg >> 3;
		int b = sb >> 3;
		int a = 255;

		fill_color = ((r & 0x1F) << 11) | ((g & 0x1F) << 6) | ((b & 0x1F) << 1) | (a >> 7);
		fill_color = fill_color | (fill_color << 16);
	} else {
		auto sr = (unsigned char)(255 * color[0]);
		auto sg = (unsigned char)(255 * color[1]);
		auto sb = (unsigned char)(255 * color[2]);
		fill_color = (sr << 24) | (sg << 16) | (sb << 8) | ((unsigned char)255);
	}

	return fill_color;
}

unsigned int Libdragon::GetColor4(const float color[4], BitDepth bit_depth) {
	unsigned int fill_color;
	if (bit_depth == DEPTH_16_BPP) {
		auto sr = (unsigned char)(255 * color[0]);
		auto sg = (unsigned char)(255 * color[1]);
		auto sb = (unsigned char)(255 * color[2]);
		auto sa = (unsigned char)(255 * color[3]);

		int r = sr >> 3;
		int g = sg >> 3;
		int b = sb >> 3;
		int a = sa;

		fill_color = ((r & 0x1F) << 11) | ((g & 0x1F) << 6) | ((b & 0x1F) << 1) | (a >> 7);
		fill_color = fill_color | (fill_color << 16);
	} else {
		auto sr = (unsigned char)(255 * color[0]);
		auto sg = (unsigned char)(255 * color[1]);
		auto sb = (unsigned char)(255 * color[2]);
		auto sa = (unsigned char)(255 * color[3]);

		fill_color = (sr << 24) | (sg << 16) | (sb << 8) | sa;
	}

	return fill_color;
}
