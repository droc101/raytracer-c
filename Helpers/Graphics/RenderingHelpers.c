//
// Created by droc101 on 10/2/24.
//

#include "RenderingHelpers.h"
#include "../CommonAssets.h"
#include "../../Structs/GlobalState.h"
#include "../Core/MathEx.h"
#include "GL/GLHelper.h"
#include "Vulkan/Vulkan.h"

Renderer currentRenderer;
bool lowFPSMode;

mat4 *GetMatrix(const Camera *cam)
{
    vec3 cam_pos = {cam->x, cam->y, cam->z};
    const float aspect = (float) WindowWidth() / (float) WindowHeight();

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
    glm_mat4_mul(PERSPECTIVE, MODEL_VIEW, *MODEL_VIEW_PROJECTION);

    return MODEL_VIEW_PROJECTION;
}

mat4 *ActorTransformMatrix(const Actor *Actor)
{
    mat4 *MODEL = malloc(sizeof(mat4));
    glm_mat4_identity(*MODEL);
    glm_translate(*MODEL, (vec3){Actor->position.x, Actor->yPosition, Actor->position.y});
    glm_rotate(*MODEL, -Actor->rotation, (vec3){0, 1, 0});
    return MODEL;
}

bool RenderPreInit()
{
    currentRenderer = GetState()->options.renderer;
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            return true;
        case RENDERER_OPENGL:
            return GL_PreInit();
        default:
            return false;
    }
}

bool RenderInit()
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            return VK_Init(GetGameWindow());
        case RENDERER_OPENGL:
            const bool gli = GL_Init(GetGameWindow());
            return gli;
        default:
            return false;
    }
}

void RenderDestroy()
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            VK_Cleanup();
            break;
        case RENDERER_OPENGL:
            GL_DestroyGL();
            break;
        default: break;
    }
}

void FrameStart()
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            VK_FrameStart();
            break;
        case RENDERER_OPENGL: // NOLINT(*-branch-clone)

            break;
        default: break;
    }
}

void FrameEnd()
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            VK_FrameEnd();
            break;
        case RENDERER_OPENGL:
            GL_Swap();
            break;
        default: break;
    }
}

void RenderLevel3D(const Level *l, const Camera *cam)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            VK_RenderLevel();
            break;
        case RENDERER_OPENGL:
            GL_RenderLevel(l, cam);
            break;
        default: break;
    }
}

inline void UpdateViewportSize()
{
    const float newScaleX = (float) ActualWindowSize().x / (float) DEF_WIDTH;
    const float newScaleY = (float) ActualWindowSize().y / (float) DEF_HEIGHT;
    float newScale = newScaleX < newScaleY ? newScaleX : newScaleY;
    newScale = max(newScale, 1.0f);
    GetState()->uiScale = newScale;
    UpdateWindowSize();
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            int w, h;
            SDL_GetWindowSize(GetGameWindow(), &w, &h);

            break;
        case RENDERER_OPENGL:
            GL_UpdateViewportSize();
            break;
        default: break;
    }
}

inline void WindowObscured()
{
    lowFPSMode = true;
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            VK_Minimize();
            break;
        case RENDERER_OPENGL:

            break;
        default: break;
    }
}

inline void WindowRestored()
{
    lowFPSMode = false;
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            VK_Restore();
            break;
        case RENDERER_OPENGL:

            break;
        default: break;
    }
}

inline void SetLowFPS(const bool val)
{
    lowFPSMode = val;
}

inline bool IsLowFPSModeEnabled()
{
    return lowFPSMode;
}

inline void DrawBatchedQuadsTextured(const BatchedQuadArray *batch, const unsigned char *imageData, const uint color)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_DrawTexturedArrays(batch->verts, batch->indices, batch->quad_count, imageData, color);
            break;
        default: break;
    }
}

inline void DrawBatchedQuadsColored(const BatchedQuadArray *batch, const uint color)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_DrawColoredArrays(batch->verts, batch->indices, batch->quad_count, color);
            break;
        default: break;
    }
}

inline float X_TO_NDC(const float x)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN: // NOLINT(*-branch-clone)
            return VK_X_TO_NDC(x);
        case RENDERER_OPENGL:
            return GL_X_TO_NDC(x);
        default:
            return 0;
    }
}

inline float Y_TO_NDC(const float y)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:
            return VK_Y_TO_NDC(y);
        case RENDERER_OPENGL:
            return GL_Y_TO_NDC(y);
        default:
            return 0;
    }
}
