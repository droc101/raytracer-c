# Levels, Walls, and Triggers

## Levels
A level is primarily a collection of walls actors, and triggers, with a few additional properties that are mainly visual.

### Properties
- Sky or Ceiling Texture: The texture used for the sky or ceiling.
- Has Ceiling: Whether the level has a ceiling.
- Floor texture: The texture used for the floor.
- Music: The music to play in the level, or "none" for no music.
- Fog Color: The color of the fog in the level.
- Fog Start: The distance from the camera at which the fog starts.
- Fog End: The distance from the camera at which the fog is fully opaque.

## Walls
Walls are used to create the main layout of a level. They are defined in 2D space, start and end point.
Walls are rendered infinitely thin, but collision is calculated with a slight depth.

### Properties
- Texture: The texture to use for the wall.
- UV Scale: The scale of the texture on the wall.
- UV Offset: The offset of the texture on the wall.

### Properties (Actor Walls Only)
- Height: The height extent from the center of the wall (the wall is 2x this value tall).

## Triggers
Triggers are invisible boxes that run commands when entered.

### Properties
- Position: The center position of the trigger.
- Extents: The size of the trigger.
- Rotation: The rotation of the trigger.
- Command: The command to run when the trigger is entered.
- Flags: Flags that modify the behavior of the trigger.
  - One-Shot: The trigger will be destroyed after running once.
