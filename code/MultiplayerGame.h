

#ifndef MULTIPLAYER_GAME_H
#define MULTIPLAYER_GAME_H

#include <string>
#include "SaveGame.h"
using namespace std;


struct MPEnemy {
    int x, y, dx, dy;
    
    MPEnemy() : x(300), y(300), dx(2), dy(2) {}
    
    void move(int grid[25][40], int M, int N, int ts) {
        x += dx;
        if (grid[y / ts][x / ts] == 1) { dx = -dx; x += dx; }
        y += dy;
        if (grid[y / ts][x / ts] == 1) { dy = -dy; y += dy; }
    }
};


struct GamePlayer {
    int x, y;               // position in grid coords
    int dx, dy;             // direction
    int score;              // session score (for this round)
    int powerUps;           // individual power-ups
    bool alive;             // alive status
    bool constructing;      // is constructing tiles? (true ONLY when moving on empty (0))
    
    int bonusCounter;       // How many times bonus was triggered (3 or 5)
    int bonusThreshold;     // Tiles needed for 2x bonus (starts 10, becomes 5 after 3 bonuses)
    int currentMultiplier;  // 1, 2, or 4
    
    GamePlayer() : x(0), y(0), dx(0), dy(0), score(0), powerUps(0), alive(true), constructing(false),
                   bonusCounter(0), bonusThreshold(10), currentMultiplier(1) {}
};

class MultiplayerGame {
private:
    GamePlayer player1;
    GamePlayer player2;
    int grid[25][40];   
    const int M = 25;
    const int N = 40;
    const int ts = 18; 
    float gameTimer;
    bool p1Frozen;
    bool p2Frozen;
    float p1FreezeTime;
    float p2FreezeTime;
    const float FREEZE_DURATION = 3.0f;
    float moveTimer;
    const float MOVE_DELAY = 0.07f;  
    MPEnemy enemies[10];
    int enemyCount;
    bool enemiesFrozen;
    float enemiesFreezeTime;
    string deathMessage;
    float deathMessageTimer;
    
public:
    static const float DEATH_MESSAGE_DURATION;  
    MultiplayerGame();
    void initializeGame();
    void initializeGrid();
    void initializeEnemies();
    void setPlayer1Controls(int dx, int dy);
    void setPlayer2Controls(int dx, int dy);
    void update(float deltaTime);
    void movePlayer(int playerIdx);
    void moveEnemies();
    void handlePlayerCollisions();
    void handleEnemyCollisions();
    void dropFloodFill(int y, int x);  
    bool usePowerUp(int playerIdx);
    void freezeAll(int playerIdx);
    void updateFreezeStates(float deltaTime);
    bool isPlayerFrozen(int playerIdx) const;
    bool areEnemiesFrozen() const { return enemiesFrozen; }
    void awardScore(int playerIdx, int capturedTiles);
    void updatePowerupRewards(int playerIdx);
    const GamePlayer& getPlayer1() const { return player1; }
    const GamePlayer& getPlayer2() const { return player2; }
    const MPEnemy* getEnemies() const { return enemies; }
    int getEnemyCount() const { return enemyCount; }
    int getGrid(int y, int x) const { return grid[y][x]; }
    float getGameTimer() const { return gameTimer; }
    bool isPlayer1Alive() const { return player1.alive; }
    bool isPlayer2Alive() const { return player2.alive; }
    bool gameEnded() const { return !player1.alive && !player2.alive; }
    int getWinner() const;
   string getDeathMessage() const { return deathMessage; }
    float getDeathMessageTimer() const { return deathMessageTimer; }
    void setPlayer1PowerUps(int count) { player1.powerUps = count; }
    void setPlayer2PowerUps(int count) { player2.powerUps = count; }
    GameState* captureGameState(const char* saveID, const char* username);
    bool restoreGameState(const GameState* state);
    void recordTileInSave(GameState* state, int x, int y, int type);
};

#endif
