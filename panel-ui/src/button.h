#include "raylib.h"

#ifndef BUTTON_H
#define BUTTON_H

typedef struct
{
	Rectangle rec;
	char *text;
	
	// Usually this will equal the result of MeasureTextEx divided by 2.0f.
	Vector2 textOffset;

	Vector2 center;

	// This is for the text and the border of the button.
	Color color;

	float fontSize;
	float fontSpacing;
} Button;

Button *MakeButton(char *text, Rectangle rec);
void SetButtonRec(Button *button, Rectangle rec);
bool CheckButton(Button *button);

#endif