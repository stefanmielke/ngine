# NGine - N64 Engine Powered by Libdragon

#### Engine to create N64 games powered by [Libdragon](https://github.com/DragonMinded/libdragon) for Windows and Linux.

### Download

- [Latest Release](https://github.com/stefanmielke/ngine/releases/latest)
- [Latest Builds](https://github.com/stefanmielke/ngine/actions) (Unstable)

### Requirements

- [SDL2](https://www.libsdl.org/index.php) (bundled on Windows)
- [SDL2_Image](https://www.libsdl.org/projects/SDL_image/) (bundled on Windows)
- [SDL2_Mixer](https://www.libsdl.org/projects/SDL_mixer/) (bundled on Windows)
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf) (bundled on Windows)
- [Libdragon CLI](https://github.com/anacierdem/libdragon-docker) (bundled)
- [Docker](https://www.docker.com/get-started)

### What it is

A tool that generates C code based on the modules selected, and that can import sprites and sounds into the game.

It can also build the game, as well as run it on an emulator (you can configure which).

### What it is NOT

Unity or UE4, where you can create the game using another language. It also (for now) does not have a way to create
nodes nor supports [ECS](https://en.wikipedia.org/wiki/Entity_component_system) directly (but you can add those yourself
if you want).

Also Libdragon still has no support for 3D (but should have soon), so that is also not provided natively (but you can import 3D assets).

### How to use it

Refer to this project's [wiki](https://github.com/stefanmielke/ngine/wiki) for more information (under construction).

### Functionalities Supported

- Modules:
  - Input
  - Audio / Mixer
  - Display / RDP
  - Console
  - Libdragon's Debug Modules
  - Real-Time Clock
  - Scene Manager (from [Libdragon-Extensions](https://github.com/stefanmielke/libdragon-extensions))
  - Memory Pool (from [Libdragon-Extensions](https://github.com/stefanmielke/libdragon-extensions))
  - Scenes (from [Libdragon-Extensions](https://github.com/stefanmielke/libdragon-extensions))
- Asset Importing:
  - Sprites/Textures
    - BMP
    - PCX
    - PNG
    - JPG
    - TGA
  - Sounds
    - WAV
    - YM
    - XM
  - Maps (using [Libdragon-Extensions](https://github.com/stefanmielke/libdragon-extensions))
    - [Tiled](https://www.mapeditor.org/)
    - [LDtk](https://ldtk.io/)
  - Custom Content (can copy the content or use a custom build pipeline)
- Scripting for code (can attach to scenes or globally)

### Overview Video

> Version 1.0

[![Watch the video](https://img.youtube.com/vi/jswC7u4MbQ0/default.jpg)](https://youtu.be/jswC7u4MbQ0)

### Building

- Install SDL2, SDL2_image, SDL2_mixer, SDL2_ttf
    - Windows: has to be discoverable by CMake
    - Linux: download/install latest from source
- Run `cmake` and `make` on the project folder.
    - You can see the build process inside `/.github/workflows/cmake.yml` to understand how to build based on your OS.