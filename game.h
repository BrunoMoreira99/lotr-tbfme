#pragma once
#include <stdint.h>

#include "math_utils.h"
#include "linked_list.h"

#define NO_OWNER _UI8_MAX // 255 / 0xFF

typedef enum {
    PLAIN,
    FOREST,
    MOUNTAIN,
    RIVER,
    WATER,
    BRIDGE,
    SNOW,
    LAVA,
    BASALT
} TerrainType;

typedef enum {
    // Player Colors
    C_GONDOR = 0x0057E9,
    C_MORDOR = 0xE11845,
    // Terrain Colors
    C_PLAIN = 0x84FF70,
    C_FOREST = 0x288440,
    C_MOUNTAIN = 0xACACAC,
    C_RIVER = 0x213B79,
    C_WATER = 0x62CBF7,
    C_BRIDGE = 0xAC8B4A,
    C_SNOW = 0xFFFFFF,
    C_LAVA = 0xE95F1A,
    C_BASALT = 0x33292D
} GameColor;

typedef enum {
    EMPTY_CELL,
    BASE,
    MINES,
    BARRACKS,
    STABLES,
    ARMOURY,
    INFANTRY,
    CAVALRY,
    ARTILLERY
} EntityType;

typedef struct {
    uint8_t id;
    char name[20];
    int32_t coins;
    bool isMordor;
    bool isAlive;
    bool isCPU;
} Player;

typedef struct {
    TerrainType terrainType;
    EntityType entityType;
    uint8_t owner;
    int16_t health;
    bool hasAttackedThisRound;
} GameBoardCell;

typedef struct {
    uint64_t creationTimestamp;
    uint64_t lastSaveTimestamp;
    uint64_t elapsedTimeSeconds;
    uint32_t currentRound;
    uint8_t currentPlayerTurn;
    bool isGameOver;
    GameBoardCell board[17][26];
    uint8_t nPlayers; // This is here for future-proofing. Needed to support games with more than 2 players.
    Player players[];
} GameData;

typedef struct {
    EntityType entityType;
    uint16_t maxHealth;
    uint16_t spawnCost;
    uint16_t moveCost;
    uint16_t attackPower;
} EntityInfo;

typedef struct {
    uint16_t MINE_INCOME;
    // Building Health
    uint16_t BASE_HEALTH;
    uint16_t MINES_HEALTH;
    uint16_t BARRACKS_HEALTH;
    uint16_t STABLES_HEALTH;
    uint16_t ARMOURY_HEALTH;
    // Unit Health
    uint16_t INFANTRY_HEALTH;
    uint16_t CAVALRY_HEALTH;
    uint16_t ARTILLERY_HEALTH;
    // Building Cost
    uint16_t BASE_COST;
    uint16_t MINES_COST;
    uint16_t BARRACKS_COST;
    uint16_t STABLES_COST;
    uint16_t ARMOURY_COST;
    // Unit Spawn Cost
    uint16_t INFANTRY_SPAWN_COST;
    uint16_t CAVALRY_SPAWN_COST;
    uint16_t ARTILLERY_SPAWN_COST;
    // Unit Movement Cost
    uint16_t INFANTRY_MOVEMENT_COST;
    uint16_t CAVALRY_MOVEMENT_COST;
    uint16_t ARTILLERY_MOVEMENT_COST;
    // Unit Attack Power
    uint16_t INFANTRY_ATTACK_POWER;
    uint16_t CAVALRY_ATTACK_POWER;
    uint16_t ARTILLERY_ATTACK_POWER;
    // Other Settings
    bool ENTITIES_MAY_ONLY_ATTACK_ONCE_PER_ROUND;
} GameSettings;

typedef struct {
    GameData* gameData;
    Player* currentPlayer;
    Int16Vector2 currentPlayerBaseCoord;
    List currentPlayerBuildingsList;
    List currentPlayerUnitsList;
    GameSettings* gameSettings;
} GameDataExtended;

extern const GameSettings GameSettings_Default;
extern GameSettings *GameSettings_Current;

GameSettings* getGameSettings(void);

bool loadGame(GameData** gameData, uint8_t saveSlot);

void startNewSinglePlayerGame(uint8_t saveSlot, char* mapFile, const char* playerName, bool player1IsMordor);
void startNewMultiplayerGame(uint8_t saveSlot, char* mapFile, const char* player1Name, const char* player2Name, bool player1IsMordor);
void resumeSaveGame(uint8_t saveSlot);
