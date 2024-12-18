# Actors

Actors are the main form of "objects" or "entities" in the game. They are used to create any part of a level that is interactable or moving.

Each type of actor has a name, parameter names, initialization, update, and destruction function.
Every instance of an actor has:
- A 3D position
- A single rotation around the Y-axis
- An optional shadow
- An optional model, either a wall or a full 3D model
- Optional collision using the wall (3D models are not supported for collision)
- Four parameters that can be set in the level editor
- A pointer to any additional data the actor needs