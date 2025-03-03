//
// Created by droc101 on 3/3/25.
//

#ifndef TEXTINPUTSYSTEM_H
#define TEXTINPUTSYSTEM_H
#include <SDL_events.h>

typedef struct TextInput TextInput;

typedef void (TextInputCallback)(TextInput *data, SDL_TextInputEvent *event);

struct TextInput
{
	char *composition;
	int cursor;
	int selection_len;
	void* user_data;
	TextInputCallback *TextInput;
};

/**
 * Start a text input session
 * @param input The text input to start
 */
void SetTextInput(TextInput *input);

/**
 * Stop the current text input session
 */
void StopTextInput();

/**
 * Handle a text input event
 * @param event The text input event to handle
 */
void HandleTextInput(SDL_TextInputEvent *event);

#endif //TEXTINPUTSYSTEM_H
