# NGine - N64 Engine Powered by Libdragon

#### Engine to create N64 games powered by [Libdragon](https://github.com/DragonMinded/libdragon).

### Requirements

- [SDL2](https://www.libsdl.org/index.php) (bundled on Windows)
- [SDL2_Image](https://www.libsdl.org/projects/SDL_image/) (bundled on Windows)
- [Docker](https://www.docker.com/get-started)
- [Libdragon CLI](https://github.com/anacierdem/libdragon-docker)

### What it is

A tool that generates C code based on the modules selected, and that can import sprites and sounds into the game.

It can also build the game, as well as run it on an emulator (you can configure which).

### What it is NOT

Unity or UE4, where you can create the game using another language. It also (for now) does not have a way to create
nodes nor supports [ECS](https://en.wikipedia.org/wiki/Entity_component_system) directly (but you can add those yourself
if you want).

Also Libdragon still has no support for 3D (but should have soon), so that is also not provided.

### How to use it

Refer to this project's [wiki](https://github.com/stefanmielke/ngine/wiki) for more information (under construction).

### Functionalities Supported

- Input
- Audio / Mixer
- Display / RDP
- Console
- Libdragon's Debug Modules
- Scene Manager (from [Libdragon-Extensions](https://github.com/stefanmielke/libdragon-extensions))
- Memory Pool (from [Libdragon-Extensions](https://github.com/stefanmielke/libdragon-extensions))
- Imports
    - Sprites
    - Sounds
    - Custom Content (can copy the content or use a custom build pipeline)
- Scenes
- Scripts (can attach to scenes or globally)

### Building

- Install SDL2 and SDL2_image
    - Windows: has to be discoverable by CMake
    - Linux: download/install latest from source
- Run `cmake` and `make` on the project folder