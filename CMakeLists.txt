cmake_minimum_required(VERSION 3.21)
project(ngine)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wno-deprecated-enum-enum-conversion)

set(SOURCES main.cpp console_app.h)
set(SOURCES ${SOURCES} imgui-SFML/imgui-SFML.cpp)
set(SOURCES ${SOURCES} imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE imgui imgui-SFML)
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system OpenGL)