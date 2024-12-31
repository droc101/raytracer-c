# Building - Windows

## Windows build are currently not supported. The following instructions are for reference only, and things may be missing/out of date

This is the far more complicated of the two platforms to build on.

### Setup
You will need cmake and gcc installed. No other compilers are supported.

Make sure you have [FindSDL2.cmake](https://github.com/tcbrindle/sdl2-cmake-scripts/blob/master/FindSDL2.cmake) and [FindSDL2_mixer.cmake](https://github.com/tcbrindle/sdl2-cmake-scripts/blob/master/FindSDL2_mixer.cmake) in your CMake modules directory.
You need Python 3 (as the command `python`) with PIL or Pillow installed to run the asset compression script (required to build)

Extract the following libraries into the `lib` directory (create it if it doesn't exist):
- Download [SDL3 3.X w64](https://github.com/libsdl-org/SDL/releases/) (look for SDL3-devel-3.X.X-mingw.zip)
  and extract it as `SDL3`.
- Download [SDL3_Mixer 3.X w64](https://github.com/libsdl-org/SDL_mixer/releases/) (look for SDL3_mixer-devel-3.X.X-mingw.zip)
  and extract it as `SDL3_Mixer`.
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
You'll need to copy `lib/SDL3/x86_64-w64-mingw32/bin/SDL3.dll`, `lib/SDL3_mixer/x86_64-w64-mingw32/bin/SDL3_mixer.dll`, and `lib/zlib/bin/libzlib1.dll` into your output directory in order for the program to run.
