#include "SaveGame.h"
#include <cstdlib>
#include <iostream>
#include <filesystem>
using namespace std;

// ========================================
// GAMESTATE IMPLEMENTATION
// ========================================

GameState::GameState() 
    : timestamp(0), p1_x(0), p1_y(0), p1_score(0), p1_powerUps(0),
      p1_alive(1), p1_constructing(0), p1_bonusCounter(0), p1_bonusThreshold(10),
      p1_currentMultiplier(1), p2_x(0), p2_y(0), p2_score(0), p2_powerUps(0),
      p2_alive(1), p2_constructing(0), p2_bonusCounter(0), p2_bonusThreshold(10),
      p2_currentMultiplier(1), gameTimer(0), gameMode(2), gameEnded(0),
      tileListHead(nullptr), tileCount(0) {
    saveID[0] = '\0';
    playerID[0] = '\0';
}

GameState::~GameState() {
    clearTiles();
}

void GameState::addTile(int x, int y, int type) {
    TileNode* newNode = new TileNode(x, y, type);
    
    // Append to end of linked list to preserve insertion order
    if (tileListHead == nullptr) {
        tileListHead = newNode;
    } else {
        TileNode* current = tileListHead;
        while (current->next != nullptr) {
            current = current->next;
        }
        current->next = newNode;
    }
    tileCount++;
}

void GameState::clearTiles() {
    TileNode* current = tileListHead;
    while (current) {
        TileNode* temp = current;
        current = current->next;
        delete temp;
    }
    tileListHead = nullptr;
    tileCount = 0;
}

void GameState::printTileList() const {
    cout << "[GameState] Tile list (" << tileCount << " tiles):" << endl;
    TileNode* node = tileListHead;
    int count = 0;
    while (node && count < 10) {
        cout << "  (" << node->x << "," << node->y << ") type=" << node->tileType << endl;
        node = node->next;
        count++;
    }
    if (tileCount > 10) {
        cout << "  ... and " << (tileCount - 10) << " more" << endl;
    }
}

// ========================================
// SAVE/LOAD MANAGER IMPLEMENTATION
// ========================================

SaveLoadManager::SaveLoadManager() {
    filesystem::create_directories("saves");
}

SaveLoadManager::~SaveLoadManager() {
}

void SaveLoadManager::getFilePath(const char* saveID, char* buffer, int bufferSize) {
    snprintf(buffer, bufferSize, "saves/%s.txt", saveID);
}

bool SaveLoadManager::saveExists(const char* saveID) {
    char filepath[256];
    getFilePath(saveID, filepath, 256);
    FILE* file = fopen(filepath, "rb");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

bool SaveLoadManager::deleteSave(const char* saveID) {
    char filepath[256];
    getFilePath(saveID, filepath, 256);
    if (remove(filepath) == 0) {
        cout << "[SaveLoad] Save deleted: " << saveID << endl;
        return true;
    }
    return false;
}

void SaveLoadManager::listAllSaves() {
    cout << "\n[SaveLoad] Available Saves:\n";
    cout << "=====================================\n";
    cout << "Place save files in: saves/\n";
    cout << "Format: <saveID>.sav\n";
    cout << "=====================================\n\n";
}

int SaveLoadManager::listSavesForPlayer(const char* username, SaveInfo* outArray, int maxCount) {
    int saveCount = 0;
    
    try {
        // Check if saves directory exists
        if (!filesystem::exists("saves")) {
            cout << "[SaveLoad] Cannot open saves directory" << endl;
            return 0;
        }
        
        // Iterate through .txt files in saves directory
        for (const auto& entry : filesystem::directory_iterator("saves")) {
            if (saveCount >= maxCount) break;
            
            // Check if it's a file and ends with .txt
            if (entry.is_regular_file()) {
                string filename = entry.path().filename().string();
                int len = filename.length();
                
                if (len >= 5 && filename.substr(len - 4) == ".txt") {
                    // Extract save ID (filename without .txt)
                    string saveID = filename.substr(0, len - 4);
                    
                    // Try to get save info
                    SaveInfo info;
                    if (getSaveInfo(saveID.c_str(), info)) {
                        // Check if it's for this player
                        if (strcmp(info.playerID, username) == 0) {
                            outArray[saveCount] = info;
                            saveCount++;
                        }
                    }
                }
            }
        }
    } catch (const exception& e) {
        cout << "[SaveLoad] Error reading saves directory: " << e.what() << endl;
    }
    
    return saveCount;
}

bool SaveLoadManager::getSaveInfo(const char* saveID, SaveInfo& outInfo) {
    char filepath[256];
    getFilePath(saveID, filepath, 256);
    
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        return false;
    }
    
    char line[512];
    bool foundPlayerID = false;
    bool foundTimestamp = false;
    bool foundP1Score = false;
    bool foundP2Score = false;
    
    // Initialize with safe null-termination
    outInfo.saveID[0] = '\0';
    outInfo.playerID[0] = '\0';
    strncpy(outInfo.saveID, saveID, sizeof(outInfo.saveID) - 1);
    outInfo.saveID[sizeof(outInfo.saveID) - 1] = '\0';
    
    // Parse file line by line looking for key info
    while (fgets(line, 512, file)) {
        int len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        if (strlen(line) == 0) continue;
        
        if (strncmp(line, "=== END SAVE ===", 16) == 0) {
            break;
        }
        
        if (strncmp(line, "PLAYERID:", 9) == 0 && !foundPlayerID) {
            strncpy(outInfo.playerID, line + 9, sizeof(outInfo.playerID) - 1);
            outInfo.playerID[sizeof(outInfo.playerID) - 1] = '\0';
            foundPlayerID = true;
        }
        else if (strncmp(line, "TIMESTAMP:", 10) == 0 && !foundTimestamp) {
            outInfo.timestamp = (time_t)atol(line + 10);
            foundTimestamp = true;
        }
        else if (strncmp(line, "SCORE:", 6) == 0) {
            // In PLAYER1 section
            if (!foundP1Score) {
                outInfo.p1_score = atoi(line + 6);
                foundP1Score = true;
            } else if (!foundP2Score) {
                outInfo.p2_score = atoi(line + 6);
                foundP2Score = true;
            }
        }
        
        // Early exit if we have all the info
        if (foundPlayerID && foundTimestamp && foundP1Score && foundP2Score) {
            break;
        }
    }
    
    fclose(file);
    return foundPlayerID && foundTimestamp;
}

bool SaveLoadManager::saveGame(GameState& state, const char* saveID) {
    // Update timestamp and save ID with safe null-termination
    state.timestamp = time(nullptr);
    strncpy(state.saveID, saveID, sizeof(state.saveID) - 1);
    state.saveID[sizeof(state.saveID) - 1] = '\0';
    
    char filepath[256];
    getFilePath(saveID, filepath, 256);
    
    FILE* file = fopen(filepath, "wb");
    if (!file) {
        cout << "[SaveLoad] ERROR: Cannot open " << filepath << " for writing" << endl;
        return false;
    }
    
    bool success = serializeState(state, file);
    fclose(file);
    
    if (success) {
        cout << "[SaveLoad] Game saved: " << saveID << endl;
    } else {
        cout << "[SaveLoad] Failed to save game" << endl;
    }
    return success;
}

GameState* SaveLoadManager::loadGame(const char* saveID) {
    char filepath[256];
    getFilePath(saveID, filepath, 256);
    
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        cout << "[SaveLoad] ERROR: Save file not found: " << saveID << endl;
        return nullptr;
    }
    
    GameState* state = new GameState();
    bool success = deserializeState(*state, file);
    fclose(file);
    
    if (!success) {
        cout << "[SaveLoad] ERROR: Failed to deserialize" << endl;
        delete state;
        return nullptr;
    }
    
    cout << "[SaveLoad] Game loaded: " << saveID << endl;
    return state;
}

// ========================================
// SERIALIZATION - SAVE TO FILE
// ========================================

bool SaveLoadManager::serializeState(GameState& state, FILE* file) {
    // Header
    fprintf(file, "=== XONIX SAVE FILE ===\n");
    fprintf(file, "VERSION:1\n");
    fprintf(file, "\n");
    
    // Metadata (REQUIRED BY SPECS)
    fprintf(file, "[METADATA]\n");
    fprintf(file, "SAVEID:%s\n", state.saveID);
    fprintf(file, "PLAYERID:%s\n", state.playerID);
    fprintf(file, "TIMESTAMP:%ld\n", state.timestamp);
    fprintf(file, "\n");
    
    // Game State
    fprintf(file, "[GAME]\n");
    fprintf(file, "GAMEMODE:%d\n", state.gameMode);
    fprintf(file, "GAMETIMER:%d\n", state.gameTimer);
    fprintf(file, "GAMEENDED:%d\n", state.gameEnded);
    fprintf(file, "\n");
    
    // Player 1
    fprintf(file, "[PLAYER1]\n");
    fprintf(file, "X:%d\n", state.p1_x);
    fprintf(file, "Y:%d\n", state.p1_y);
    fprintf(file, "SCORE:%d\n", state.p1_score);
    fprintf(file, "POWERUPS:%d\n", state.p1_powerUps);
    fprintf(file, "ALIVE:%d\n", state.p1_alive);
    fprintf(file, "CONSTRUCTING:%d\n", state.p1_constructing);
    fprintf(file, "BONUSCOUNTER:%d\n", state.p1_bonusCounter);
    fprintf(file, "BONUSTHRESHOLD:%d\n", state.p1_bonusThreshold);
    fprintf(file, "MULTIPLIER:%d\n", state.p1_currentMultiplier);
    fprintf(file, "\n");
    
    // Player 2
    fprintf(file, "[PLAYER2]\n");
    fprintf(file, "X:%d\n", state.p2_x);
    fprintf(file, "Y:%d\n", state.p2_y);
    fprintf(file, "SCORE:%d\n", state.p2_score);
    fprintf(file, "POWERUPS:%d\n", state.p2_powerUps);
    fprintf(file, "ALIVE:%d\n", state.p2_alive);
    fprintf(file, "CONSTRUCTING:%d\n", state.p2_constructing);
    fprintf(file, "BONUSCOUNTER:%d\n", state.p2_bonusCounter);
    fprintf(file, "BONUSTHRESHOLD:%d\n", state.p2_bonusThreshold);
    fprintf(file, "MULTIPLIER:%d\n", state.p2_currentMultiplier);
    fprintf(file, "\n");
    
    // Tile List (REQUIRED BY SPECS)
    fprintf(file, "[TILELIST]\n");
    fprintf(file, "TILECOUNT:%d\n", state.tileCount);
    fprintf(file, "\n");
    
    TileNode* node = state.tileListHead;
    while (node) {
        fprintf(file, "TILE:%d,%d,%d\n", node->x, node->y, node->tileType);
        node = node->next;
    }
    
    fprintf(file, "\n=== END SAVE ===\n");
    return true;
}

// ========================================
// DESERIALIZATION - LOAD FROM FILE
// ========================================

bool SaveLoadManager::deserializeState(GameState& state, FILE* file) {
    // Clear any existing tiles to prevent memory leak when loading multiple saves
    state.clearTiles();
    
    char line[512];
    int playerSection = 0;  // Track which player section we're in
    int expectedTileCount = 0;
    int actualTileCount = 0;
    
    // Read and validate header
    if (!fgets(line, 512, file) || strncmp(line, "=== XONIX SAVE FILE ===", 23) != 0) {
        cout << "[SaveLoad] ERROR: Invalid file header" << endl;
        return false;
    }
    
    // Read and validate VERSION line
    if (!fgets(line, 512, file) || strncmp(line, "VERSION:", 8) != 0) {
        cout << "[SaveLoad] ERROR: Missing or invalid VERSION line" << endl;
        return false;
    }
    int fileVersion = atoi(line + 8);
    if (fileVersion != 1) {
        cout << "[SaveLoad] ERROR: Unsupported save file version: " << fileVersion << " (expected 1)" << endl;
        return false;
    }
    
    // Parse line by line
    while (fgets(line, 512, file)) {
        // Remove trailing newline
        int len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Skip empty lines
        if (strlen(line) == 0) continue;
        
        if (strncmp(line, "=== END SAVE ===", 16) == 0) {
            break;
        }
        
        // Track sections
        if (strcmp(line, "[PLAYER1]") == 0) {
            playerSection = 1;
            continue;
        }
        if (strcmp(line, "[PLAYER2]") == 0) {
            playerSection = 2;
            continue;
        }
        if (strcmp(line, "[METADATA]") == 0 || strcmp(line, "[GAME]") == 0 || strcmp(line, "[TILELIST]") == 0) {
            playerSection = 0;
            continue;
        }
        
        // Parse metadata
        if (strncmp(line, "SAVEID:", 7) == 0) {
            strncpy(state.saveID, line + 7, sizeof(state.saveID) - 1);
            state.saveID[sizeof(state.saveID) - 1] = '\0';
        }
        else if (strncmp(line, "PLAYERID:", 9) == 0) {
            strncpy(state.playerID, line + 9, sizeof(state.playerID) - 1);
            state.playerID[sizeof(state.playerID) - 1] = '\0';
        }
        else if (strncmp(line, "TIMESTAMP:", 10) == 0) {
            state.timestamp = (time_t)atol(line + 10);
        }
        else if (strncmp(line, "GAMEMODE:", 9) == 0) {
            state.gameMode = atoi(line + 9);
        }
        else if (strncmp(line, "GAMETIMER:", 10) == 0) {
            state.gameTimer = atoi(line + 10);
        }
        else if (strncmp(line, "GAMEENDED:", 10) == 0) {
            state.gameEnded = atoi(line + 10);
        }
        // Player 1 data
        else if (playerSection == 1 && strncmp(line, "X:", 2) == 0) {
            state.p1_x = atoi(line + 2);
        }
        else if (playerSection == 1 && strncmp(line, "Y:", 2) == 0) {
            state.p1_y = atoi(line + 2);
        }
        else if (playerSection == 1 && strncmp(line, "SCORE:", 6) == 0) {
            state.p1_score = atoi(line + 6);
        }
        else if (playerSection == 1 && strncmp(line, "POWERUPS:", 9) == 0) {
            state.p1_powerUps = atoi(line + 9);
        }
        else if (playerSection == 1 && strncmp(line, "ALIVE:", 6) == 0) {
            state.p1_alive = atoi(line + 6);
        }
        else if (playerSection == 1 && strncmp(line, "CONSTRUCTING:", 13) == 0) {
            state.p1_constructing = atoi(line + 13);
        }
        else if (playerSection == 1 && strncmp(line, "BONUSCOUNTER:", 13) == 0) {
            state.p1_bonusCounter = atoi(line + 13);
        }
        else if (playerSection == 1 && strncmp(line, "BONUSTHRESHOLD:", 15) == 0) {
            state.p1_bonusThreshold = atoi(line + 15);
        }
        else if (playerSection == 1 && strncmp(line, "MULTIPLIER:", 11) == 0) {
            state.p1_currentMultiplier = atoi(line + 11);
        }
        // Player 2 data
        else if (playerSection == 2 && strncmp(line, "X:", 2) == 0) {
            state.p2_x = atoi(line + 2);
        }
        else if (playerSection == 2 && strncmp(line, "Y:", 2) == 0) {
            state.p2_y = atoi(line + 2);
        }
        else if (playerSection == 2 && strncmp(line, "SCORE:", 6) == 0) {
            state.p2_score = atoi(line + 6);
        }
        else if (playerSection == 2 && strncmp(line, "POWERUPS:", 9) == 0) {
            state.p2_powerUps = atoi(line + 9);
        }
        else if (playerSection == 2 && strncmp(line, "ALIVE:", 6) == 0) {
            state.p2_alive = atoi(line + 6);
        }
        else if (playerSection == 2 && strncmp(line, "CONSTRUCTING:", 13) == 0) {
            state.p2_constructing = atoi(line + 13);
        }
        else if (playerSection == 2 && strncmp(line, "BONUSCOUNTER:", 13) == 0) {
            state.p2_bonusCounter = atoi(line + 13);
        }
        else if (playerSection == 2 && strncmp(line, "BONUSTHRESHOLD:", 15) == 0) {
            state.p2_bonusThreshold = atoi(line + 15);
        }
        else if (playerSection == 2 && strncmp(line, "MULTIPLIER:", 11) == 0) {
            state.p2_currentMultiplier = atoi(line + 11);
        }
        // Parse tiles (REQUIRED BY SPECS)
        else if (strncmp(line, "TILECOUNT:", 10) == 0) {
            expectedTileCount = atoi(line + 10);
        }
        else if (strncmp(line, "TILE:", 5) == 0) {
            int x, y, type;
            sscanf(line + 5, "%d,%d,%d", &x, &y, &type);
            state.addTile(x, y, type);
            actualTileCount++;
        }
        // Fix 6: Unknown line debugging
        else {
            cout << "[SaveLoad] WARNING: Unknown line in save file: " << line << endl;
        }
    }
    
    // Validate tile count
    if (expectedTileCount > 0 && actualTileCount != expectedTileCount) {
        cout << "[SaveLoad] WARNING: Tile count mismatch! Expected " << expectedTileCount << ", got " << actualTileCount << endl;
    }
    
    return true;
}


