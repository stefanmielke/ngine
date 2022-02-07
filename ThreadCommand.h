#pragma once

#include <string>

class ThreadCommand {
   public:
	static void RunCommand(std::string command);
	static void RunCommandDetached(std::string command);
};
