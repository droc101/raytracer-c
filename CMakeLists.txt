cmake_minimum_required(VERSION 3.25)
project(game C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wno-discarded-qualifiers") # shut.

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -s -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections -Wall")

# Add --static flag when building on Windows to include libwinpthread-1.dll
if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --static")
endif ()

set(SDL2_PATH "${CMAKE_SOURCE_DIR}/SDL2/x86_64-w64-mingw32")
set(SDL2_MIXER_PATH "${CMAKE_SOURCE_DIR}/SDL2_mixer/x86_64-w64-mingw32")
set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/zlib")

find_package(SDL2 REQUIRED) # Graphics
include_directories(${SDL2_INCLUDE_DIR})

find_package(SDL2_mixer REQUIRED) # Audio
include_directories(${SDL2_MIXER_INCLUDE_DIR})

find_package(ZLIB REQUIRED) # Decompression
include_directories(${ZLIB_INCLUDE_DIRS})

# I have no idea why it has decided to start adding newlines, but it still works soooooo
add_executable(game main.c Helpers/Input.h Helpers/Input.c defines.h Helpers/Drawing.h Helpers/Drawing.c Assets/Assets.h Assets/Assets.c Structs/Vector2.h Structs/Vector2.c Helpers/MathEx.h Helpers/MathEx.c Structs/Ray.h Structs/Ray.c Structs/Wall.h Structs/Wall.c Helpers/List.h Helpers/List.c Structs/Level.h Structs/Level.c Helpers/Font.c Helpers/Font.h Helpers/Error.c Helpers/Error.h Helpers/LevelLoader.c Helpers/LevelLoader.h Structs/GlobalState.c Structs/GlobalState.h GameStates/GMainState.c GameStates/GMainState.h GameStates/GMenuState.c GameStates/GMenuState.h Structs/Actor.c Structs/Actor.h Actor/TestActor.c Actor/TestActor.h GameStates/GPauseState.c GameStates/GPauseState.h
        Debug/FrameGrapher.c
        Debug/FrameGrapher.h
        Debug/DPrint.c
        Debug/DPrint.h
        Helpers/Timing.c
        Helpers/Timing.h
        Assets/AssetReader.c
        Assets/AssetReader.h
        Helpers/DataReader.c
        Helpers/DataReader.h
)

if (UNIX)
    target_link_libraries(game ${SDL2_LIBRARY} ${ZLIB_LIBRARIES} ${SDL2_MIXER_LIBRARY} m SDL2 SDL2_mixer z) # extra l* flags are needed on Linux
elseif (WIN32)
    target_link_libraries(game PRIVATE ${SDL2_LIBRARY} ${ZLIB_LIBRARIES} ${SDL2_MIXER_LIBRARY}) # Don't pass extra l* flags on Windows (it isn't happy with them)
endif ()

add_custom_target(
        run ALL
        COMMAND "python" ${CMAKE_CURRENT_SOURCE_DIR}/Assets/genassets.py ${CMAKE_CURRENT_SOURCE_DIR}/Assets/
        COMMENT "Generating assets"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Assets/
        BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/Assets/Assets.h ${CMAKE_CURRENT_SOURCE_DIR}/Assets/Assets.c
)

add_dependencies(game run)

