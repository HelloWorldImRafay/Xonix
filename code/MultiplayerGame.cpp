
#include "MultiplayerGame.h"
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>

const float MultiplayerGame::DEATH_MESSAGE_DURATION = 2.0f;

MultiplayerGame::MultiplayerGame()
    : gameTimer(0), p1Frozen(false), p2Frozen(false), p1FreezeTime(0), p2FreezeTime(0), 
      moveTimer(0), enemyCount(4), enemiesFrozen(false), enemiesFreezeTime(0) {
    initializeGame();
}

void MultiplayerGame::initializeGame() {
    player1 = GamePlayer();
    player2 = GamePlayer();
    
    initializeGrid();
    
    player1.x = N / 2;      
    player1.y = 1;          
    
    player2.x = N / 2;      
    player2.y = M - 2;      
    
    initializeEnemies();
    
    gameTimer = 0;
    p1Frozen = false;
    p2Frozen = false;
    p1FreezeTime = 0;
    p2FreezeTime = 0;
    moveTimer = 0;
    enemiesFrozen = false;
    enemiesFreezeTime = 0;
    deathMessage = "";
    deathMessageTimer = DEATH_MESSAGE_DURATION;
    
    fprintf(stderr, "Game initialized: P1 at (%d,%d), P2 at (%d,%d)\n",
            player1.x, player1.y, player2.x, player2.y);
}

void MultiplayerGame::initializeGrid() {
    // Initialize with borders (1) and empty space (0)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1) {
                grid[i][j] = 1;  // border
            } else {
                grid[i][j] = 0;  // empty
            }
        }
    }
}

void MultiplayerGame::initializeEnemies() {
    for (int i = 0; i < enemyCount; i++) {
        enemies[i] = MPEnemy();
        enemies[i].x = 300;  
        enemies[i].y = 300;
        enemies[i].dx = 4 - (rand() % 8);
        enemies[i].dy = 4 - (rand() % 8);
    }
}

void MultiplayerGame::setPlayer1Controls(int dx, int dy) {
    if (!p1Frozen) {
        player1.dx = dx;
        player1.dy = dy;
    } else {
        player1.dx = 0;
        player1.dy = 0;
    }
}

void MultiplayerGame::setPlayer2Controls(int dx, int dy) {
    if (!p2Frozen) {
        player2.dx = dx;
        player2.dy = dy;
    } else {
        player2.dx = 0;
        player2.dy = 0;
    }
}

void MultiplayerGame::update(float deltaTime) {
    gameTimer += deltaTime;
    moveTimer += deltaTime;
    deathMessageTimer += deltaTime;  
    
    if (!player1.alive && !player2.alive) {
        updateFreezeStates(deltaTime);
        return;  
    }
    
    updateFreezeStates(deltaTime);
    
    if (moveTimer < MOVE_DELAY) {
        return;
    }
    moveTimer = 0;
    
    movePlayer(0);  
    movePlayer(1);  
    
    moveEnemies();
    
    handlePlayerCollisions();
    handleEnemyCollisions();
}

void MultiplayerGame::movePlayer(int playerIdx) {
    GamePlayer& player = (playerIdx == 0) ? player1 : player2;
    GamePlayer& opponent = (playerIdx == 0) ? player2 : player1;
    
   
    
    if (!player.alive) {
        return; 
    }
    
    if ((playerIdx == 0 && p1Frozen) || (playerIdx == 1 && p2Frozen)) {
        player.dx = 0;
        player.dy = 0;
        player.constructing = false;
        return;
    }
    
    bool isMoving = (player.dx != 0 || player.dy != 0);
    if (!isMoving) {
        player.constructing = false;
        return;
    }
    
    
    
    int newX = player.x + player.dx;
    int newY = player.y + player.dy;
    
    if (newX < 0 || newX >= N || newY < 0 || newY >= M) {
        return;
    }
    
    int cellContent = grid[newY][newX];
    int trailMarker = (playerIdx == 0) ? 2 : 3;
    int opponentTrail = (playerIdx == 0) ? 3 : 2;
    
    if (cellContent == opponentTrail && opponent.constructing) {
        player.alive = false;
        deathMessage = (playerIdx == 0) ? "Player 1 Eliminated!" : "Player 2 Eliminated!";
        deathMessageTimer = 0;
        
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == trailMarker) {
                    grid[i][j] = 0;
                }
            }
        }
        return;
    }
    
    if (newX == opponent.x && newY == opponent.y) {
        if (player.constructing && opponent.constructing) {
            player.alive = false;
            opponent.alive = false;
            deathMessage = "Both Eliminated!";
            deathMessageTimer = 0;
            return;
        }
        
        if (player.constructing && !opponent.constructing) {
            player.alive = false;
            deathMessage = (playerIdx == 0) ? "Player 1 Eliminated!" : "Player 2 Eliminated!";
            deathMessageTimer = 0;
            return;
        }
        
    
    }
    
    
    bool isBorder = (newX == 0 || newX == N-1 || newY == 0 || newY == M-1);
    
    if (isBorder && cellContent == 1) {
        player.constructing = false;
        
        int prevGrid[M][N];
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                prevGrid[i][j] = grid[i][j];
            }
        }
        
   
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == trailMarker) {
                    grid[i][j] = 9;  
                }
            }
        }
        
        
        for (int i = 0; i < enemyCount; i++) {
            int ex = enemies[i].x / ts;
            int ey = enemies[i].y / ts;
            if (ex >= 0 && ex < N && ey >= 0 && ey < M) {
                if (grid[ey][ex] == 0) {
                    dropFloodFill(ey, ex);  
                }
            }
        }
        
        int capturedCount = 0;
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == -1) {
                    grid[i][j] = 0;
                } else if (grid[i][j] == 0) {
                    grid[i][j] = 1;
                    capturedCount++;
                } else if (grid[i][j] == 9) {
                    grid[i][j] = 1;
                    capturedCount++;
                }
            }
        }
        
        if (capturedCount > 0) {
            awardScore(playerIdx, capturedCount);
            fprintf(stderr, "Player %d captured %d tiles, score now: %d\n", 
                    playerIdx + 1, capturedCount, player.score);
        }
        
        return;  
    }
    
    
    bool onConstructedTile = (cellContent != 0 && cellContent != 1);
    
    player.x = newX;
    player.y = newY;
    
    
    if (cellContent == 0) {
        player.constructing = true;
        grid[player.y][player.x] = trailMarker;  
    } else {
        player.constructing = false;
        
        if (onConstructedTile) {
            player.dx = 0;
            player.dy = 0;
        }
    }
}



void MultiplayerGame::moveEnemies() {
    if (enemiesFrozen) return;
    
    for (int i = 0; i < enemyCount; i++) {
        enemies[i].move(grid, M, N, ts);
    }
}

void MultiplayerGame::handlePlayerCollisions() {
    if (player1.alive) {
        for (int i = 0; i < enemyCount; i++) {
            int ex = enemies[i].x / ts;
            int ey = enemies[i].y / ts;
            if (ex >= 0 && ex < N && ey >= 0 && ey < M) {
                if (grid[ey][ex] == 2) {
                    player1.alive = false;
                    deathMessage = "Player 1 Eliminated by Enemy!";
                    deathMessageTimer = 0;
                }
            }
        }
    }
    
    if (player2.alive) {
        for (int i = 0; i < enemyCount; i++) {
            int ex = enemies[i].x / ts;
            int ey = enemies[i].y / ts;
            if (ex >= 0 && ex < N && ey >= 0 && ey < M) {
                if (grid[ey][ex] == 3) {
                    player2.alive = false;
                    deathMessage = "Player 2 Eliminated by Enemy!";
                    deathMessageTimer = 0;
                }
            }
        }
    }
}

void MultiplayerGame::handleEnemyCollisions() {
}

void MultiplayerGame::awardScore(int playerIdx, int capturedTiles) {
    GamePlayer& player = (playerIdx == 0) ? player1 : player2;
    
    
    int pointsEarned = capturedTiles;  
    
    if (capturedTiles > player.bonusThreshold) {
        player.bonusCounter++;
        
        if (player.bonusCounter >= 5) {
            player.currentMultiplier = 4;  
            pointsEarned = capturedTiles * 4;
        } else {
            player.currentMultiplier = 2;  
            pointsEarned = capturedTiles * 2;
        }
        
        if (player.bonusCounter == 3) {
            player.bonusThreshold = 5;
        }
        
        fprintf(stderr, "Player %d BONUS! Bonus count: %d, Multiplier: %dx, Points earned: %d\n",
                playerIdx + 1, player.bonusCounter, player.currentMultiplier, pointsEarned);
    } else {
        player.currentMultiplier = 1; 
        pointsEarned = capturedTiles;
    }
    
    player.score += pointsEarned;
    
    updatePowerupRewards(playerIdx);
}

void MultiplayerGame::updatePowerupRewards(int playerIdx) {
    GamePlayer& player = (playerIdx == 0) ? player1 : player2;
    
    
    
    int powerUpThresholds[] = {50, 70, 100, 130, 160, 190, 220, 250, 280, 310, 340, 370, 400};
    
    int newPowerUps = 0;
    for (int threshold : powerUpThresholds) {
        if (player.score >= threshold) {
            newPowerUps++;
        } else {
            break; 
        }
    }
    
    if (newPowerUps > player.powerUps) {
        int newReward = newPowerUps - player.powerUps;
        player.powerUps = newPowerUps;
        fprintf(stderr, "Player %d earned %d power-up(s)! Total: %d\n", 
                playerIdx + 1, newReward, player.powerUps);
    }
}

bool MultiplayerGame::usePowerUp(int playerIdx) {
    GamePlayer& player = (playerIdx == 0) ? player1 : player2;
    
    if (!player.alive) {
        return false;
    }
    
    if (player.powerUps > 0) {
        player.powerUps--;
        freezeAll(playerIdx);
        return true;
    }
    return false;
}

void MultiplayerGame::freezeAll(int playerIdx) {
    if (playerIdx == 0) {
        p2Frozen = true;
        p2FreezeTime = 0;
        player2.dx = 0;
        player2.dy = 0;
    } else {
        p1Frozen = true;
        p1FreezeTime = 0;
        player1.dx = 0;
        player1.dy = 0;
    }
    
    // Freeze all enemies
    enemiesFrozen = true;
    enemiesFreezeTime = 0;
}

void MultiplayerGame::updateFreezeStates(float deltaTime) {
    if (p1Frozen) {
        p1FreezeTime += deltaTime;
        if (p1FreezeTime >= FREEZE_DURATION) {
            p1Frozen = false;
        }
    }
    
    if (p2Frozen) {
        p2FreezeTime += deltaTime;
        if (p2FreezeTime >= FREEZE_DURATION) {
            p2Frozen = false;
        }
    }
    
    if (enemiesFrozen) {
        enemiesFreezeTime += deltaTime;
        if (enemiesFreezeTime >= FREEZE_DURATION) {
            enemiesFrozen = false;
        }
    }
}

bool MultiplayerGame::isPlayerFrozen(int playerIdx) const {
    return (playerIdx == 0) ? p1Frozen : p2Frozen;
}

void MultiplayerGame::dropFloodFill(int y, int x) {
    if (y < 0 || y >= M || x < 0 || x >= N) return;
    if (grid[y][x] != 0) return; 
    
    grid[y][x] = -1;  
    
    dropFloodFill(y - 1, x);
    dropFloodFill(y + 1, x);
    dropFloodFill(y, x - 1);
    dropFloodFill(y, x + 1);
}

int MultiplayerGame::getWinner() const {
    if (player1.score > player2.score) return 1;
    if (player2.score > player1.score) return 2;
    return 0;  // Tie
}



GameState* MultiplayerGame::captureGameState(const char* saveID, const char* username) {
    GameState* state = new GameState();
    
    strcpy(state->saveID, saveID);
    strcpy(state->playerID, username);
    state->timestamp = time(nullptr);
    
    state->p1_x = player1.x;
    state->p1_y = player1.y;
    state->p1_score = player1.score;
    state->p1_powerUps = player1.powerUps;
    state->p1_alive = player1.alive ? 1 : 0;
    state->p1_constructing = player1.constructing ? 1 : 0;
    state->p1_bonusCounter = player1.bonusCounter;
    state->p1_bonusThreshold = player1.bonusThreshold;
    state->p1_currentMultiplier = player1.currentMultiplier;
    
    state->p2_x = player2.x;
    state->p2_y = player2.y;
    state->p2_score = player2.score;
    state->p2_powerUps = player2.powerUps;
    state->p2_alive = player2.alive ? 1 : 0;
    state->p2_constructing = player2.constructing ? 1 : 0;
    state->p2_bonusCounter = player2.bonusCounter;
    state->p2_bonusThreshold = player2.bonusThreshold;
    state->p2_currentMultiplier = player2.currentMultiplier;
    
    state->gameTimer = (int)gameTimer;
    state->gameEnded = gameEnded() ? 1 : 0;
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] != 0) {
                state->addTile(j, i, grid[i][j]);
            }
        }
    }
    
    fprintf(stderr, "[MultiplayerGame] Game state captured for save: %s\n", saveID);
    return state;
}

bool MultiplayerGame::restoreGameState(const GameState* state) {
    if (!state) {
        fprintf(stderr, "[MultiplayerGame] Error: Invalid game state\n");
        return false;
    }
    
    player1.x = state->p1_x;
    player1.y = state->p1_y;
    player1.score = state->p1_score;
    player1.powerUps = state->p1_powerUps;
    player1.alive = (state->p1_alive != 0);
    player1.constructing = (state->p1_constructing != 0);
    player1.bonusCounter = state->p1_bonusCounter;
    player1.bonusThreshold = state->p1_bonusThreshold;
    player1.currentMultiplier = state->p1_currentMultiplier;
    
    player2.x = state->p2_x;
    player2.y = state->p2_y;
    player2.score = state->p2_score;
    player2.powerUps = state->p2_powerUps;
    player2.alive = (state->p2_alive != 0);
    player2.constructing = (state->p2_constructing != 0);
    player2.bonusCounter = state->p2_bonusCounter;
    player2.bonusThreshold = state->p2_bonusThreshold;
    player2.currentMultiplier = state->p2_currentMultiplier;
    
    gameTimer = (float)state->gameTimer;
    deathMessageTimer = state->gameEnded ? DEATH_MESSAGE_DURATION : 0;
    
    initializeGrid();
    TileNode* tileNode = state->tileListHead;
    while (tileNode) {
        if (tileNode->x >= 0 && tileNode->x < N && tileNode->y >= 0 && tileNode->y < M) {
            grid[tileNode->y][tileNode->x] = tileNode->tileType;
        }
        tileNode = tileNode->next;
    }
    
    fprintf(stderr, "[MultiplayerGame] Game state restored\n");
    return true;
}

void MultiplayerGame::recordTileInSave(GameState* state, int x, int y, int type) {
    if (state) {
        state->addTile(x, y, type);
    }
}