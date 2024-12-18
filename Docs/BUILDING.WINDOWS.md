# Building - Windows
This is the far more complicated of the two platforms to build on.

### Setup
You will need cmake and gcc installed. No other compilers are supported.

Make sure you have [FindSDL2.cmake](https://github.com/tcbrindle/sdl2-cmake-scripts/blob/master/FindSDL2.cmake) and [FindSDL2_mixer.cmake](https://github.com/tcbrindle/sdl2-cmake-scripts/blob/master/FindSDL2_mixer.cmake) in your CMake modules directory.
You need Python 3 (as the command `python`) with PIL or Pillow installed to run the asset compression script (required to build)

Extract the following libraries into the `lib` directory (create it if it doesn't exist):
- Download [SDL2 2.X w64](https://github.com/libsdl-org/SDL/releases/) (look for SDL2-devel-2.X.X-mingw.zip)
  and extract it as `SDL2`.
- Download [SDL2_Mixer 2.X w64](https://github.com/libsdl-org/SDL_mixer/releases/) (look for SDL2_mixer-devel-2.X.X-mingw.zip)
  and extract it as `SDL2_Mixer`.
- Download [GLEW 2.2.0 w64](https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip) and extract it as `glew`
- Download [CGLM 0.9.4](https://github.com/recp/cglm/archive/refs/tags/v0.9.4.zip) and extract it as `cglm`

### Building
This project uses the `cmake` build system.
There are 3 build types available:
- `Debug` - No optimizations, debug symbols and features enabled.
- `Release` - Full optimizations, no debug symbols or features.
- `RelWithDebInfo` - Full optimizations, debug symbols and features enabled.

The compiled executable will be `game` in the `build` directory.

### Running
You'll need to copy `lib/SDL2/x86_64-w64-mingw32/bin/SDL2.dll`, `lib/SDL2_mixer/x86_64-w64-mingw32/bin/SDL2_mixer.dll`, and `lib/zlib/bin/libzlib1.dll` into your output directory in order for the program to run.
