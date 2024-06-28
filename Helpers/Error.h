//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_ERROR_H
#define GAME_ERROR_H

#ifndef __OPTIMIZE__
#define Error(error) _Error_Internal(error, __FILE_NAME__, __LINE__, __func__)
#else
#define Error(error) _Error_Internal(error, "none", 0, "none")
#endif

// Print an error message on screen and lock up
_Noreturn void _Error_Internal(char* error, const char* file, int line, const char* function);

#endif //GAME_ERROR_H
