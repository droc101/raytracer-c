//
// Created by droc101 on 10/2/24.
//

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include "../defines.h"

/**
 * Create a new camera with default values
 * @return The camera
 */
Camera *CreateCamera();

/**
 * Look at a target with the camera
 * @param camera The camera to look with
 * @param target The target to look at
 */
void CameraLookAt(Camera *camera, Vector2 target);

#endif //GAME_CAMERA_H
