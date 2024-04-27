# raytracer-c
Old-School first-person raycaster game written in C using [SDL2](https://www.libsdl.org/) for rendering and audio, and [zlib](https://www.zlib.net/) for asset compression.
Runs on Windows and Linux.

### Setup
Download [SDL2 2.30.2 w64](https://github.com/libsdl-org/SDL/releases/download/release-2.30.2/SDL2-devel-2.30.2-mingw.zip)
and extract it to the root of the project as "SDL2".
Download [SDL2_Mixer 2.8.0 w64](https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-devel-2.8.0-mingw.zip) 
and extract it to the root of the project as "SDL2_Mixer".

Make sure you have [FindSDL2.cmake](https://github.com/tcbrindle/sdl2-cmake-scripts/blob/master/FindSDL2.cmake) and [FindSDL2_mixer.cmake](https://github.com/tcbrindle/sdl2-cmake-scripts/blob/master/FindSDL2_mixer.cmake)
 in your CMake modules directory.

### Notes
On Windows, you'll need to copy `SDL2/x86_64-w64-mingw32/bin/SDL2.dll`, `SDL2_mixer/x86_64-w64-mingw32/bin/SDL2_mixer.dll`, and `zlib/bin/libzlib1.dll` into your output directory. 

On Linux, you just need the sdl2, sdl2_mixer, and zlib shared libraries installed (refer to your specific distro for that)

### Tested on
- Windows 10 and 11
- Arch Linux