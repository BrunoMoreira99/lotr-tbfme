#pragma once

#define MENU_BACK INT32_MAX
#define MENU_FOOTER_GO_BACK "Press ESC to Go Back"
#define MENU_FOOTER_CANCEL "Press ESC to Cancel"

typedef enum {
    KEY_ENTER = 13,
    KEY_ESC = 27,
    KEY_SPACE = 32,
    KEY_UP = 72,
    KEY_LEFT = 75,
    KEY_RIGHT = 77,
    KEY_DOWN = 80,
    KEY_W = 119,
    KEY_A = 97,
    KEY_S = 115,
    KEY_D = 100
} KeyCode;

typedef enum {
    BLACK = 0x000000,
    WHITE = 0xFFFFFF,
    RED = 231 << 16 | 72 << 8 | 86, // 0xE74856
    GREEN = 0x98C379,
    // Player Colors
    C_GONDOR = 0x213B79,
    C_MORDOR = 0xE74856,
    // Terrain Colors
    C_PLAIN = 0x84FF70,
    C_FOREST = 0x288440,
    C_MOUNTAIN = 0xACACAC,
    C_RIVER = 0x62CBF7,
    C_WATER = 0x213B79,
    C_BRIDGE = 0xAC8B4A,
    C_SNOW = 0xFFFFFF,
    C_LAVA = 0xE95F1A,
    C_BASALT = 0x33292D
} Color;

typedef struct {
    const char* text;
    const int16_t rowOffset;
} MenuOption;

typedef struct {
    const char* text;
    const uint16_t castarCoinCost;
    const uint8_t row;
    const uint16_t consoleColumn;
} ActionMenuOption;

void printCenteredText(const char* str);

void drawFullWidthBoxTitle(const char* title);
void drawRoundedBox(const uint16_t width, const uint16_t height);
void drawBoldBox(const uint16_t width, const uint16_t height);
void drawBox(const uint16_t width, const uint16_t height);
void drawBoxWithTitleAndFooter(const char* title, const char* footer, const uint16_t width, const uint16_t height, void (*boxDrawingFunc)(const uint16_t, const uint16_t));

char* readLine(char* str, const size_t maxSize, const bool allowEmpty);
char* fReadLine(FILE* fp);
