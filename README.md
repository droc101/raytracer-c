# raytracer-c
Old-School first-person raycaster game written in C using [SDL2](https://www.libsdl.org/) for rendering.
Runs on Windows and Linux.

### Setup
Download [SDL2 2.30.2 w64](https://github.com/libsdl-org/SDL/releases/download/release-2.30.2/SDL2-devel-2.30.2-mingw.zip)
and extract it to the root of the project as "SDL2".

### Notes
On Windows, you'll need to copy `SDL2/x86_64-w64-mingw32/bin/SDL2.dll` into your output directory. On Linux, you just need the sdl2 shared library installed (refer to your specific distro for that)

### Tested on
- Windows 10 and 11
- Arch Linux
- Debian 12