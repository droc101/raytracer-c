# raytracer-c

![](Assets/actor/BLOB2.png)

Old-School first-person raycaster game written in C using [SDL2](https://www.libsdl.org/) for rendering and audio, and [zlib](https://www.zlib.net/) for asset compression.
Runs on Windows and Linux.

### Setup
Extract the following libraries into the `lib` directory (create it if it doesn't exist):
- Download [SDL2 2.30.2 w64](https://github.com/libsdl-org/SDL/releases/download/release-2.30.2/SDL2-devel-2.30.2-mingw.zip)
and extract it as `SDL2`.
- Download [SDL2_Mixer 2.8.0 w64](https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-devel-2.8.0-mingw.zip) 
and extract it as `SDL2_Mixer`.
#### Windows Only:
- Download [GLEW 2.2.0 w64](https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip) and extract it as `glew`
- Dowbload [CGLM 0.9.4](https://github.com/recp/cglm/archive/refs/tags/v0.9.4.zip) and extract it as `cglm`

### Building
Linux building requires sdl2, sdl2_mixer, glew and cglm development libraries to be installed.<br />
Arch Packages:
[glew](https://archlinux.org/packages/extra/x86_64/glew/),
[cglmᴬᵁᴿ](https://aur.archlinux.org/packages/cglm),
[sdl2](https://archlinux.org/packages/extra/x86_64/sdl2/),
[sdl2_mixer](https://archlinux.org/packages/extra/x86_64/sdl2_mixer/)

Make sure you have [FindSDL2.cmake](https://github.com/tcbrindle/sdl2-cmake-scripts/blob/master/FindSDL2.cmake) and [FindSDL2_mixer.cmake](https://github.com/tcbrindle/sdl2-cmake-scripts/blob/master/FindSDL2_mixer.cmake) in your CMake modules directory.

### Running
On Windows, you'll need to copy `lib/SDL2/x86_64-w64-mingw32/bin/SDL2.dll`, `lib/SDL2_mixer/x86_64-w64-mingw32/bin/SDL2_mixer.dll`, and `lib/zlib/bin/libzlib1.dll` into your output directory.

### Tested on
- Windows 11 (but should work on 10)
- Arch Linux