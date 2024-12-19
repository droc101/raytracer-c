# Levels & Walls

## Levels
A level is primarily a collection of walls and actors, with a few additional properties that are mainly visual.

### Properties
- Sky Color: The color of the sky. This is a mixed onto the skybox texture.
- Floor texture: The texture used for the floor.
- Ceiling texture: The texture used for the ceiling, or 0 for no ceiling.
- Music ID: The ID of the music to play in the level, or 0 for no music.
- Fog Color: The color of the fog in the level.
- Fog Start: The distance from the camera at which the fog starts.
- Fog End: The distance from the camera at which the fog is fully opaque.

## Walls
Walls are used to create the main layout of a level. They are defined in 2D space, start and end point.
Walls are rendered infinitely thin, but collision is calculated with a slight depth.

### Properties
- Texture ID: The ID of the texture to use for the wall.

### Properties (Actor Walls Only)
- UV Scale: The scale of the texture on the wall.
- UV Offset: The offset of the texture on the wall.
- Height: The height extent from the center of the wall (the wall is 2x this value tall).
