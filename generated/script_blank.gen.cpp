const char *script_blank_gen_h = R"(#pragma once

#include <libdragon.h>

void script_%s_create();
short script_%s_tick();
void script_%s_display(display_context_t disp);
void script_%s_destroy();
)";

const char *script_blank_gen_c = R"(#include "%s.script.h"

#include "../game.gen.h"

/*
Your includes go here. Eg.:
*/
// #include <mem_pool.h>
// #include <menu.h>


/*
You variables go here: Eg.:
*/
// int my_global_variable;
// static Menu my_menu; // static is used so that this variable can only be used inside this file

/**
 * @brief Called once when this script is initialized.
 *
 * @details If used as a global script, will be called once after the game is initialized (will be
 * the last function added) inside the 'setup' function in 'src/setup.gen.c'.
 *
 * If used a scene script, will be called once (and only one time) the scene is activated (inside
 * 'scene_SCENE_NAME_create()').
 *
 * This function should allocate resources that will be used throughout the script (in case of this
 * script being global, you will be able to use these resources throughout the whole game).
 *
 * If you use this as a global script and have the 'Memory Pool' module active, you should consider
 * using the 'global_memory_pool' to allocate instead of 'malloc' by using 'mem_zone_alloc()'
 * available inside 'mem_pool.h'.
 *
 * If you use this as a scene script and have the 'Memory Pool' module active, you should consider
 * using the 'scene_memory_pool' to allocate instead of 'malloc' by using 'mem_zone_alloc()'
 * available inside 'mem_pool.h'. If you use it, you do not need to 'free' your resources, as they
 * will be freed once the scene changes automatically by the 'Scene Manager' module.
 *
 * @remark See https://github.com/stefanmielke/ngine/wiki/Scripts for more information.
 */
void script_%s_create() {
	// you can use 'scene_memory_pool' to allocate memory instead of 'malloc'
}

/**
 * @brief Called every frame to update the state of the game.
 *
 * @details If used as a global script, will be called every frame after updating the controller
 * state inside the 'tick' function in 'src/setup.gen.c'. This will be called before updating the
 * scene.
 *
 * If used as a scene script, will be called every frame (inside 'scene_SCENE_NAME_tick()') right
 * after the global script tick by 'scene_manager_tick()'.
 *
 * @remark See https://github.com/stefanmielke/ngine/wiki/Scripts for more information.
 *
 * @return The scene_id it should change to, or -1 to not change scenes.
 */
short script_%s_tick() {
	// return '>= 0' to change to that screen (the value is the screen id).
	return -1;
}

/**
 * @brief Called every frame to render to the screen.
 *
 * @details If used as a global script, will be called every frame after rendering the scene (so
 * this will render on top of everything else). If you are not using the scene module, you will need
 * to clear the screen buffer by calling 'graphics_fill_screen()'.
 *
 * If used as a scene script, will be called every frame after calling 'graphics_fill_screen()'
 * using the 'Background Fill Color' choosen and resetting the graphics color to white/black by
 * calling 'graphics_set_color(0xfff, 0)'.
 *
 * This function is only called if the 'Display' module is available, and will be discarted
 * otherwise.
 *
 * @param disp Libdragon's Display Context that can be used to render.
 *
 * @remark See https://github.com/stefanmielke/ngine/wiki/Scripts for more information.
 */
void script_%s_display(display_context_t disp) {
	// runs after screen blank and before 'display_show'
}

/**
 * @brief Called once after this script ends.
 *
 * @details If used as a global script, will never be called and will be discarted.
 *
 * If used as a scene script, will be used when the scene ends (and before creating the next scene).
 * This should 'free' any resources that did not use the 'scene_memory_pool' (eg.: any resources
 * allocated with 'malloc').
 *
 * @remark See https://github.com/stefanmielke/ngine/wiki/Scripts for more information.
 */
void script_%s_destroy() {
	// remember to destroy anything that uses malloc here!
})";