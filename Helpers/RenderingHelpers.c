//
// Created by droc101 on 10/2/24.
//

#include "RenderingHelpers.h"
#include "GL/glHelper.h"

mat4 *GL_GetMatrix(Camera *cam) {
    vec3 cam_pos = {cam->x, cam->y, cam->z};
    vec3 cam_rot = {cam->pitch, cam->yaw, cam->roll};
    float aspect = (float)WindowWidth() / (float)WindowHeight();

    mat4 IDENTITY = GLM_MAT4_IDENTITY_INIT;
    mat4 PERSPECTIVE = GLM_MAT4_ZERO_INIT;
    glm_perspective(cam->fov, aspect, NEAR_Z, FAR_Z, PERSPECTIVE);

    vec3 look_at = {0, 0, 1};
    vec3 up = {0, 1, 0};
    glm_vec3_rotate(look_at, cam_rot[2], (vec3){0, 0, 1}); // Roll
    glm_vec3_rotate(look_at, cam_rot[0], (vec3){1, 0, 0}); // Pitch
    glm_vec3_rotate(look_at, cam_rot[1], (vec3){0, 1, 0}); // Yaw

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

void RenderInit() {
    GL_Init();
}

void RenderDestroy() {
    GL_DestroyGL();
}

void RenderLevel3D(Level *l, Camera *cam) {
    GL_Enable3D();

    mat4 *MODELVIEW_MATRIX = GL_GetMatrix(cam);

    for (int i = 0; i < l->staticWalls->size; i++) {
        GL_DrawWall(SizedArrayGet(l->staticWalls, i), MODELVIEW_MATRIX);
    }

    free(MODELVIEW_MATRIX);

    GL_Disable3D();
}
