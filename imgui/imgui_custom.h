#pragma once

#include "imgui.h"

extern ImVec4 color_invalid_input;

void render_badge(const char *label, ImVec4 color);
bool link_button(const char *label, const char *url = nullptr);

/**
 * Returns if text is a valid DFS Folder
 */
bool input_text_dfs_folder(char* buf, int buf_size);
