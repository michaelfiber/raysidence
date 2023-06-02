#include "raylib.h"
#include "button.h"
#include "coolmouse.h"

Button *MakeButton(char *text, Rectangle rec)
{
	Button *b = MemAlloc(sizeof(Button));

	b->color = YELLOW;
	b->fontSize = GetFontDefault().baseSize * 3;
	b->fontSpacing = 2.0f;
	b->text = text;

	SetButtonRec(b, rec);

	return b;
}

void SetButtonRec(Button *button, Rectangle rec)
{
	button->rec = rec;
	button->textOffset = MeasureTextEx(GetFontDefault(), button->text, button->fontSize, button->fontSpacing);
	button->textOffset.x = button->textOffset.x / 2;
	button->textOffset.y = button->textOffset.y / 2;

	button->center.x = button->rec.x + button->rec.width / 2;
	button->center.y = button->rec.y + button->rec.height / 2;
}

bool CheckButton(Button *button)
{
	Color fadedColor = Fade(button->color, 0.25f);
	
	if (CheckCollisionPointRec(GetMousePosition(), button->rec))
	{
		fadedColor = Fade(button->color, 0.45f);
	}

	DrawRectangleRec(button->rec, fadedColor);
	DrawTextPro(GetFontDefault(), button->text, button->center, button->textOffset, 0.0f, button->fontSize, button->fontSpacing, button->color);
	DrawRectangleLines(button->rec.x, button->rec.y, button->rec.width, button->rec.height, button->color);
	return IsCooledMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), button->rec);
}