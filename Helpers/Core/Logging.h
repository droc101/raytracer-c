//
// Created by droc101 on 11/5/24.
//

#ifndef GAME_LOGGING_H
#define GAME_LOGGING_H

//#define FLUSH_ON_INFO
//#define FLUSH_ON_DEBUG
#define FLUSH_ON_WARNING
#define FLUSH_ON_ERROR

void LogInfo(const char *str, ...);

void LogDebug(const char *str, ...);

void LogWarning(const char *str, ...);

void LogError(const char *str, ...);

#endif //GAME_LOGGING_H
