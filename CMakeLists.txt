cmake_minimum_required(VERSION 3.25)
project(game C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wno-discarded-qualifiers") # shut.

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -s -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections -Wall")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wl,--gc-sections -fdata-sections -ffunction-sections -Wall")

# Add --static flag when building on Windows to include libwinpthread-1.dll
if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --static")
    set(CMAKE_AUTO_RCC ON)
endif ()

# Enable LTO in release builds
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if (result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif ()
endif ()

add_executable(
        game
        main.c
        Helpers/Core/Input.h
        Helpers/Core/Input.c
        defines.h
        Helpers/Graphics/Drawing.h
        Helpers/Graphics/Drawing.c
        Assets/Assets.h
        Assets/Assets.c
        Structs/Vector2.h
        Structs/Vector2.c
        Helpers/Core/MathEx.h
        Helpers/Core/MathEx.c
        Structs/Ray.h
        Structs/Ray.c
        Structs/Wall.h
        Structs/Wall.c
        Helpers/Core/List.h
        Helpers/Core/List.c
        Structs/Level.h
        Structs/Level.c
        Helpers/Graphics/Font.c
        Helpers/Graphics/Font.h
        Helpers/Core/Error.c
        Helpers/Core/Error.h
        Helpers/LevelLoader.c
        Helpers/LevelLoader.h
        Structs/GlobalState.c
        Structs/GlobalState.h
        GameStates/GMainState.c
        GameStates/GMainState.h
        GameStates/GMenuState.c
        GameStates/GMenuState.h
        Structs/Actor.c
        Structs/Actor.h
        Actor/TestActor.c
        Actor/TestActor.h
        GameStates/GPauseState.c
        GameStates/GPauseState.h
        Debug/FrameGrapher.c
        Debug/FrameGrapher.h
        Debug/DPrint.c
        Debug/DPrint.h
        Helpers/Core/Timing.c
        Helpers/Core/Timing.h
        Assets/AssetReader.c
        Assets/AssetReader.h
        Helpers/Core/DataReader.c
        Helpers/Core/DataReader.h
        Helpers/Collision.c
        Helpers/Collision.h
        GameStates/GEditorState.c
        GameStates/GEditorState.h
        config.h
        GameStates/GLevelSelectState.c
        GameStates/GLevelSelectState.h
        Helpers/LevelEntries.c
        Helpers/LevelEntries.h
        Helpers/CommonAssets.c
        Helpers/CommonAssets.h
        Actor/Coin.c
        Actor/Coin.h
        Helpers/TextBox.c
        Helpers/TextBox.h
        Actor/Goal.c
        Actor/Goal.h
        GameStates/GLogoSplashState.c
        GameStates/GLogoSplashState.h
        Helpers/Graphics/GL/glHelper.c
        Helpers/Graphics/GL/glHelper.h
        Structs/Camera.c
        Structs/Camera.h
        Helpers/Graphics/RenderingHelpers.c
        Helpers/Graphics/RenderingHelpers.h
        Helpers/Graphics/GL/glDebug.c
        Helpers/Graphics/GL/glDebug.h
        Structs/UI/UiStack.c
        Structs/UI/UiStack.h
        Structs/UI/Controls/Button.c
        Structs/UI/Controls/Button.h
        Structs/UI/Controls/Slider.c
        Structs/UI/Controls/Slider.h
        $<$<BOOL:${WIN32}>:game.rc>
        Structs/Options.c
        Structs/Options.h
        GameStates/GOptionsState.c
        GameStates/GOptionsState.h
        Structs/UI/Controls/CheckBox.c
        Structs/UI/Controls/CheckBox.h
        Structs/UI/Controls/RadioButton.c
        Structs/UI/Controls/RadioButton.h
        Helpers/Core/Logging.c
        Helpers/Core/Logging.h
)

set(SDL2_PATH "${CMAKE_SOURCE_DIR}/lib/SDL2/x86_64-w64-mingw32")
set(SDL2_MIXER_PATH "${CMAKE_SOURCE_DIR}/lib/SDL2_mixer/x86_64-w64-mingw32")
set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/lib/zlib")

find_package(SDL2 REQUIRED) # Platform abstraction
include_directories(${SDL2_INCLUDE_DIR})

find_package(SDL2_mixer REQUIRED) # Audio
include_directories(${SDL2_MIXER_INCLUDE_DIR})

find_package(ZLIB REQUIRED) # Decompression
include_directories(${ZLIB_INCLUDE_DIRS})

find_package(OpenGL REQUIRED) # Graphics
target_include_directories(game PUBLIC ${OPENGL_INCLUDE_DIR})

if (UNIX)
    find_package(GLEW REQUIRED) # OpenGL Extension Wrangler
    target_include_directories(game PUBLIC ${GLEW_INCLUDE_DIRS})

    target_include_directories(game PUBLIC "/usr/include/cglm") # C OpenGL Mathematics Library
elseif (WIN32) # Thank you Windows, I hate you.
    target_include_directories(game PUBLIC "${CMAKE_SOURCE_DIR}/lib/cglm/include")

    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib/glew")
    add_definitions(-DGLEW_STATIC)
    set(GLEW_LIBRARIES "${CMAKE_SOURCE_DIR}/lib/glew/lib/Release/x64/glew32s.lib")
    set(GLEW_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/glew/include")
    target_include_directories(game PUBLIC ${GLEW_INCLUDE_DIRS})
endif ()

if (UNIX)
    target_link_libraries(game -lGL -lGLU ${SDL2_LIBRARY} ${ZLIB_LIBRARIES} ${SDL2_MIXER_LIBRARY} m SDL2 SDL2_mixer z ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES}) # extra l* flags are needed on Linux
elseif (WIN32)
    target_link_libraries(game PRIVATE ${SDL2_LIBRARY} ${ZLIB_LIBRARIES} ${SDL2_MIXER_LIBRARY} ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES} opengl32 dwmapi) # Don't pass extra l* flags on Windows (it isn't happy with them)
endif ()

add_custom_target( # Python script for generating assets code
        run ALL
        COMMAND "python" ${CMAKE_CURRENT_SOURCE_DIR}/Assets/genassets.py ${CMAKE_CURRENT_SOURCE_DIR}/Assets/
        COMMENT "Generating assets"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Assets/
        BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/Assets/Assets.h ${CMAKE_CURRENT_SOURCE_DIR}/Assets/Assets.c
)

add_dependencies(game run)
