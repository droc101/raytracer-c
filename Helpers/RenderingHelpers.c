//
// Created by droc101 on 10/2/24.
//

#include "RenderingHelpers.h"
#include "GL/glHelper.h"

mat4 *GL_GetMatrix(Camera *cam) {
    vec3 cam_pos = {cam->x, cam->y, cam->z};
    mat4 MODEL_MATRIX = GLM_MAT4_IDENTITY_INIT;
    mat4 VIEW_MATRIX = GLM_MAT4_ZERO_INIT;
    glm_rotate(MODEL_MATRIX, cam->pitch, (vec3) {0, 1, 0});
    glm_rotate(MODEL_MATRIX, cam->yaw, (vec3) {1, 0, 0});
    glm_rotate(MODEL_MATRIX, cam->roll, (vec3) {0, 0, 1});
    glm_translate(MODEL_MATRIX, cam_pos);

    float aspect = (float)WindowWidth() / (float)WindowHeight();

    mat4 PROJECTION_MATRIX = GLM_MAT4_ZERO_INIT;
    glm_perspective(cam->fov, aspect, NEAR_Z, FAR_Z, PROJECTION_MATRIX);

    mat4 *MODEL_VIEW_PROJECTION = malloc(sizeof(mat4));
    glm_mat4_mul(PROJECTION_MATRIX, VIEW_MATRIX, *MODEL_VIEW_PROJECTION);
    glm_mat4_mul(*MODEL_VIEW_PROJECTION, MODEL_MATRIX, *MODEL_VIEW_PROJECTION);

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

    GL_Disable3D();
}
