//
// Created by droc101 on 10/2/24.
//

#include "Camera.h"
#include <math.h>

/**
 * Create a camera with default values
 * @return A pointer to the camera
 */
Camera *CreateCamera()
{
    Camera *camera = malloc(sizeof(Camera));
    camera->x = 0;
    camera->y = 0;
    camera->z = 0;
    camera->pitch = 0;
    camera->yaw = 0;
    camera->roll = 0;
    camera->fov = FOV;
    return camera;
}

/**
 * Look at a target. The XY of the target vector is XZ in 3d, with Y=0
 * @param camera The camera to look with
 * @param target The target to look at
 */
void CameraLookAt(Camera *camera, const Vector2 target)
{
    camera->yaw = atan2(target.x - camera->x, target.y - camera->z);
    camera->pitch = 0;
}
