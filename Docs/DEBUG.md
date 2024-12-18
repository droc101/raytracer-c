# Debugging Features & Development Tools

The engine has various debug features available. These can be enabled or disabled in the `config.h` file or via their individual headers (under the `Debug` directory).

## Logging
`Helpers/Core/Logging.h` provides a simple logging system. It provides `LogInfo`, `LogWarning`, `LogError`, and `LogDebug`. The latter is only run in debug builds.

## DPrint
`Debug/Dprint.h` provides `DPrintf` which prints a message to an on-screen "console" that is reset every frame. This is useful for debugging without needing to open a console or slow down due to console I/O.

## Frame Grapther
`Debug/FrameGrapher.h` provides a simple frame grapher that shows the framerate/frametime over a period of time, as well as the current FPS and MSPF. Several configuration options are available in the header.

The graph contains 2 lines, the opaque one is FPS, where higher is better, and the transparent one is frametime, where lower is better.

## Level Editor
There is a full-featured level editor available. When enabled, it can be toggled by pressing F6 while in a level.
