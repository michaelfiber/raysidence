#include "dialog.h"
#include "raylib.h"
#include "string.h"
#include "button.h"

bool initialized = false;

Color fadedBlack = BLACK;
Color fadedGreen = GREEN;
Vector2 cursorSize = (Vector2){0, 0};
float cursorTimer = 0.0f;
bool cursorVisible = true;

Button *okButton = NULL;
Button *cancelButton = NULL;

/**
 * @brief Display a full screen dialog with a prompt that is centered horizontally and vertically. The prompt is very basic with OK and Cancel buttons. 
 * 
 * @param prompt What the user sees, centered below the entry line.
 * @param out Variable used to store the content of the entry line.
 * @param max Maximum length of entry.
 * @param shouldClose If set to true, the dialog is ready to be closed immediately without processing the text input.
 * @return true - The user has pressed enter or clicked Ok to accept the text entry.
 * @return false - The dialog is not done yet.
 */
bool Dialog(const char *prompt, char *out, int max, bool *shouldClose)
{
	// Place initialization logic here that should be run the first time a Dialog is opened.
	if (!initialized)
	{
		fadedBlack = Fade(BLACK, 0.5f);
		fadedGreen = Fade(GREEN, 0.6f);
		cursorSize = MeasureTextEx(GetFontDefault(), "W", GetFontDefault().baseSize * 3, 2.0f);
		initialized = true;
	}

	*shouldClose = false;

	if (okButton == NULL)
	{
		okButton = MakeButton("ok", (Rectangle){
										GetScreenWidth() / 2 - 95,
										GetScreenHeight() / 2 + 100,
										90, 30});
	}

	if (cancelButton == NULL)
	{
		cancelButton = MakeButton("cancel", (Rectangle){
												GetScreenWidth() / 2 + 45,
												GetScreenHeight() / 2 + 100,
												90, 30});
	}

	int key = GetKeyPressed();
	if (key > 0)
	{
		int i = strlen(out);

		// If the shift key is not pressed, add 32 to convert to lowercase. This could easily break stuff
		// but is intentionally simple right now because it is for typing in room name info and room names
		// are currently [A-Za-z0-9\s] only.
		if (key >= 65 && key <= 96 && !IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_RIGHT_SHIFT))
		{
			key += 32;
		}

		switch (key)
		{
		case KEY_LEFT_SHIFT:
		case KEY_RIGHT_SHIFT:
			break;
		case KEY_BACKSPACE:
			if (i > 0 && i < max)
			{
				out[i - 1] = '\0';
			}
			break;
		case KEY_ENTER:
		case KEY_KP_ENTER:
			return true;
			break;
		case KEY_ESCAPE:
			*shouldClose = true;
			return false;
			break;
		default:
			if (i < 32)
			{
				out[i] = key;
			}
			break;
		}
	}

	Vector2 currentTextSize = MeasureTextEx(GetFontDefault(), out, GetFontDefault().baseSize * 3.0f, 2.0f);

	cursorTimer -= GetFrameTime();
	if (cursorTimer < 0)
	{
		cursorTimer = 0.5f;
		cursorVisible = !cursorVisible;
	}

	Vector2 textOffset = MeasureTextEx(GetFontDefault(), prompt, GetFontDefault().baseSize * 2, 2.0f);
	textOffset.x = textOffset.x / 2;
	textOffset.y = textOffset.y / 2 - textOffset.y;

	Vector2 cursorPosition = (Vector2){GetScreenWidth() / 2 - 150 + currentTextSize.x, GetScreenHeight() / 2 + 5 - cursorSize.y};

	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), fadedBlack);
	DrawTextPro(GetFontDefault(), prompt, (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2}, textOffset, 0.0f, GetFontDefault().baseSize * 2, 2.0f, fadedGreen);
	DrawLine(GetScreenWidth() / 2 - 160, GetScreenHeight() / 2 + 10, GetScreenWidth() / 2 + 160, GetScreenHeight() / 2 + 10, fadedGreen);
	DrawTextEx(GetFontDefault(), out, (Vector2){GetScreenWidth() / 2 - 150, GetScreenHeight() / 2 - currentTextSize.y + 5}, GetFontDefault().baseSize * 3, 2.0f, GREEN);

	if (cursorVisible)
	{
		DrawRectanglePro((Rectangle){
							 cursorPosition.x,
							 cursorPosition.y,
							 cursorSize.x,
							 cursorSize.y},
						 (Vector2){0, 0}, 0.0f, fadedGreen);
	}

	if (CheckButton(okButton))
	{
		return true;
	}

	if (CheckButton(cancelButton))
	{
		TraceLog(LOG_INFO, "cancel button pressed");
		*shouldClose = true;
		return false;
	}

	return false;
}