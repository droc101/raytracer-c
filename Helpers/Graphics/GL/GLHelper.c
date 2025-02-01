//
// Created by droc101 on 9/30/2024.
//

#include "GLHelper.h"

#include <cglm/cglm.h>
#include "../../../Structs/GlobalState.h"
#include "../../../Structs/Vector2.h"
#include "../../../Structs/Wall.h"
#include "../../CommonAssets.h"
#include "../../Core/AssetReader.h"
#include "../../Core/Error.h"
#include "../../Core/Logging.h"
#include "../RenderingHelpers.h"
#include "GLInternal.h"

SDL_GLContext ctx;

GL_Shader *uiTextured;
GL_Shader *uiColored;
GL_Shader *wall;
GL_Shader *floorAndCeiling;
GL_Shader *shadow;
GL_Shader *sky;
GL_Shader *modelUnshaded;
GL_Shader *modelShaded;
GL_Shader *fbBlur;

GL_Buffer *glBuffer;

GLuint GL_Textures[GL_MAX_TEXTURE_SLOTS];
int GL_NextFreeSlot = 1; // Slot 0 is reserved for the framebuffer copy
int GL_AssetTextureMap[MAX_TEXTURES];
char GL_LastError[512];

GL_Buffer *GL_ModelBuffers[MAX_MODELS];

void GL_Error(const char *error)
{
	LogError("OpenGL Error: %s\n", error);
	strcpy(GL_LastError, error);
}

bool GL_PreInit()
{
	const bool msaaEnabled = GetState()->options.msaa != MSAA_NONE;
	if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaaEnabled) != 0)
	{
		LogError("Failed to set MSAA buffers attribute: %s\n", SDL_GetError());
	}
	if (msaaEnabled)
	{
		int mssaValue = 0;
		switch (GetState()->options.msaa)
		{
			case MSAA_2X:
				mssaValue = 2;
				break;
			case MSAA_4X:
				mssaValue = 4;
				break;
			case MSAA_8X:
				mssaValue = 8;
				break;
			default:
				GL_Error("Invalid MSAA value!");
				return false;
		}
		if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, mssaValue) != 0)
		{
			LogError("Failed to set MSAA samples attribute: %s\n", SDL_GetError());
		}
	}
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3), "Failed to set OpenGL major version", "Failed to start OpenGL");
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3), "Failed to set OpenGL minor version", "Failed to start OpenGL");
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1), "Failed to set OpenGL accelerated visual", "Failed to start OpenGL");
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE), "Failed to set OpenGL profile", "Failed to start OpenGL");
	TestSDLFunction(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1), "Failed to set OpenGL double buffer", "Failed to start OpenGL");

	memset(GL_AssetTextureMap, -1, MAX_TEXTURES * sizeof(int));
	memset(GL_Textures, 0, sizeof(GL_Textures));

	return true;
}

bool GL_Init(SDL_Window *wnd)
{
	LogInfo("Initializing OpenGL\n");

	ctx = SDL_GL_CreateContext(wnd);
	if (ctx == NULL)
	{
		LogError("SDL_GL_CreateContext Error: %s\n", SDL_GetError());
		GL_Error("Failed to create OpenGL context");
		return false;
	}

	TestSDLFunction_NonFatal(SDL_GL_SetSwapInterval(GetState()->options.vsync ? 1 : 0), "Failed to set VSync");

	// ReSharper disable once CppJoinDeclarationAndAssignment
	GLenum err;
	glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
	err = glewInit();
	if (err != GLEW_OK)
	{
		SDL_GL_DeleteContext(ctx);
		GL_Error("Failed to start OpenGL. Your GPU or drivers may not support OpenGL 3.3.");
		return false;
	}


#ifdef BUILDSTYLE_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GL_DebugMessageCallback, NULL);
#endif

	uiTextured = GL_ConstructShaderFromAssets(OGL_SHADER("GL_hud_textured_f"), OGL_SHADER("GL_hud_textured_v"));
	uiColored = GL_ConstructShaderFromAssets(OGL_SHADER("GL_hud_color_f"), OGL_SHADER("GL_hud_color_v"));
	wall = GL_ConstructShaderFromAssets(OGL_SHADER("GL_wall_f"), OGL_SHADER("GL_wall_v"));
	floorAndCeiling = GL_ConstructShaderFromAssets(OGL_SHADER("GL_floor_f"), OGL_SHADER("GL_floor_v"));
	shadow = GL_ConstructShaderFromAssets(OGL_SHADER("GL_shadow_f"), OGL_SHADER("GL_shadow_v"));
	sky = GL_ConstructShaderFromAssets(OGL_SHADER("GL_sky_f"), OGL_SHADER("GL_sky_v"));
	modelShaded = GL_ConstructShaderFromAssets(OGL_SHADER("GL_model_shaded_f"), OGL_SHADER("GL_model_shaded_v"));
	modelUnshaded = GL_ConstructShaderFromAssets(OGL_SHADER("GL_model_unshaded_f"), OGL_SHADER("GL_model_unshaded_v"));
	fbBlur = GL_ConstructShaderFromAssets(OGL_SHADER("GL_fb_blur_f"), OGL_SHADER("GL_fb_blur_v"));

	if (!uiTextured ||
		!uiColored ||
		!wall ||
		!floorAndCeiling ||
		!shadow ||
		!sky ||
		!modelShaded ||
		!modelUnshaded ||
		!fbBlur)
	{
		GL_Error("Failed to compile shaders");
		return false;
	}

	glBuffer = GL_ConstructBuffer();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);

	char *vendor = (char *)glGetString(GL_VENDOR);
	char *renderer = (char *)glGetString(GL_RENDERER);
	char *version = (char *)glGetString(GL_VERSION);
	char *shadingLanguage = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

	LogInfo("OpenGL Initialized\n");
	LogInfo("OpenGL Vendor: %s\n", vendor);
	LogInfo("OpenGL Renderer: %s\n", renderer);
	LogInfo("OpenGL Version: %s\n", version);
	LogInfo("GLSL: %s\n", shadingLanguage);

	fflush(stdout);

	GL_Disable3D();

	return true;
}

void GL_UpdateFramebufferTexture()
{
	glBindTexture(GL_TEXTURE_2D, GL_Textures[0]);
	int w;
	int h;
	SDL_GL_GetDrawableSize(GetGameWindow(), &w, &h);

	glReadBuffer(GL_BACK);

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, w, h, 0);
}

GL_Shader *GL_ConstructShaderFromAssets(const char *fsh, const char *vsh)
{
	const Asset *fragmentSource = DecompressAsset(fsh);
	const Asset *vertexSource = DecompressAsset(vsh);
	if (fragmentSource == NULL || vertexSource == NULL)
	{
		Error("Failed to load shaders!");
	}
	return GL_ConstructShader((char *)fragmentSource->data, (char *)vertexSource->data);
}

GL_Shader *GL_ConstructShader(const char *fsh, const char *vsh)
{
	GLint status;
	char errorBuffer[512];

	GL_Shader *shd = malloc(sizeof(GL_Shader));
	CheckAlloc(shd);

	shd->vsh = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shd->vsh, 1, (const GLchar *const *)&vsh, NULL);
	glCompileShader(shd->vsh);
	glGetShaderiv(shd->vsh, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		glGetShaderInfoLog(shd->vsh, sizeof(errorBuffer), NULL, errorBuffer);
		errorBuffer[sizeof(errorBuffer) - 1] = '\0';
		Error(errorBuffer);
	}

	shd->fsh = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shd->fsh, 1, (const GLchar *const *)&fsh, NULL);
	glCompileShader(shd->fsh);
	glGetShaderiv(shd->fsh, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		glGetShaderInfoLog(shd->fsh, sizeof(errorBuffer), NULL, errorBuffer);
		errorBuffer[sizeof(errorBuffer) - 1] = '\0';
		LogError(errorBuffer);
		free(shd);
		return NULL;
	}

	shd->program = glCreateProgram();
	glAttachShader(shd->program, shd->vsh);
	glAttachShader(shd->program, shd->fsh);
	glBindFragDataLocation(shd->program, 0, "COLOR");
	glLinkProgram(shd->program);

	glGetProgramiv(shd->program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		glGetProgramInfoLog(shd->program, sizeof(errorBuffer), NULL, errorBuffer);
		errorBuffer[sizeof(errorBuffer) - 1] = '\0';
		LogError(errorBuffer);
		free(shd);
		return NULL;
	}

	return shd;
}

void GL_DestroyShader(GL_Shader *shd)
{
	glDeleteShader(shd->vsh);
	glDeleteShader(shd->fsh);
	glDeleteProgram(shd->program);
	free(shd);
	shd = NULL;
}

GL_Buffer *GL_ConstructBuffer()
{
	GL_Buffer *buf = malloc(sizeof(GL_Buffer));
	CheckAlloc(buf);

	glGenVertexArrays(1, &buf->vao);
	glGenBuffers(1, &buf->vbo);
	glGenBuffers(1, &buf->ebo);

	return buf;
}

void GL_DestroyBuffer(GL_Buffer *buf)
{
	glDeleteVertexArrays(1, &buf->vao);
	glDeleteBuffers(1, &buf->vbo);
	glDeleteBuffers(1, &buf->ebo);
	free(buf);
}

inline void GL_ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GL_ClearColor(const uint color)
{
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;
	const float a = (float)(color >> 24 & 0xFF) / 255.0f;

	glClearColor(r, g, b, a);

	GL_ClearScreen();
}

inline void GL_ClearDepthOnly()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

inline void GL_Swap()
{
	SDL_GL_SwapWindow(GetGameWindow());
}

void GL_DestroyGL()
{
	GL_DestroyShader(uiTextured);
	GL_DestroyShader(uiColored);
	GL_DestroyShader(wall);
	GL_DestroyShader(floorAndCeiling);
	GL_DestroyShader(shadow);
	GL_DestroyShader(sky);
	GL_DestroyShader(modelShaded);
	GL_DestroyShader(modelUnshaded);
	glUseProgram(0);
	glDisableVertexAttribArray(0);
	GL_DestroyBuffer(glBuffer);
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		if (GL_Textures[i] != 0)
		{
			glDeleteTextures(1, &GL_Textures[i]);
		}
	}
	for (int i = 0; i < MAX_MODELS; i++)
	{
		if (GL_ModelBuffers[i] != NULL)
		{
			GL_DestroyBuffer(GL_ModelBuffers[i]);
		}
	}
	SDL_GL_DeleteContext(ctx);
}

void GL_DrawRect(const Vector2 pos, const Vector2 size, const uint color)
{
	glUseProgram(uiColored->program);

	const float a = (float)(color >> 24 & 0xFF) / 255.0f;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	glUniform4f(glGetUniformLocation(uiColored->program, "col"), r, g, b, a);

	const Vector2 ndcPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ndcPosEnd = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


	const float vertices[4][2] = {
		{(float)ndcPos.x, (float)ndcPos.y},
		{(float)ndcPosEnd.x, (float)ndcPos.y},
		{(float)ndcPosEnd.x, (float)ndcPosEnd.y},
		{(float)ndcPos.x, (float)ndcPosEnd.y},
	};

	const uint indices[] = {0, 1, 2, 0, 2, 3};

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiColored->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawRectOutline(const Vector2 pos, const Vector2 size, const uint color, const float thickness)
{
	if (thickness < 1.0f)
	{
		glEnable(GL_LINE_SMOOTH);
	} else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glLineWidth(thickness);

	glUseProgram(uiColored->program);

	const float a = (float)(color >> 24 & 0xFF) / 255.0f;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	glUniform4f(glGetUniformLocation(uiColored->program, "col"), r, g, b, a);

	const Vector2 ndcPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ndcPosEnd = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


	const float vertices[4][2] = {
		{(float)ndcPos.x, (float)ndcPos.y},
		{(float)ndcPosEnd.x, (float)ndcPos.y},
		{(float)ndcPosEnd.x, (float)ndcPosEnd.y},
		{(float)ndcPos.x, (float)ndcPosEnd.y},
	};

	const uint indices[] = {0, 1, 2, 3};

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiColored->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, NULL);
}

void GL_LoadTextureFromAsset(const char *texture)
{
	const Image *img = LoadImage(texture);

	// if the texture is already loaded, don't load it again
	if (GL_AssetTextureMap[img->id] != -1)
	{
		if (glIsTexture(GL_Textures[GL_AssetTextureMap[img->id]]))
		{
			glBindTexture(GL_TEXTURE_2D, GL_Textures[GL_AssetTextureMap[img->id]]);
			return;
		}
	}

	const int slot = GL_RegisterTexture(img->pixelData, (int)img->width, (int)img->height);

	GL_AssetTextureMap[img->id] = slot;
}

int GL_RegisterTexture(const unsigned char *pixelData, const int width, const int height)
{
	const int slot = GL_NextFreeSlot;

	glGenTextures(1, &GL_Textures[slot]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GL_Textures[slot]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.5f);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (GetState()->options.mipmaps)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, pixelData);

	GL_NextFreeSlot++;

	return slot;
}

void GL_SetTexParams(const char *texture, const bool linear, const bool repeat)
{
	GL_LoadTextureFromAsset(texture); // make sure the texture is loaded

	const Image *img = LoadImage(texture);

	const GLuint tex = GL_Textures[GL_AssetTextureMap[img->id]];

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	if (GetState()->options.mipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D,
						GL_TEXTURE_MIN_FILTER,
						linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
	} else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
	}
}

void GL_DrawBlur(const Vector2 pos, const Vector2 size, const int blurRadius)
{
	glUseProgram(fbBlur->program);

	GL_UpdateFramebufferTexture();

	glUniform1i(glGetUniformLocation(fbBlur->program, "blurRadius"), blurRadius);

	const Vector2 ndcPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ndcPosEnd = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


	const float vertices[4][2] = {
		{(float)ndcPos.x, (float)ndcPos.y},
		{(float)ndcPosEnd.x, (float)ndcPos.y},
		{(float)ndcPosEnd.x, (float)ndcPosEnd.y},
		{(float)ndcPos.x, (float)ndcPosEnd.y},
	};

	const uint indices[] = {0, 1, 2, 0, 2, 3};

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(fbBlur->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawTexture_Internal(const Vector2 pos,
							 const Vector2 size,
							 const char *texture,
							 const uint color,
							 const Vector2 region_start,
							 const Vector2 region_end)
{
	glUseProgram(uiTextured->program);

	GL_LoadTextureFromAsset(texture);


	const float a = (float)(color >> 24 & 0xFF) / 255.0f;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	glUniform4f(glGetUniformLocation(uiTextured->program, "col"), r, g, b, a);

	glUniform4f(glGetUniformLocation(uiTextured->program, "region"),
				(GLfloat)region_start.x,
				(GLfloat)region_start.y,
				(GLfloat)region_end.x,
				(GLfloat)region_end.y);

	const Vector2 ndcPos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
	const Vector2 ndcPosEnd = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


	const float vertices[4][4] = {
		{(float)ndcPos.x, (float)ndcPos.y, 0.0f, 0.0f},
		{(float)ndcPosEnd.x, (float)ndcPos.y, 1.0f, 0.0f},
		{(float)ndcPosEnd.x, (float)ndcPosEnd.y, 1.0f, 1.0f},
		{(float)ndcPos.x, (float)ndcPosEnd.y, 0.0f, 1.0f},
	};

	const uint indices[] = {0, 1, 2, 0, 2, 3};

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiTextured->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(uiTextured->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

inline void GL_DrawTexture(const Vector2 pos, const Vector2 size, const char *texture)
{
	GL_DrawTexture_Internal(pos, size, texture, 0xFFFFFFFF, v2(-1, 0), v2s(0));
}

inline void GL_DrawTextureMod(const Vector2 pos, const Vector2 size, const char *texture, const uint color)
{
	GL_DrawTexture_Internal(pos, size, texture, color, v2(-1, 0), v2s(0));
}

inline void GL_DrawTextureRegion(const Vector2 pos,
								 const Vector2 size,
								 const char *texture,
								 const Vector2 region_start,
								 const Vector2 region_end)
{
	GL_DrawTexture_Internal(pos, size, texture, 0xFFFFFFFF, region_start, region_end);
}

inline void GL_DrawTextureRegionMod(const Vector2 pos,
									const Vector2 size,
									const char *texture,
									const Vector2 region_start,
									const Vector2 region_end,
									const uint color)
{
	GL_DrawTexture_Internal(pos, size, texture, color, region_start, region_end);
}

void GL_DrawLine(const Vector2 start, const Vector2 end, const uint color, const float thickness)
{
	if (thickness < 1.0f)
	{
		glEnable(GL_LINE_SMOOTH);
	} else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glUseProgram(uiColored->program);

	const float a = (float)(color >> 24 & 0xFF) / 255.0f;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	glUniform4f(glGetUniformLocation(uiColored->program, "col"), r, g, b, a);

	const Vector2 ndcStart = v2(GL_X_TO_NDC(start.x), GL_Y_TO_NDC(start.y));
	const Vector2 ndcEnd = v2(GL_X_TO_NDC(end.x), GL_Y_TO_NDC(end.y));

	// Calculate the 2 corner vertices of each point for a thick line
	const float vertices[2][2] = {
		{(float)ndcStart.x, (float)ndcStart.y},
		{(float)ndcEnd.x, (float)ndcEnd.y},
	};

	const uint indices[] = {0, 1};

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiColored->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glLineWidth(thickness);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, NULL);
}

void GL_SetLevelParams(const mat4 *mvp, const Level *l)
{
	glUseProgram(wall->program);

	glUniformMatrix4fv(glGetUniformLocation(wall->program, "WORLD_VIEW_MATRIX"),
					   1,
					   GL_FALSE,
					   mvp[0][0]); // world -> screen

	const uint color = l->fogColor;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	glUniform3f(glGetUniformLocation(wall->program, "fog_color"), r, g, b);

	glUniform1f(glGetUniformLocation(wall->program, "fog_start"), (float)l->fogStart);
	glUniform1f(glGetUniformLocation(wall->program, "fog_end"), (float)l->fogEnd);

	glUseProgram(sky->program);
	glUniformMatrix4fv(glGetUniformLocation(sky->program, "WORLD_VIEW_MATRIX"),
					   1,
					   GL_FALSE,
					   mvp[0][0]); // world -> screen

	glUseProgram(modelShaded->program);
	glUniformMatrix4fv(glGetUniformLocation(modelShaded->program, "WORLD_VIEW_MATRIX"),
					   1,
					   GL_FALSE,
					   mvp[0][0]); // world -> screen

	glUseProgram(modelUnshaded->program);
	glUniformMatrix4fv(glGetUniformLocation(modelUnshaded->program, "WORLD_VIEW_MATRIX"),
					   1,
					   GL_FALSE,
					   mvp[0][0]); // world -> screen
}

void GL_DrawWall(const Wall *w, const mat4 mdl, const Camera *cam, const Level * /*l*/)
{
	glUseProgram(wall->program);

	GL_LoadTextureFromAsset(w->tex);

	glUniformMatrix4fv(glGetUniformLocation(wall->program, "MODEL_WORLD_MATRIX"),
					   1,
					   GL_FALSE,
					   mdl[0]); // model -> world

	glUniform1f(glGetUniformLocation(wall->program, "camera_yaw"), cam->yaw);
	glUniform1f(glGetUniformLocation(wall->program, "wall_angle"), (float)w->angle);

	float vertices[4][5] = {
		// X Y Z U V
		{(float)w->a.x, 0.5f * w->height, (float)w->a.y, 0.0f, 0.0f},
		{(float)w->b.x, 0.5f * w->height, (float)w->b.y, (float)w->length, 0.0f},
		{(float)w->b.x, -0.5f * w->height, (float)w->b.y, (float)w->length, 1.0f},
		{(float)w->a.x, -0.5f * w->height, (float)w->a.y, 0.0f, 1.0f},
	};

	const float uvOffset = w->uvOffset;
	const float uvScale = w->uvScale;
	for (int i = 0; i < 4; i++)
	{
		vertices[i][3] = vertices[i][3] * uvScale + uvOffset;
	}

	const uint indices[] = {0, 1, 2, 0, 2, 3};

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(wall->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(wall->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawFloor(const Vector2 vp1,
				  const Vector2 vp2,
				  const mat4 *mvp,
				  const Level *l,
				  const char *texture,
				  const float height,
				  const float shade)
{
	glUseProgram(floorAndCeiling->program);

	GL_LoadTextureFromAsset(texture);

	glUniformMatrix4fv(glGetUniformLocation(floorAndCeiling->program, "WORLD_VIEW_MATRIX"),
					   1,
					   GL_FALSE,
					   mvp[0][0]); // world -> screen

	const uint color = l->fogColor;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	glUniform3f(glGetUniformLocation(floorAndCeiling->program, "fog_color"), r, g, b);

	glUniform1f(glGetUniformLocation(floorAndCeiling->program, "fog_start"), (float)l->fogStart);
	glUniform1f(glGetUniformLocation(floorAndCeiling->program, "fog_end"), (float)l->fogEnd);

	glUniform1f(glGetUniformLocation(floorAndCeiling->program, "height"), height);
	glUniform1f(glGetUniformLocation(floorAndCeiling->program, "shade"), shade);

	const float vertices[4][2] = {
		// X Z
		{(float)vp1.x, (float)vp1.y},
		{(float)vp2.x, (float)vp1.y},
		{(float)vp2.x, (float)vp2.y},
		{(float)vp1.x, (float)vp2.y},
	};

	const uint indices[] = {0, 1, 2, 0, 2, 3};

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(floorAndCeiling->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawShadow(const Vector2 vp1, const Vector2 vp2, const mat4 *mvp, const mat4 mdl, const Level *l)
{
	glUseProgram(shadow->program);

	GL_LoadTextureFromAsset(TEXTURE("vfx_shadow"));

	glUniformMatrix4fv(glGetUniformLocation(shadow->program, "WORLD_VIEW_MATRIX"),
					   1,
					   GL_FALSE,
					   mvp[0][0]); // world -> screen
	glUniformMatrix4fv(glGetUniformLocation(shadow->program, "MODEL_WORLD_MATRIX"),
					   1,
					   GL_FALSE,
					   mdl[0]); // model -> world

	const uint color = l->fogColor;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	glUniform3f(glGetUniformLocation(shadow->program, "fog_color"), r, g, b);

	glUniform1f(glGetUniformLocation(shadow->program, "fog_start"), (float)l->fogStart);
	glUniform1f(glGetUniformLocation(shadow->program, "fog_end"), (float)l->fogEnd);

	const float vertices[4][2] = {
		// X Z
		{(float)vp1.x, (float)vp1.y},
		{(float)vp2.x, (float)vp1.y},
		{(float)vp2.x, (float)vp2.y},
		{(float)vp1.x, (float)vp2.y},
	};

	const uint indices[] = {0, 1, 2, 0, 2, 3};

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(shadow->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

inline void GL_Enable3D()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glClear(GL_DEPTH_BUFFER_BIT);
}

inline void GL_Disable3D()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);
	glClear(GL_DEPTH_BUFFER_BIT);
}

inline void GL_UpdateViewportSize()
{
	int w;
	int h;
	SDL_GL_GetDrawableSize(GetGameWindow(), &w, &h);
	glViewport(0, 0, w, h);

	if (GL_Textures[0] != -1)
	{
		glDeleteTextures(1, &GL_Textures[0]);
	}

	GLuint frameBufferTexture;
	glGenTextures(1, &frameBufferTexture);
	glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	GL_Textures[0] = frameBufferTexture;
}

void GL_DrawColoredArrays(const float *vertices, const uint *indices, const uint quad_count, const uint color)
{
	glUseProgram(uiColored->program);

	const float a = (float)(color >> 24 & 0xFF) / 255.0f;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	glUniform4f(glGetUniformLocation(uiTextured->program, "col"), r, g, b, a);

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, (long)(quad_count * 16 * sizeof(float)), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long)(quad_count * 6 * sizeof(uint)), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiColored->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	glDrawElements(GL_TRIANGLES, (int)(quad_count * 6), GL_UNSIGNED_INT, NULL);
}

void GL_DrawTexturedArrays(const float *vertices,
						   const uint *indices,
						   const int quad_count,
						   const char *texture,
						   const uint color)
{
	glUseProgram(uiTextured->program);

	GL_LoadTextureFromAsset(texture);

	const float a = (float)(color >> 24 & 0xFF) / 255.0f;
	const float r = (float)(color >> 16 & 0xFF) / 255.0f;
	const float g = (float)(color >> 8 & 0xFF) / 255.0f;
	const float b = (float)(color & 0xFF) / 255.0f;

	glUniform4f(glGetUniformLocation(uiTextured->program, "col"), r, g, b, a);

	glUniform4f(glGetUniformLocation(uiTextured->program, "region"), -1, 0, 0, 0);

	glBindVertexArray(glBuffer->vao);

	glBindBuffer(GL_ARRAY_BUFFER, glBuffer->vbo);
	glBufferData(GL_ARRAY_BUFFER, (long)(quad_count * 16 * sizeof(float)), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (long)(quad_count * 6 * sizeof(uint)), indices, GL_STATIC_DRAW);

	const GLint posAttrLoc = glGetAttribLocation(uiTextured->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(uiTextured->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	glDrawElements(GL_TRIANGLES, quad_count * 6, GL_UNSIGNED_INT, NULL);
}

mat4 *GL_GetMatrix(const Camera *cam)
{
	vec3 cam_pos = {cam->x, cam->y, cam->z};
	const float aspect = (float)WindowWidth() / (float)WindowHeight();

	mat4 IDENTITY = GLM_MAT4_IDENTITY_INIT;
	mat4 PERSPECTIVE = GLM_MAT4_ZERO_INIT;
	glm_perspective(glm_rad(cam->fov), aspect, NEAR_Z, FAR_Z, PERSPECTIVE);

	vec3 look_at = {cosf(cam->yaw), 0, sinf(cam->yaw)};
	vec3 up = {0, 1, 0};

	// TODO: roll and pitch are messed up

	glm_vec3_rotate(look_at, cam->roll, (vec3){0, 0, 1}); // Roll
	glm_vec3_rotate(look_at, cam->pitch, (vec3){1, 0, 0}); // Pitch

	look_at[0] += cam_pos[0];
	look_at[1] += cam_pos[1];
	look_at[2] += cam_pos[2];

	mat4 VIEW = GLM_MAT4_ZERO_INIT;
	glm_lookat(cam_pos, look_at, up, VIEW);

	mat4 MODEL_VIEW = GLM_MAT4_ZERO_INIT;
	glm_mat4_mul(VIEW, IDENTITY, MODEL_VIEW);

	mat4 *MODEL_VIEW_PROJECTION = malloc(sizeof(mat4));
	CheckAlloc(MODEL_VIEW_PROJECTION);
	glm_mat4_mul(PERSPECTIVE, MODEL_VIEW, *MODEL_VIEW_PROJECTION);

	return MODEL_VIEW_PROJECTION;
}

void GL_RenderLevel(const Level *l, const Camera *cam)
{
	GL_Enable3D();

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	//glLineWidth(2);

	mat4 *WORLD_VIEW_MATRIX = GL_GetMatrix(cam);
	const mat4 IDENTITY = GLM_MAT4_IDENTITY_INIT;
	mat4 SKY_MODEL_WORLD = GLM_MAT4_IDENTITY_INIT;
	glm_translated(SKY_MODEL_WORLD, (vec3){(float)l->player.pos.x, 0, (float)l->player.pos.y});

	const Vector2 floor_start = v2(l->player.pos.x - 100, l->player.pos.y - 100);
	const Vector2 floor_end = v2(l->player.pos.x + 100, l->player.pos.y + 100);

	GL_SetLevelParams(WORLD_VIEW_MATRIX, l);

	if (l->hasCeiling)
	{
		GL_DrawFloor(floor_start, floor_end, WORLD_VIEW_MATRIX, l, l->ceilOrSkyTex, 0.5f, 0.8f);
	} else
	{
		GL_RenderModel(skyModel, SKY_MODEL_WORLD, l->ceilOrSkyTex, SHADER_SKY);
		GL_ClearDepthOnly(); // prevent sky from clipping into walls
	}

	GL_DrawFloor(floor_start, floor_end, WORLD_VIEW_MATRIX, l, l->floorTex, -0.5f, 1.0f);

	glDisable(GL_DEPTH_TEST);
	for (int i = 0; i < l->actors.length; i++)
	{
		const Actor *actor = ListGet(l->actors, i);
		if (actor->showShadow)
		{
			mat4 actor_xfm;
			ActorTransformMatrix(actor, &actor_xfm);
			// remove the rotation and y position from the actor matrix so the shadow draws correctly
			glm_rotate(actor_xfm, (float)actor->rotation, (vec3){0, 1, 0});
			glm_translate(actor_xfm, (vec3){0, -actor->yPosition, 0});

			GL_DrawShadow(v2s(-0.5 * actor->shadowSize), v2s(0.5 * actor->shadowSize), WORLD_VIEW_MATRIX, actor_xfm, l);
		}
	}
	glEnable(GL_DEPTH_TEST);

	for (int i = 0; i < l->walls.length; i++)
	{
		GL_DrawWall(ListGet(l->walls, i), IDENTITY, cam, l);
	}

	for (int i = 0; i < l->actors.length; i++)
	{
		const Actor *actor = ListGet(l->actors, i);
		mat4 actor_xfm = GLM_MAT4_IDENTITY_INIT;
		ActorTransformMatrix(actor, &actor_xfm);
		if (actor->actorModel == NULL)
		{
			if (actor->actorWall == NULL)
			{
				continue;
			}
			Wall w;
			memcpy(&w, actor->actorWall, sizeof(Wall));
			WallBake(&w);
			w.angle += actor->rotation;
			GL_DrawWall(&w, actor_xfm, cam, l);
		} else
		{
			GL_RenderModel(actor->actorModel, actor_xfm, actor->actorModelTexture, SHADER_SHADED);
		}
	}

	free(WORLD_VIEW_MATRIX);

	//glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	GL_Disable3D();
}

void GL_LoadModel(const Model *model)
{
	if (GL_ModelBuffers[model->id] != NULL)
	{
		const GL_Buffer *buf = GL_ModelBuffers[model->id];
		glBindVertexArray(buf->vao);
		glBindBuffer(GL_ARRAY_BUFFER, buf->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->ebo);
		return;
	}
	GL_Buffer *buf = GL_ConstructBuffer();
	GL_ModelBuffers[model->id] = buf;

	glBindVertexArray(buf->vao);

	glBindBuffer(GL_ARRAY_BUFFER, buf->vbo);
	glBufferData(GL_ARRAY_BUFFER,
				 (long)(model->vertexCount * sizeof(float) * 8),
				 model->vertexData,
				 GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 (long)(model->indexCount * sizeof(uint)),
				 model->indexData,
				 GL_STATIC_DRAW);
}

void GL_RenderModel(const Model *model, const mat4 modelWorldMatrix, const char *texture, const ModelShader shader)
{
	GL_Shader *shd;
	switch (shader)
	{
		case SHADER_SKY:
			shd = sky;
			break;
		case SHADER_SHADED:
			shd = modelShaded;
			break;
		case SHADER_UNSHADED:
			shd = modelUnshaded;
			break;
		default:
			Error("Invalid shader for model drawing");
	}

	glUseProgram(shd->program);

	GL_LoadTextureFromAsset(texture);

	glUniformMatrix4fv(glGetUniformLocation(shd->program, "MODEL_WORLD_MATRIX"),
					   1,
					   GL_FALSE,
					   modelWorldMatrix[0]); // model -> world

	GL_LoadModel(model);

	const GLint posAttrLoc = glGetAttribLocation(shd->program, "VERTEX");
	glVertexAttribPointer(posAttrLoc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(posAttrLoc);

	const GLint texAttrLoc = glGetAttribLocation(shd->program, "VERTEX_UV");
	glVertexAttribPointer(texAttrLoc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrLoc);

	if (shader == SHADER_SHADED) // other shaders do not take normals
	{
		const GLint normAttrLoc = glGetAttribLocation(shd->program, "VERTEX_NORMAL");
		glVertexAttribPointer(normAttrLoc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
		glEnableVertexAttribArray(normAttrLoc);
	}

	glDrawElements(GL_TRIANGLES, (int)model->indexCount, GL_UNSIGNED_INT, NULL);
}
