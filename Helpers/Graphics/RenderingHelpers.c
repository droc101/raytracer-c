//
// Created by droc101 on 10/2/24.
//

#include "RenderingHelpers.h"
#include "GL/glHelper.h"
#include "../../Structs/Wall.h"
#include "../../Structs/Vector2.h"
#include "../CommonAssets.h"
#include "../../Structs/GlobalState.h"

mat4 *GetMatrix(Camera *cam) {
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

mat4 *ActorTransformMatrix(Actor *Actor) {
    mat4 *MODEL = malloc(sizeof(mat4));
    glm_mat4_identity(*MODEL);
    glm_translate(*MODEL, (vec3){Actor->position.x, Actor->yPosition, Actor->position.y});
    glm_rotate(*MODEL, -Actor->rotation, (vec3){0, 1, 0});
    return MODEL;
}

void RenderPreInit() {
    GL_PreInit();
}

void RenderInit() {
    GL_Init();
    GL_Disable3D(); // just to make sure we are in the correct state
}

void RenderDestroy() {
    GL_DestroyGL();
}

void RenderLevel3D(Level *l, Camera *cam) {
    GL_Enable3D();
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    mat4 *WORLD_VIEW_MATRIX = GetMatrix(cam);
    mat4 *IDENTITY = malloc(sizeof(mat4));
    glm_mat4_identity(*IDENTITY);

    Vector2 floor_start = v2(l->position.x - 100, l->position.y - 100);
    Vector2 floor_end = v2(l->position.x + 100, l->position.y + 100);

    GL_DrawFloor(floor_start, floor_end, WORLD_VIEW_MATRIX, l, wallTextures[l->FloorTexture], -0.5, 1.0);
    if (l->CeilingTexture != 0) {
        GL_DrawFloor(floor_start, floor_end, WORLD_VIEW_MATRIX, l, wallTextures[l->CeilingTexture - 1], 0.5, 0.8);
    }

    for (int i = 0; i < l->staticWalls->size; i++) {
        GL_DrawWall(SizedArrayGet(l->staticWalls, i), WORLD_VIEW_MATRIX, IDENTITY, cam, l);
    }

    for (int i = 0; i < l->staticActors->size; i++) {
        Actor *actor = SizedArrayGet(l->staticActors, i);
        WallBake(actor->actorWall);
        mat4 *actor_xfm = ActorTransformMatrix(actor);
        GL_DrawWall(actor->actorWall, WORLD_VIEW_MATRIX, actor_xfm, cam, l);

        if (actor->showShadow) {
            // remove the rotation and y position from the actor matrix so the shadow draws correctly
            glm_rotate(*actor_xfm, actor->rotation, (vec3) {0, 1, 0});
            glm_translate(*actor_xfm, (vec3) {0, -actor->yPosition, 0});

            GL_DrawShadow(v2s(-0.5 * actor->shadowSize), v2s(0.5 * actor->shadowSize), WORLD_VIEW_MATRIX, actor_xfm, l);
        }

        free(actor_xfm);
    }

    free(WORLD_VIEW_MATRIX);
    free(IDENTITY);

    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    GL_Disable3D();
}

inline void UpdateViewportSize() {
    float newScale = (float)ActualWindowSize().x / (float)DEF_WIDTH;
    GetState()->options.uiScale = newScale;
    GL_UpdateViewportSize();
}

inline void DrawBatchedQuadsTextured(BatchedQuadArray *batch, const unsigned char *imageData, uint color) {
    GL_DrawTexturedArrays(batch->verts, batch->indices, batch->quad_count, imageData, color);
}

inline void DrawBatchedQuadsColored(BatchedQuadArray *batch, uint color) {
    GL_DrawColoredArrays(batch->verts, batch->indices, batch->quad_count, color);
}

inline float X_TO_NDC(float x) {
    return GL_X_TO_NDC(x);
}

inline float Y_TO_NDC(float y) {
    return GL_Y_TO_NDC(y);
}
