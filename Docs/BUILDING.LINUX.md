# Building - Linux
Congratulations! Linux is far easier to build on than Windows.

### Setup
- You will need to have `cmake`, `make`, and `gcc` installed.
- You will need development packages for SDL3, SDL3_Mixer, GLEW, and CGLM.
- You need Python 3 (as the command `python`) with PIL or Pillow installed to run the asset compression script (required to build)

### Libraries
You will need development packages for SDL3, SDL3_Mixer, GLEW, ZLIB, and CGLM.

Arch Packages:
[glew](https://archlinux.org/packages/extra/x86_64/glew/),
[cglmᴬᵁᴿ](https://aur.archlinux.org/packages/cglm),
[sdl3-gitᴬᵁᴿ](https://aur.archlinux.org/packages/sdl3-git),
[sdl3_mixer-gitᴬᵁᴿ](https://aur.archlinux.org/packages/sdl3_mixer-git)
[zlib](https://archlinux.org/packages/core/x86_64/zlib/)
[python](https://archlinux.org/packages/core/x86_64/python/),
[python-pillow](https://archlinux.org/packages/extra/x86_64/python-pillow/)

### Building
There are 3 build types available:
- `Debug` - No optimizations, debug symbols and features enabled.
- `Release` - Full optimizations, no debug symbols or features.
- `RelWithDebInfo` - Full optimizations, debug symbols and features enabled.

Open the terminal in the project directory and run the following commands to build the project:
```sh
mkdir build
cd build
cmake -B . -DCMAKE_BUILD_TYPE=[Build type] ..
cmake --build . --target game -- -j
```
The compiled executable will be `game` in the `build` directory.
