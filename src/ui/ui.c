#include "ui.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include <string.h>
#include <stdio.h>

// Global font variable
Font gameFont = {0};

void LoadGameFont(void) {
    // Using default font - no loading needed
    gameFont = GetFontDefault();
    TraceLog(LOG_INFO, "Using default font");
}

void InitUI(void) {
    printf("UI system initialized\n");
    LoadGameFont();
}


Button CreateButton(Rectangle bounds, const char* text, void (*onClick)(void)) {
    Button button = {0};
    button.bounds = bounds;
    strncpy(button.text, text, sizeof(button.text) - 1);
    button.text[sizeof(button.text) - 1] = '\0';
    button.isHovered = false;
    button.isPressed = false;
    button.isEnabled = true;
    button.onClick = onClick;
    return button;
}

void UpdateButton(Button* button) {
    if (!button || !button->isEnabled) return;
    
    Vector2 mousePos = GetMousePosition();
    button->isHovered = CheckCollisionPointRec(mousePos, button->bounds);
    
    if (button->isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        button->isPressed = true;
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        if (button->isPressed && button->isHovered && button->onClick) {
            button->onClick();
        }
        button->isPressed = false;
    }
}

void RenderButton(Button* button) {
    if (!button) return;
    
    Color bgColor = UI_BUTTON_NORMAL;
    if (!button->isEnabled) {
        bgColor = (Color){200, 200, 200, 128};
    } else if (button->isPressed) {
        bgColor = UI_BUTTON_ACTIVE;
    } else if (button->isHovered) {
        bgColor = UI_BUTTON_HOVER;
    }
    
    DrawRectangleRec(button->bounds, bgColor);
    DrawRectangleLinesEx(button->bounds, 2, UI_BORDER);
    
    Color textColor = button->isEnabled ? UI_TEXT_PRIMARY : (Color){100, 100, 100, 255};
    
    // Use smaller font size to prevent overflow
    int fontSize = FONT_SIZE_BUTTON;
    int textWidth = MeasureText(button->text, fontSize);
    
    // If text is still too wide, use even smaller font
    if (textWidth > button->bounds.width - 10) {
        fontSize = FONT_SIZE_SMALL;
        textWidth = MeasureText(button->text, fontSize);
        if (textWidth > button->bounds.width - 10) {
            fontSize = 16;
        }
    }
    
    Vector2 textPos = CenterText(button->text, fontSize, button->bounds);
    DrawText(button->text, (int)textPos.x, (int)textPos.y, fontSize, textColor);
}

bool IsButtonClicked(Button* button) {
    return button && button->isPressed && button->isHovered;
}

Vector2 CenterText(const char* text, int fontSize, Rectangle bounds) {
    int textWidth = MeasureText(text, fontSize);
    Vector2 textPos = { bounds.x + (bounds.width - textWidth) / 2,
                    bounds.y + (bounds.height - fontSize) / 2 };
    return textPos;
}

Rectangle CenterRectangle(int width, int height, int screenWidth, int screenHeight) {
    Rectangle rect = {
        (screenWidth - width) / 2,
        (screenHeight - height) / 2,
        width,
        height
    };
    return rect;
}

// 1. First, update your DrawCenteredText function in ui.c to handle float parameters:
void DrawCenteredText(const char* text, float x, float y, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    
    // Actually use the calculated width to center the text
    DrawText(text, x - textWidth/2, y - fontSize/2, fontSize, color);
}

void DrawButtonGrid(Button* buttons, int count, int columns, int startX, int startY) {
    for (int i = 0; i < count; i++) {
        int col = i % columns;
        int row = i / columns;
        
        buttons[i].bounds.x = startX + col * (BUTTON_WIDTH + BUTTON_SPACING);
        buttons[i].bounds.y = startY + row * (BUTTON_HEIGHT + BUTTON_SPACING);
        buttons[i].bounds.width = BUTTON_WIDTH;
        buttons[i].bounds.height = BUTTON_HEIGHT;
        
        UpdateButton(&buttons[i]);
        RenderButton(&buttons[i]);
    }
}