#pragma once

#include <string>

class Libdragon {
   public:
	static void Init(const std::string &folder, const std::string &libdragon_exe_folder);
	static bool InitSync(const std::string &folder, const std::string &libdragon_exe_folder);
	static void Build(const std::string &folder, const std::string &libdragon_exe_folder);
	static void Clean(const std::string &folder, const std::string &libdragon_exe_folder);
	static void CleanSync(const std::string &folder, const std::string &libdragon_exe_folder);
	static void Update(const std::string &folder, const std::string &libdragon_exe_folder);
	static void Install(const std::string &folder, const std::string &libdragon_exe_folder);
	static void Disasm(const std::string &folder, const std::string &libdragon_exe_folder);
	static std::string GetVersion(const std::string &libdragon_exe_folder);

	static void GitCheckout(const std::string &libdragon_exe_folder,
							const std::string &checkout_folder_relative, const std::string &branch);
	static bool GitSubmoduleAddSync(const std::string &libdragon_exe_folder,
									const std::string &submodule_uri,
									const std::string &submodule_folder);
};
