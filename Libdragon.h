#pragma once

#include <string>

class Libdragon {
   public:
	static void Init(std::string folder);
	static void InitSync(std::string folder);
	static void Build(std::string folder);
	static void Clean(std::string folder);
	static void CleanSync(std::string folder);
};
