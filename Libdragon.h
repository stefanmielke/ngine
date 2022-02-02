#pragma once

#include <string>

class Libdragon {
   public:
	static void Init(std::string folder);
	static void Build(std::string folder);
	static void Rebuild(std::string folder);
};
