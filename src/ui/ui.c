#include "ui.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include <string.h>
#include <stdio.h>

// Global font variable
Font gameFont = {0};

void LoadGameFont(void) {
    // Load the Mecha font
    // gameFont = LoadFont("resources/sprite_fonts/alpha_beta.png");
    gameFont = GetFontDefault();
    if (gameFont.texture.id == 0) {
        TraceLog(LOG_WARNING, "Failed to load Mecha font, falling back to default font");
        gameFont = GetFontDefault();
    } else {
        TraceLog(LOG_INFO, "Mecha font loaded successfully");
    }
}

void InitUI(void) {
    printf("UI system initialized\n");
    LoadGameFont();
}

void UpdateUI(void) {
    // UI updates are handled in game.c for now
    // This could be expanded for more complex UI interactions
}

void RenderUI(void) {
    // UI rendering is handled in game.c for now
    // This could be expanded for overlay UI elements
}

void CleanupUI(void) {
    if (gameFont.texture.id != 0 && gameFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(gameFont);
    }
    printf("UI system cleaned up\n");
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
    Vector2 textSize = MeasureTextEx(gameFont, button->text, fontSize, 1);
    
    // If text is still too wide, use even smaller font
    if (textSize.x > button->bounds.width - 10) {
        fontSize = FONT_SIZE_SMALL;
        textSize = MeasureTextEx(gameFont, button->text, fontSize, 1);
        if (textSize.x > button->bounds.width - 10) {
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
    Vector2 textSize = MeasureTextEx(gameFont, text, fontSize, 1);
    Vector2 textPos = { bounds.x + (bounds.width - textSize.x) / 2,
                    bounds.y + (bounds.height - textSize.y) / 2 };
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

// void DrawCenteredText(const char* text, int x, int y, int fontSize, Color color) {
//     Vector2 textSize = MeasureTextEx(gameFont, text, fontSize, 1);
//     DrawTextEx(gameFont, text, (Vector2){x - textSize.x/2, y - textSize.y/2}, fontSize, 1, color);
// }

// 1. First, update your DrawCenteredText function in ui.c to handle float parameters:
void DrawCenteredText(const char* text, float x, float y, int fontSize, Color color) {
    Vector2 textSize = MeasureTextEx(gameFont, text, fontSize, 1);
    float drawX = x - textSize.x / 2.0f;
    float drawY = y - textSize.y / 2.0f;
    
    // Debug output to see what's happening
    printf("DrawCenteredText: text='%s', pos=(%.1f,%.1f), textSize=(%.1f,%.1f), drawPos=(%.1f,%.1f)\n", 
           text, x, y, textSize.x, textSize.y, drawX, drawY);
    
    DrawTextEx(gameFont, text, (Vector2){drawX, drawY}, fontSize, 1, color);
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