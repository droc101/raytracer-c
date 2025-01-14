//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "Helpers/Core/List.h"

#pragma region Forward Declarations/Typedefs

// Basic types
typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

// Enum forward declarations
typedef enum CurrentState CurrentState;
typedef enum Renderer Renderer;
typedef enum OptionsMsaa OptionsMsaa;
typedef enum ModelShader ModelShader;
typedef enum ImageDataOffsets ImageDataOffsets;
typedef enum AssetType AssetType;

// Struct forward declarations
typedef struct GlobalState GlobalState;
typedef struct Vector2 Vector2;
typedef struct Camera Camera;
typedef struct Player Player;
typedef struct Wall Wall;
typedef struct Level Level;
typedef struct RayCastResult RayCastResult;
typedef struct TextBox TextBox;
typedef struct Model Model;
typedef struct Actor Actor;
typedef struct ModelHeader ModelHeader;
typedef struct Options Options;
typedef struct Asset Asset;
typedef struct TextureSizeTable TextureSizeTable;
typedef struct Image Image;

// Function signatures
typedef void (*FixedUpdateFunction)(GlobalState *state, double delta);

typedef void (*FrameUpdateFunction)(GlobalState *state);

typedef void (*FrameRenderFunction)(GlobalState *state);

typedef void (*TextBoxCloseFunction)(TextBox *textBox);

typedef void (*ActorInitFunction)(Actor *self);

typedef void (*ActorUpdateFunction)(Actor *self, double delta);

typedef void (*ActorDestroyFunction)(Actor *self);

#pragma endregion

#pragma region Utility defines

#define STR(x) #x
#define TO_STR(x) STR(x)
#define PI 3.14159265358979323846
#define PIf 3.14159265358979323846f
#define PHYSICS_TARGET_MS (1000 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_MS_D (1000.0 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_NS (1000000000 / PHYSICS_TARGET_TPS) // nanoseconds because precision
#define PHYSICS_TARGET_NS_D (1000000000.0 / PHYSICS_TARGET_TPS)

#define CONTROLLER_OK (GetState()->options.controllerSwapOkCancel ? SDL_CONTROLLER_BUTTON_B : SDL_CONTROLLER_BUTTON_A)
#define CONTROLLER_CANCEL \
	(GetState()->options.controllerSwapOkCancel ? SDL_CONTROLLER_BUTTON_A : SDL_CONTROLLER_BUTTON_B)

#pragma endregion

#pragma region Enum definitions

enum AssetType
{
	ASSET_TYPE_TEXTURE = 0,
	ASSET_TYPE_MP3 = 1,
	ASSET_TYPE_WAV = 2,
	ASSET_TYPE_LEVEL = 3,
	ASSET_TYPE_GLSL = 4,
	ASSET_TYPE_SPIRV_FRAG = 5,
	ASSET_TYPE_SPIRV_VERT = 6,
	ASSET_TYPE_MODEL = 7,
};

/**
 * Use to get data from a decompressed image asset using @c ReadUintA
 */
enum ImageDataOffsets
{
	IMAGE_SIZE_OFFSET = 0,
	IMAGE_WIDTH_OFFSET = 4,
	IMAGE_HEIGHT_OFFSET = 8,
	IMAGE_ID_OFFSET = 12
};

/**
 * Used to check which game state the game is in
 * Now, you *could* just set a complete mess of state functions and disregard this, but if you do that, I will find you.
 */
enum CurrentState
{
	EDITOR_STATE,
	LEVEL_SELECT_STATE,
	LOGO_SPLASH_STATE,
	MAIN_STATE,
	MENU_STATE,
	PAUSE_STATE,
	OPTIONS_STATE,
	VIDEO_OPTIONS_STATE,
	SOUND_OPTIONS_STATE,
	INPUT_OPTIONS_STATE
};

/**
 * Used to check which renderer the game is using
 */
enum Renderer
{
	RENDERER_OPENGL,
	RENDERER_VULKAN,
	RENDERER_MAX
};

/**
 * Used the check the MSAA level setting
 */
enum OptionsMsaa
{
	MSAA_NONE = 0,
	MSAA_2X = 1,
	MSAA_4X = 2,
	MSAA_8X = 3
};

/**
 * List of shaders a model can be rendered with
 */
enum ModelShader
{
	SHADER_SKY,
	SHADER_UNSHADED,
	SHADER_SHADED
};

#pragma endregion

#pragma region Struct definitions

// Utility functions are in Structs/Vector2.h
struct Vector2
{
	double x;
	double y;
};

struct Camera
{
	float x;
	float y;
	float z;

	float pitch;
	float yaw;
	float roll;

	float fov;
};

struct Player
{
	Vector2 pos;
	double angle;
};

// Utility functions are in Structs/wall.h
struct Wall
{
	Vector2 a; // The first point of the wall
	Vector2 b; // The second point of the wall
	const char *tex; // The texture name
	int texId; // The texture ID
	double length; // The length of the wall (Call WallBake to update)
	double angle; // The angle of the wall (Call WallBake to update)
	double dx;
	double dy;
	float uvScale; // The X scale of the texture
	float uvOffset; // The X offset of the texture
	float height; // height of the wall for rendering. Does not affect collision
};

// Utility functions are in Structs/level.h
struct Level
{
	char name[32];
	short courseNum;
	/// The list of actors in the level.
	List *actors;
	/// The list of walls in the level.
	List *walls;
	/// The color of the sky.
	uint skyColor;
	/// The texture index of the floor.
	uint floorTextureIndex;
	/// The texture index of the ceiling, where -1 means that the level has no ceiling.
	uint ceilingTextureIndex;
	/// The index of the music used in the level.
	uint musicIndex;
	/// The color of the fog in the level.
	uint fogColor;
	/// The distance from the player at which the fog begins.
	double fogStart;
	/// The distance from the player at which the fog ends.
	double fogEnd;
	/// The player object.
	Player player;
};

// Utility functions are in Structs/ray.h
struct RayCastResult
{
	Vector2 collisionPoint; // The point of collision
	double distance; // The distance to the collision
	bool collided; // Whether the ray collided with anything
	Wall collisionWall; // The wall that was collided with
};

struct TextBox
{
	char *text; // The text to display
	int rows; // The number of rows to display
	int cols; // The number of columns to display
	int x; // The x position of the text box
	int y; // The y position of the text box

	int hAlign; // The horizontal alignment of the text box
	int vAlign; // The vertical alignment of the text box

	int theme; // The theme of the text box

	TextBoxCloseFunction Close; // The function to call when the text box is closed
};

struct Options
{
	ushort checksum; // Checksum of the options struct (helps prevent corruption)

	// Controls
	bool controllerMode; // Whether the game is in controller mode
	double mouseSpeed; // The look speed (it affects controller speed too)
	float rumbleStrength; // The strength of the rumble
	bool cameraInvertX; // Whether to invert the camera X axis (controller only)
	bool controllerSwapOkCancel; // Whether to swap the controller A and B buttons

	// Video
	Renderer renderer; // The renderer to use
	bool fullscreen; // Whether the game is fullscreen
	bool vsync; // Whether vsync is enabled
	OptionsMsaa msaa;
	bool mipmaps;

	// Sound
	double musicVolume; // The volume of the music
	double sfxVolume; // The volume of the sound effects
	double masterVolume; // The master volume
} __attribute__((packed)); // This is packed because it is saved to disk

struct ModelHeader
{
	char sig[4]; // "MESH"
	uint indexCount;
	char dataSig[4]; // "DATA"
} __attribute__((packed));

struct Model
{
	ModelHeader header;

	uint packedVertsUvsCount;
	uint packedIndicesCount;
	float *packedVertsUvs; // X Y Z U V, use for rendering
	uint *packedIndices; // Just the vert index, use for rendering
};

// Global state of the game
struct GlobalState
{
	Level *level; // Current level
	FrameUpdateFunction UpdateGame; // State update function
	FrameRenderFunction RenderGame; // State render function
	CurrentState currentState; // The current state of the game
	int hp; // Player health
	int maxHp; // Player max health
	int ammo; // Player ammo
	int maxAmmo; // Player max ammo
	int coins; // The number of coins the player has
	int blueCoins; // The number of blue coins the player has
	ulong physicsFrame; // The number of physics frames that have passed since the last game state change
	bool requestExit; // Request to exit the game
	Mix_Music *music; // background music
	Mix_Chunk *channels[SFX_CHANNEL_COUNT]; // sound effects
	double cameraY; // The Y position of the camera

	bool textBoxActive; // Whether the text box is active
	TextBox textBox; // The text box
	int textBoxPage; // The current page of the text box

	Camera *cam; // The camera

	Options options; // Game options

	char executablePath[261]; // The path to the executable
	char executableFolder[261];

	double uiScale; // The scale of the UI.
	bool freezeEvents; // Whether to freeze the event loop. This should only be used for debugging.
	bool isAudioStarted; // Whether the audio system has been started successfully
};

// Actor (interactable/moving wall) struct
struct Actor
{
	Vector2 position; // The position of the actor
	double rotation; // The rotation of the actor
	Wall *actorWall; // (0,0) in this wall is the actor's position (also transformed by rotation)
	bool solid; // can the player walk through this actor?
	int health; // health. may be unused for some actors
	void *extra_data; // extra data for the actor
	ActorInitFunction Init;
	ActorUpdateFunction Update;
	ActorDestroyFunction Destroy;
	int actorType; // type of actor. do not change this after creation.
	byte paramA; // extra parameters for the actor. saved in level data, so can be used during Init
	byte paramB;
	byte paramC;
	byte paramD;
	float yPosition; // y position for rendering. Does not affect collision
	bool showShadow; // should the actor cast a shadow?
	float shadowSize; // size of the shadow
	Model *actorModel; // Optional model for the actor, if not NULL, will be rendered instead of the wall
	char *actorModelTexture; // Texture for the model
};

struct Asset
{
	uint compressedSize;
	uint size;
	uint assetId;
	AssetType type;
	byte *data;
};

struct TextureSizeTable
{
	/// The total number of textures in the table
	uint textureCount;
	/// The total number of assets in the game
	uint assetCount;
	/// The names of the textures in the table. You can load them with @code DecompressAsset(TEXTURE(assetName)) @endcode
	char (*textureNames)[32];
} __attribute__((packed));

struct Image
{
	uint pixelDataSize;
	uint width;
	uint height;
	uint id;
	char *name;
	byte *pixelData;
};

#pragma endregion

#endif //GAME_DEFINES_H
