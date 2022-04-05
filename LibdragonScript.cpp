#include "LibdragonScript.h"

#include <fstream>

#include "json.hpp"

#include "App.h"
#include "ScriptBuilder.h"

std::string LibdragonScript::GetFilePath(App *app) {
	return app->project.project_settings.project_directory + "/src/scripts/" + GetFileName();
}

void LibdragonScript::LoadText(App *app) {
	std::ifstream filestream(GetFilePath(app));

	std::ostringstream sstr;
	sstr << filestream.rdbuf();
	text = sstr.str();
	text = text.substr(0, std::min(1000, (int)text.length()));
	filestream.close();
}

void LibdragonScript::LoadFromDisk(const std::string &filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
}

void LibdragonScript::DeleteFromDisk(App *app) const {
	ScriptBuilder::DeleteScriptFile(app, name.c_str());
}