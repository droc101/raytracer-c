//
// Created by droc101 on 10/2/24.
//

#include "RenderingHelpers.h"
#include "GL/glHelper.h"

mat4 *GetMatrix(Camera *cam) {
    vec3 cam_pos = {cam->x, cam->y, cam->z};
    float aspect = (float)WindowWidth() / (float)WindowHeight();

    mat4 IDENTITY = GLM_MAT4_IDENTITY_INIT;
    mat4 PERSPECTIVE = GLM_MAT4_ZERO_INIT;
    glm_perspective(cam->fov, aspect, NEAR_Z, FAR_Z, PERSPECTIVE);

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

// TODO: rotation is not working
mat4 *ActorTransformMatrix(Actor *Actor) {
    mat4 *MODEL = malloc(sizeof(mat4));
    glm_mat4_identity(*MODEL);
    glm_rotate(*MODEL, glm_rad(Actor->rotation), (vec3){0, 1, 0});
    glm_translate(*MODEL, (vec3){Actor->position.x, 0, Actor->position.y});
    return MODEL;
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

    for (int i = 0; i < l->staticWalls->size; i++) {
        GL_DrawWall(SizedArrayGet(l->staticWalls, i), WORLD_VIEW_MATRIX, IDENTITY);
    }

    for (int i = 0; i < l->staticActors->size; i++) {
        mat4 *actor = ActorTransformMatrix(SizedArrayGet(l->staticActors, i));
        GL_DrawWall(((Actor*)SizedArrayGet(l->staticActors, i))->actorWall, WORLD_VIEW_MATRIX, actor);
        free(actor);
    }

    free(WORLD_VIEW_MATRIX);

    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    GL_Disable3D();
}

void UpdateViewportSize() {
    GL_UpdateViewportSize();
}
