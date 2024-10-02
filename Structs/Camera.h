//
// Created by droc101 on 10/2/24.
//

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include "../defines.h"

Camera *CreateCamera();

void CameraLookAt(Camera *camera, Vector2 target);

#endif //GAME_CAMERA_H
