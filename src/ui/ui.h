#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>

// Game font
extern Font gameFont;

// Button structure
typedef struct {
    Rectangle bounds;
    char text[64];
    bool isHovered;
    bool isPressed;
    bool isEnabled;
    void (*onClick)(void);
} Button;

// UI Constants
#define MAX_BUTTONS 10
#define FONT_SIZE_TITLE 48
#define FONT_SIZE_BUTTON 24
#define FONT_SIZE_BODY 20
#define FONT_SIZE_SMALL 16

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 60
#define BUTTON_SPACING 20

// UI Functions
void InitUI(void);
void UpdateUI(void);
void LoadGameFont(void);
void RenderUI(void);
void CleanupUI(void);

// Button functions
Button CreateButton(Rectangle bounds, const char* text, void (*onClick)(void));
void UpdateButton(Button* button);
void RenderButton(Button* button);
bool IsButtonClicked(Button* button);

// Layout helpers
Vector2 CenterText(const char* text, int fontSize, Rectangle bounds);
Rectangle CenterRectangle(int width, int height, int screenWidth, int screenHeight);

// Drawing utilities
// void DrawCenteredText(const char* text, int x, int y, int fontSize, Color color);
void DrawCenteredText(const char* text, float x, float y, int fontSize, Color color);
void DrawButtonGrid(Button* buttons, int count, int columns, int startX, int startY);

#endif // UI_H