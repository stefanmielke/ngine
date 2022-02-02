cmake_minimum_required(VERSION 3.21)
project(ngine)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wno-deprecated-enum-enum-conversion)

set(SOURCES main.cpp ProjectBuilder.cpp Libdragon.cpp VSCode.cpp)
set(SOURCES ${SOURCES} generated/makefile.gen.cpp generated/setup.gen.cpp generated/change_scene.gen.cpp static/change_scene.s.h.cpp generated/scene_gen.cpp)
set(SOURCES ${SOURCES} settings/DisplaySettings.cpp settings/ProjectSettings.cpp settings/ModulesSettings.cpp settings/EngineSettings.cpp settings/Scene.cpp settings/Project.cpp)
set(SOURCES ${SOURCES} static/main.s.cpp static/vscode_c_cpp_properties.cpp static/gitignore.cpp static/clang_format.cpp static/game.s.h.cpp)

set(SOURCES ${SOURCES} imgui-SFML/imgui-SFML.cpp)
set(SOURCES ${SOURCES} imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp static/change_scene.s.h.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE imgui imgui-SFML)
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system OpenGL)