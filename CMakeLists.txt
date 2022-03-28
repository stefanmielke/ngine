cmake_minimum_required(VERSION 3.21)
project(ngine)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(SOURCES main.cpp ProjectBuilder.cpp Libdragon.cpp CodeEditor.cpp ScriptBuilder.cpp ThreadCommand.cpp Emulator.cpp LibdragonImage.cpp Content.cpp LibdragonSound.cpp App.cpp ImportAssets.cpp Sdl.cpp AppGui.cpp LibdragonFile.cpp)
set(SOURCES ${SOURCES} generated/makefile.gen.cpp generated/setup.gen.cpp generated/change_scene.gen.cpp generated/scene_gen.cpp generated/script_blank.gen.cpp)
set(SOURCES ${SOURCES} settings/DisplaySettings.cpp settings/ProjectSettings.cpp settings/ModulesSettings.cpp settings/EngineSettings.cpp settings/Scene.cpp settings/Project.cpp settings/AudioSettings.cpp settings/AudioMixerSettings.cpp)
set(SOURCES ${SOURCES} static/main.s.cpp static/vscode_c_cpp_properties.cpp static/gitignore.cpp static/clang_format.cpp generated/game.gen.h.cpp static/change_scene.s.h.cpp)

set(SOURCES ${SOURCES} imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui_impl_sdl.cpp imgui/imgui_impl_sdlrenderer.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE imgui)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} GL dl)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)

    find_package(SDL2 REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} SDL2 SDL2_image)
elseif(WIN32)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE D:/a/ngine/ngine/SDL2/SDL2-2.0.20/x86_64-w64-mingw32/include/ D:/a/ngine/ngine/SDL2/SDL2-2.0.20/x86_64-w64-mingw32/include/SDL2/ D:/a/ngine/ngine/SDL2/SDL2_image-2.0.5/x86_64-w64-mingw32/include/)
    target_link_directories(${PROJECT_NAME} PRIVATE D:/a/ngine/ngine/SDL2/SDL2-2.0.20/x86_64-w64-mingw32/lib/ D:/a/ngine/ngine/SDL2/SDL2_image-2.0.5/x86_64-w64-mingw32/lib/)
    target_link_libraries(${PROJECT_NAME} mingw32 SDL2main SDL2 -mwindows SDL2_image)
endif()

add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/montserrat.ttf
        ${CMAKE_CURRENT_BINARY_DIR}/montserrat.ttf
)
