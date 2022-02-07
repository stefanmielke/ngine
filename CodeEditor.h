#pragma once

#include <string>

class App;

class CodeEditor {
   public:
	static void OpenPath(App *app, const std::string& path);
};

