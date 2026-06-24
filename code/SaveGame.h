#ifndef SAVE_GAME_H
#define SAVE_GAME_H

#include <ctime>
#include <cstring>
#include <cstdio>

// ========================================
// TILE NODE - Linked List for Game Tiles
// ========================================
struct TileNode {
    int x, y;               // Grid coordinates
    int tileType;           // 0=empty, 1=border, 2=P1trail, 3=P2trail, etc
    TileNode* next;         // Pointer to next node
    
    TileNode(int _x, int _y, int _type = 0) 
        : x(_x), y(_y), tileType(_type), next(nullptr) {}
};

// ========================================
// GAME STATE - Complete Game Snapshot
// ========================================
struct GameState {
    // Metadata (REQUIRED BY SPECS)
    char saveID[64];            // Unique save identifier
    char playerID[64];          // Player username or ID
    time_t timestamp;           // Time of save
    
    // PLAYER 1 STATE
    int p1_x, p1_y;             // Position
    int p1_score;               // Score
    int p1_powerUps;            // Power-ups count
    int p1_alive;               // Alive status (0/1)
    int p1_constructing;        // Constructing flag (0/1)
    int p1_bonusCounter;        // Bonus counter
    int p1_bonusThreshold;      // Bonus threshold
    int p1_currentMultiplier;   // Current multiplier
    
    // PLAYER 2 STATE
    int p2_x, p2_y;
    int p2_score;
    int p2_powerUps;
    int p2_alive;
    int p2_constructing;
    int p2_bonusCounter;
    int p2_bonusThreshold;
    int p2_currentMultiplier;
    
    // GAME META
    int gameTimer;              // Elapsed time
    int gameMode;               // 1=single, 2=multi
    int gameEnded;              // Is game over (0/1)
    
    // TILE LINKED LIST (REQUIRED BY SPECS)
    TileNode* tileListHead;     // Head of linked list
    int tileCount;              // Number of tiles in list
    
    // Constructor
    GameState();
    
    // Destructor - CRITICAL for cleanup
    ~GameState();
    
    // Tile linked list operations
    void addTile(int x, int y, int type);
    void clearTiles();
    void printTileList() const;
};

// ========================================
// SAVE INFO - For displaying save details
// ========================================
struct SaveInfo {
    char saveID[64];
    char playerID[64];
    time_t timestamp;
    int p1_score;
    int p2_score;
    
    SaveInfo() {
        saveID[0] = '\0';
        playerID[0] = '\0';
        timestamp = 0;
        p1_score = 0;
        p2_score = 0;
    }
};

// ========================================
// SAVE/LOAD MANAGER - File I/O
// ========================================
class SaveLoadManager {
public:
    SaveLoadManager();
    ~SaveLoadManager();
    
    // Save game state to file
    bool saveGame(GameState& state, const char* saveID);
    
    // Load game state from file
    GameState* loadGame(const char* saveID);
    
    // Check if save exists
    bool saveExists(const char* saveID);
    
    // Delete a save
    bool deleteSave(const char* saveID);
    
    // List all saves for a player (by username)
    int listSavesForPlayer(const char* username, SaveInfo* outArray, int maxCount);
    
    // List all saves in general
    void listAllSaves();
    
    // Get file path for save ID
    void getFilePath(const char* saveID, char* buffer, int bufferSize);
    
    // Get save info without loading full state
    bool getSaveInfo(const char* saveID, SaveInfo& outInfo);
    
private:
    // Serialization helpers
    bool serializeState(GameState& state, FILE* file);
    bool deserializeState(GameState& state, FILE* file);
};

#endif
