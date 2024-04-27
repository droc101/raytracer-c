cmake_minimum_required(VERSION 3.25)
project(game C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 --strip -Wl,--gc-sections -fdata-sections -ffunction-sections")

# Add --static flag when building on Windows to include libwinpthread-1.dll
if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} --static")
endif ()

set(SDL2_PATH "${CMAKE_SOURCE_DIR}/SDL2/x86_64-w64-mingw32")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(game main.c Helpers/Input.h Helpers/Input.c defines.h Helpers/Drawing.h Helpers/Drawing.c Assets/Assets.h Assets/Assets.c Structs/Vector2.h Structs/Vector2.c Helpers/MathEx.h Helpers/MathEx.c Structs/Ray.h Structs/Ray.c Structs/Wall.h Structs/Wall.c Helpers/List.h Helpers/List.c Structs/Level.h Structs/Level.c Helpers/Font.c Helpers/Font.h Helpers/Error.c Helpers/Error.h Helpers/LevelLoader.c Helpers/LevelLoader.h Structs/GlobalState.c Structs/GlobalState.h GameStates/GMainState.c GameStates/GMainState.h GameStates/GMenuState.c GameStates/GMenuState.h Structs/Actor.c Structs/Actor.h Actor/TestActor.c Actor/TestActor.h GameStates/GPauseState.c GameStates/GPauseState.h
        Debug/FrameGrapher.c
        Debug/FrameGrapher.h
        Debug/DPrint.c
        Debug/DPrint.h
        Helpers/Timing.c
        Helpers/Timing.h
)

if (UNIX)
    target_link_libraries(game ${SDL2_LIBRARY} m SDL2) # Link math library on Unix
elseif (WIN32)
    target_link_libraries(game ${SDL2_LIBRARY})
endif ()

