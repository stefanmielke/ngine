const char *vs_code_cpp_properties =
	R"({
"configurations": [
	{
		"name": "N64",
		"includePath": [
			"${default}",
			"libs/libdragon-extensions/include",
			"libdragon/include"
		],
		"defines": [],
		"compilerPath": "/usr/bin/gcc",
		"cStandard": "gnu99",
		"cppStandard": "gnu++14",
		"intelliSenseMode": "linux-gcc-x64",
		"configurationProvider": "ms-vscode.makefile-tools"
	}
],
"version": 4
})";