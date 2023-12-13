#pragma once

typedef enum {
    BLACK = 0x000000,
    WHITE = 0xFFFFFF,
    RED = 231 << 16 | 72 << 8 | 86, // 0xE74856
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
    C_LAVA = 0xE95F1A
} Color;

typedef struct {
    const char* text;
    const uint16_t consoleRow;
    const uint16_t consoleColumn;
} MenuOption;

void printCenteredText(const char* str);

void drawFullWidthBoxTitle(const char* title);
void drawRoundedBox(const uint16_t width, const uint16_t height);
void drawBoldBox(const uint16_t width, const uint16_t height);
void drawBox(const uint16_t width, const uint16_t height);

char* readLine(char* str, const size_t maxSize, const bool allowEmpty);
char* fReadLine(FILE* fp);
