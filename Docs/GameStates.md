# GameStates

Each game state is a separate file that contains the logic for that state.
A game state can provide the following functions:
- Render callback
- Per-frame update callback
- Fixed update callback, usually for physics (optional)

The game only remembers the current state, so each state is responsible for transitioning to any other state it needs, as well as cleaning up after itself.
