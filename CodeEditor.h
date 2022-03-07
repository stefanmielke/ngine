#pragma once

#include <string>

class App;

class CodeEditor {
   public:
	static bool OpenPath(App *app, const std::string& path);
};

