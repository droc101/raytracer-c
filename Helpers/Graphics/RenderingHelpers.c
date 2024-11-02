//
// Created by droc101 on 10/2/24.
//

#include "RenderingHelpers.h"
#include "GL/glHelper.h"
#include "../../Structs/Wall.h"
#include "../../Structs/Vector2.h"
#include "../CommonAssets.h"
#include "../../Structs/GlobalState.h"
#include "Vulkan/Vulkan.h"

mat4* GetMatrix(Camera *cam) {
    vec3 cam_pos = {cam->x, cam->y, cam->z};
    float aspect = (float)WindowWidth() / (float)WindowHeight();

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

mat4* ActorTransformMatrix(Actor *Actor) {
    mat4 *MODEL = malloc(sizeof(mat4));
    glm_mat4_identity(*MODEL);
    glm_translate(*MODEL, (vec3){Actor->position.x, Actor->yPosition, Actor->position.y});
    glm_rotate(*MODEL, -Actor->rotation, (vec3){0, 1, 0});
    return MODEL;
}

bool RenderPreInit() {
    switch (GetState()->options.renderer) {
        case RENDERER_VULKAN:
            return true;
        case RENDERER_OPENGL:
            return GL_PreInit();
        default:
            return false;
    }
}

bool RenderInit() {
    switch (GetState()->options.renderer) {
        case RENDERER_VULKAN:
            return InitVulkan(GetWindow());
        case RENDERER_OPENGL:
            const bool gli = GL_Init(GetWindow());
            GL_Disable3D(); // just to make sure we are in the correct state
            return gli;
        default:
            return false;
    }
}

void RenderDestroy() {
    switch (GetState()->options.renderer) {
        case RENDERER_VULKAN:
            CleanupVulkan();
            break;
        case RENDERER_OPENGL:
            GL_DestroyGL();
            break;
    }
}

void RenderLevel3D(Level *l, Camera *cam) {
    switch (GetState()->options.renderer) {
        case RENDERER_VULKAN:
            DrawFrame();
            break;
        case RENDERER_OPENGL:
            GL_RenderLevel(l, cam);
            break;
    }
}

inline void UpdateViewportSize() {
    float newScaleX = (float)ActualWindowSize().x / (float)DEF_WIDTH;
    float newScaleY = (float)ActualWindowSize().y / (float)DEF_HEIGHT;
    float newScale = newScaleX < newScaleY ? newScaleX : newScaleY;
    GetState()->options.uiScale = newScale;
    switch (GetState()->options.renderer) {
        case RENDERER_VULKAN:
            // TODO: Implement this. Guide can be found at https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/04_Swap_chain_recreation.html 
            break;
        case RENDERER_OPENGL:
            GL_UpdateViewportSize();
            break;
    }
}

inline void DrawBatchedQuadsTextured(BatchedQuadArray *batch, const unsigned char *imageData, uint color) {
    switch (GetState()->options.renderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_DrawTexturedArrays(batch->verts, batch->indices, batch->quad_count, imageData, color);
            break;
    }
}

inline void DrawBatchedQuadsColored(BatchedQuadArray *batch, uint color) {
    switch (GetState()->options.renderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_DrawColoredArrays(batch->verts, batch->indices, batch->quad_count, color);
            break;
    }
}

inline float X_TO_NDC(float x) {
    switch (GetState()->options.renderer) {
        case RENDERER_VULKAN:
            return 0;
        case RENDERER_OPENGL:
            return GL_X_TO_NDC(x);
        default:
            return 0;
    }
}

inline float Y_TO_NDC(float y) {
    switch (GetState()->options.renderer) {
        case RENDERER_VULKAN:
            return 0;
        case RENDERER_OPENGL:
            return GL_Y_TO_NDC(y);
        default:
            return 0;
    }
}
