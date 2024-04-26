cmake_minimum_required(VERSION 3.25)
project(game C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 --strip -Wl,--gc-sections -fdata-sections -ffunction-sections")

set(SDL2_PATH "${CMAKE_SOURCE_DIR}/SDL2/x86_64-w64-mingw32")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(game main.c Helpers/Input.h Helpers/Input.c defines.h Helpers/Drawing.h Helpers/Drawing.c assets/assets.h assets/assets.c Structs/Vector2.h Structs/Vector2.c Helpers/MathEx.h Helpers/MathEx.c Structs/Ray.h Structs/Ray.c Structs/Wall.h Structs/Wall.c Helpers/List.h Helpers/List.c Structs/Level.h Structs/Level.c Helpers/Font.c Helpers/Font.h Helpers/Error.c Helpers/Error.h Helpers/LevelLoader.c Helpers/LevelLoader.h Structs/GlobalState.c Structs/GlobalState.h GameStates/GMainState.c GameStates/GMainState.h GameStates/GMenuState.c GameStates/GMenuState.h Structs/Actor.c Structs/Actor.h actor/TestActor.c actor/TestActor.h GameStates/GPauseState.c GameStates/GPauseState.h
        Debug/FrameGrapher.c
        Debug/FrameGrapher.h
)

if (UNIX)
    target_link_libraries(game ${SDL2_LIBRARY} m SDL2)
elseif (WIN32)
    target_link_libraries(game ${SDL2_LIBRARY})
endif ()

