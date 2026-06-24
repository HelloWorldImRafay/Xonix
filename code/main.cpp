
#include <SFML/Graphics.hpp>
#include <time.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include "LoginSystem.h"      
#include "PointSystem.h"     
#include "Leaderboard.h"     
#include "FriendSystem.h"    
#include "PlayerProfile.h"     
#include "MultiplayerGame.h"   
#include "GameRoom.h"          
#include "Inventory.h"         
#include "SaveGame.h"          
#include "Achievements.h"       

using namespace sf;
using namespace std;


Color parseHexColor(const string& hexCode) {
    string hex = hexCode;
    if (hex[0] == '#') {
        hex = hex.substr(1);
    }
    
    while (hex.length() < 6) {
        hex = "0" + hex;
    }
    
    try {
        unsigned int colorValue = stoul(hex, nullptr, 16);
        unsigned char r = (colorValue >> 16) & 0xFF;
        unsigned char g = (colorValue >> 8) & 0xFF;
        unsigned char b = colorValue & 0xFF;
        return Color(r, g, b);
    } catch (...) {
        return Color(30, 144, 255);  
    }
}

// -------------------------------
// GAME CONSTANTS
// -------------------------------
const int M = 25;
const int N = 40;
int grid[M][N] = {0};
int ts = 18;

// -------------------------------
// MENU STATES
// -------------------------------
const int STATE_MAIN_MENU         = 0;
const int STATE_START_MENU        = 1;
const int STATE_LEVEL_MENU        = 2;
const int STATE_LEADERBOARD       = 3;
const int STATE_PLAY              = 4;
const int STATE_GAMEOVER          = 5;
const int STATE_AUTH              = 6;  // Authentication (login/register) screen
const int STATE_FRIEND_SYSTEM     = 7;  // Friend System
const int STATE_PLAYER_PROFILE    = 8;  // Player Profile
const int STATE_MODE_SELECT       = 9;  // Single-player vs Multiplayer selection
const int STATE_PLAY_MULTIPLAYER  = 10; // Multiplayer game
const int STATE_MATCHMAKING       = 11; // Matchmaking/Game Room waiting
const int STATE_MATCHMADE         = 12; // Match created, ready to start
const int STATE_THEME_SELECTION   = 13; // Theme/Inventory selection
const int STATE_PAUSE_MENU        = 14; // Pause menu (during multiplayer)
const int STATE_SAVED_GAMES       = 15; // Saved games browser
const int STATE_ACHIEVEMENTS      = 16; // Achievements menu

int gameState = STATE_AUTH; // start at auth screen
int selectedLevel = 1;

// -----------------------------------------------
// ACHIEVEMENTS SYSTEM
// -----------------------------------------------
AchievementData playerAchievements;
bool achievementsLoaded = false;
float achievementsScrollOffset = 0.0f;

// -----------------------------------------------
// SAVED GAMES MENU SYSTEM
// -----------------------------------------------
SaveInfo playerSaves[100];  // List of player's saves
int playerSaveCount = 0;
int savedGamesMenuIndex = 0;
float savedGamesScrollOffset = 0.0f;
GameState* resumeGameState = nullptr;  // Game state selected to resume
bool savesListLoaded = false;  // Track if saves list has been loaded
bool savedGamesJustEntered = false;  // Skip first RETURN key press when entering state

// -----------------------------------------------
// PAUSE MENU SYSTEM
// -----------------------------------------------
bool gamePaused = false;
int pauseMenuSelection = 0;  // 0=Resume, 1=Save, 2=Quit (Load Game moved to main menu)
SaveLoadManager saveLoadManager;
GameState* currentGameSnapshot = nullptr;
char currentSaveID[64] = {0};

// Load Game in Pause Menu
bool pauseLoadGameMode = false;  // True when browsing saves in pause menu
int pauseLoadGameIndex = 0;       // Selected save in pause menu
SaveInfo pausePlayerSaves[100];   // List of saves shown in pause menu
int pausePauseCount = 0;          // Number of saves in pause menu
float pauseLoadGameScrollOffset = 0.0f;

// -------------------------------
// ENEMY STRUCT
// -------------------------------
struct Enemy {
    int x, y, dx, dy;
    Enemy() {
        x = y = 300;
        dx = 4 - rand() % 8;
        dy = 4 - rand() % 8;
    }
    void move() {
        x += dx;
        if (grid[y / ts][x / ts] == 1) { dx = -dx; x += dx; }
        y += dy;
        if (grid[y / ts][x / ts] == 1) { dy = -dy; y += dy; }
    }
};

// -------------------------------
// DROP LOGIC
// -------------------------------
void drop(int y, int x) {
    if (grid[y][x] == 0) grid[y][x] = -1;
    if (grid[y - 1][x] == 0) drop(y - 1, x);
    if (grid[y + 1][x] == 0) drop(y + 1, x);
    if (grid[y][x - 1] == 0) drop(y, x - 1);
    if (grid[y][x + 1] == 0) drop(y, x + 1);
}

// -------------------------------
// PARTICLE STRUCT
// -------------------------------
struct Particle {
    Vector2f pos;
    Vector2f vel;
    float radius;
    Color color;
};

// -------------------------------
// WINDOW RECREATION (FULLSCREEN <-> WINDOWED)
// -------------------------------
void recreateWindow(RenderWindow &window, bool fullscreenMode,
                    int ts, int N, int M,
                    float &scaleX, float &scaleY)
{
    window.close();
    VideoMode desktop = VideoMode::getDesktopMode();

    if (fullscreenMode) {
        window.create(desktop, "Xonix!", Style::Fullscreen);
        scaleX = (float)desktop.width / (N * ts);
        scaleY = (float)desktop.height / (M * ts);
    } else {
        window.create(VideoMode(N * ts, M * ts), "Xonix!", Style::Titlebar | Style::Close);
        scaleX = 1.f;
        scaleY = 1.f;
    }

    window.setFramerateLimit(60);
}

int main() {
    srand(time(0));

    // -------------------------------
    // SELECT MODE
    // -------------------------------
    bool fullscreenMode = false;
    float scaleX = 1.f, scaleY = 1.f;

    RenderWindow window;
    recreateWindow(window, fullscreenMode, ts, N, M, scaleX, scaleY);

    // -------------------------------
    // TEXTURES
    // -------------------------------
    Texture t1, t2, t3;
    t1.loadFromFile("images/tiles.png");
    t2.loadFromFile("images/gameover.png");
    t3.loadFromFile("images/enemy.png");
    Sprite sTile(t1), sEnemy(t3);
    sEnemy.setOrigin(20, 20);

    // -------------------------------
    // FONT
    // -------------------------------
    Font font;
    if (!font.loadFromFile("../images/SairaStencilOne-Regular.ttf")) {
        // fallback to a default font if needed (but SFML has no default)
    }

    // -------------------------------
    // BUTTONS (main menus)
    // -------------------------------
    Text btnStart("Start Game", font, 32); btnStart.setPosition(180,150);
    Text btnMatchmaking("Game Room", font, 32); btnMatchmaking.setPosition(180,210);
    Text btnMultiplayerQuick("Multiplayer (2P)", font, 32); btnMultiplayerQuick.setPosition(180,270);
    Text btnLoadGame("Load Game", font, 32); btnLoadGame.setPosition(180,330);
    Text btnLevel("Select Level", font, 32); btnLevel.setPosition(180,390);
    Text btnThemes("Select Theme", font, 32); btnThemes.setPosition(180,450);
    Text btnLeaderboard("Leaderboard", font, 32); btnLeaderboard.setPosition(180,510);
    Text btnFriends("Friends", font, 32); btnFriends.setPosition(180,570);
    Text btnProfile("Profile", font, 32); btnProfile.setPosition(180,630);
    Text btnAchievements("Achievements", font, 32); btnAchievements.setPosition(180,690);
    Text btnSignOut("Sign Out", font, 32); btnSignOut.setPosition(180,750);
    Text btnExit("Exit", font, 32); btnExit.setPosition(180,810);
    Text btnFullscreen("Toggle Fullscreen (F11)", font, 20); btnFullscreen.setPosition(10, 10);

    Text lvl1("Level 1", font, 32); lvl1.setPosition(200,140);
    Text lvl2("Level 2", font, 32); lvl2.setPosition(200,200);
    Text lvl3("Level 3", font, 32); lvl3.setPosition(200,260);
    Text lvlBack("Back", font, 32); lvlBack.setPosition(200,320);

    Text lbText("Leaderboard (Press ESC)", font, 28); lbText.setPosition(100,200);

    Text btnRestart("Restart", font, 32); btnRestart.setPosition(180,200);
    Text btnMainMenu("Main Menu", font, 32); btnMainMenu.setPosition(180,260);
    Text btnQuit("Exit Game", font, 32); btnQuit.setPosition(180,320);

    // Mode selection buttons (Single-player vs Multiplayer)
    Text btnSinglePlayer("Single Player", font, 32); btnSinglePlayer.setPosition(150, 150);
    Text btnMultiplayer("Multiplayer", font, 32); btnMultiplayer.setPosition(150, 250);
    Text btnModeBack("Back", font, 32); btnModeBack.setPosition(150, 350);

    // Multiplayer game over buttons
    Text btnMPRestart("Restart", font, 32); btnMPRestart.setPosition(180, 150);
    Text btnMPMainMenu("Main Menu", font, 32); btnMPMainMenu.setPosition(180, 210);
    Text btnMPQuit("Exit Game", font, 32); btnMPQuit.setPosition(180, 270);

    // Menu indexes
    int mainMenuIndex = 0;      // 0-12 (Start, Game Room, Multiplayer, Load Game, Level, Theme, Leaderboard, Friends, Profile, Achievements, Sign Out, Exit, Fullscreen)
    int modeSelectIndex = 0;  // 0-2 for mode selection
    int mpGameOverMenuIndex = 0;  // 0-2 for multiplayer game over

    // -------------------------------
    // AUTH (login/register) UI
    // -------------------------------
    enum AuthMode { AUTH_LOGIN = 0, AUTH_REGISTER = 1 };
    AuthMode authMode = AUTH_LOGIN;

    // Input strings
    string inputUsername = "";
    string inputPassword = "";
    string inputNickname = "";
    string inputEmail = "";

    // Which input box is focused
    enum Field { FIELD_NONE= -1, FIELD_USERNAME=0, FIELD_PASSWORD=1, FIELD_NICKNAME=2, FIELD_EMAIL=3 };
    Field focusedField = FIELD_USERNAME;

    // Auth UI text elements
    Text authTitle("Login", font, 40);
    authTitle.setPosition(200, 40);

    Text labelUsername("Username:", font, 20); labelUsername.setPosition(120, 120);
    Text labelPassword("Password:", font, 20); labelPassword.setPosition(120, 170);
    Text labelNickname("Nickname:", font, 20); labelNickname.setPosition(120, 220);
    Text labelEmail("Email:", font, 20); labelEmail.setPosition(120, 270);

    RectangleShape boxUsername(Vector2f(300, 30)); boxUsername.setPosition(240, 115);
    RectangleShape boxPassword(Vector2f(300, 30)); boxPassword.setPosition(240, 165);
    RectangleShape boxNickname(Vector2f(300, 30)); boxNickname.setPosition(240, 215);
    RectangleShape boxEmail(Vector2f(300, 30)); boxEmail.setPosition(240, 265);

    Text textUsername("", font, 18); textUsername.setPosition(245, 118);
    Text textPassword("", font, 18); textPassword.setPosition(245, 168);
    Text textNickname("", font, 18); textNickname.setPosition(245, 218);
    Text textEmail("", font, 18); textEmail.setPosition(245, 268);

    Text btnAuthSubmit("Submit", font, 24); btnAuthSubmit.setPosition(240, 320);
    Text btnAuthSwitch("Switch to Register", font, 18); btnAuthSwitch.setPosition(340, 326);

    Text authStatus("", font, 16); authStatus.setPosition(120, 370);

    // caret blink
    Clock caretClock;
    bool showCaret = true;
    
    // Theme color (updated when theme changes)
    Color currentThemeColor = Color(30, 144, 255);  // Default blue (#1E90FF)

    // -------------------------------
    // MENU SELECTION INDEXES
    // -------------------------------
    int levelMenuIndex = 0;     // 0-3 (Level1,2,3,Back)
    int gameOverMenuIndex = 0;  // 0-2 (Restart, Main Menu, Exit)
    float mainMenuScrollOffset = 0.0f;  // For scrolling on small screens

    // -------------------------------
    // GAME VARIABLES
    // -------------------------------
    int enemyCount = 4;
    Enemy enemies[10];
    bool Game = true;
    int x = 0, y = 0, dx = 0, dy = 0;
    float timer = 0, delay = 0.07;
    Clock clock;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1) grid[i][j] = 1;

    // -------------------------------
    // PARTICLES
    // -------------------------------
    Particle particles[50];
    for (int i = 0; i < 50; i++) {
        particles[i].pos = Vector2f(rand() % (N * ts), rand() % (M *ts));
        particles[i].vel = Vector2f((rand() % 100 - 50) / 100.f, (rand() % 100 - 50) / 100.f);
        particles[i].radius = rand() % 3 + 1;
        particles[i].color = Color(255,255,255,100);
    }
    float colorOffset = 0;

    // currently logged-in player
    Player currentPlayer;
    bool loggedIn = false;

    // Multiplayer game instance
    MultiplayerGame multiplayerGame;
    bool multiplayerActive = false;

    // -------------------------------
    // POINT SYSTEM (integrated)
    // -------------------------------
    PointSystem points; // uses internal score, rewardCounter, powerUps, thresholds

    // -------------------------------
    // LEADERBOARD & PLAYER PROFILE
    // -------------------------------
    Leaderboard leaderboard;
    PlayerProfile playerProfile(0, "", ""); // will be initialized after login

    // -------------------------------
    // GAME ROOM (MATCHMAKING WITH PRIORITY QUEUE)
    // -------------------------------
    GameRoom gameRoom;
    float matchmakingTimer = 0.0f;
    string playerWaitingForMatch = "";
    
    // Matchmaking UI
    Text mmTitle("MATCHMAKING", font, 40);
    mmTitle.setPosition(150, 100);
    mmTitle.setFillColor(Color::Yellow);
    
    Text mmStatus("Searching for opponent...", font, 20);
    mmStatus.setPosition(150, 200);
    mmStatus.setFillColor(Color::Cyan);
    
    Text mmQueueInfo("", font, 16);
    mmQueueInfo.setPosition(150, 260);
    mmQueueInfo.setFillColor(Color::White);
    
    Text mmCancel("Press ESC to cancel", font, 14);
    mmCancel.setPosition(150, 320);
    mmCancel.setFillColor(Color(200, 200, 200));
    
    Text mmCountdown("", font, 18);
    mmCountdown.setPosition(150, 370);
    mmCountdown.setFillColor(Color::Green);

    // -------------------------------
    // INVENTORY SYSTEM (AVL TREE)
    // -------------------------------
    Inventory playerInventory;
    playerInventory.initialize("player_inventory.txt");
    
    // Load current theme color from inventory
    currentThemeColor = parseHexColor(playerInventory.getCurrentThemeColorCode());
    
    // Inventory/Theme UI elements
    Text themeTitle("THEME SELECTION", font, 40);
    themeTitle.setPosition(100, 50);
    themeTitle.setFillColor(Color::Magenta);
    
    Text themeInfo("Use UP/DOWN to browse | ENTER to select | ESC to go back", font, 14);
    themeInfo.setPosition(50, 120);
    themeInfo.setFillColor(Color(200, 200, 200));

    // -------------------------------
    // FRIEND SYSTEM
    // -------------------------------
    FriendSystem friendSystem;
    
    // Friend System UI elements
    string friendSearchInput = "";
    enum FriendUIMode { FRIEND_VIEW = 0, FRIEND_SEARCH = 1 };
    FriendUIMode friendUIMode = FRIEND_VIEW;
    
    RectangleShape fsSearchBox(Vector2f(300, 30));
    fsSearchBox.setPosition(240, 60);
    
    RectangleShape fsSendButton(Vector2f(80, 30));
    fsSendButton.setPosition(550, 60);
    
    // For displaying accept/reject buttons next to pending requests
    struct PendingRequestUI {
        string fromUsername;
        int yPosition;
        RectangleShape acceptBtn;
        RectangleShape rejectBtn;
    };

    // Freeze (power-up) state
    bool enemiesFrozen = false;
    Clock freezeClock;
    const float freezeDuration = 3.0f; // 3 seconds as per rules

    // -------------------------------
    // HUD TEXTS (score & power-ups)
    // -------------------------------
    Text hudUsername("", font, 18);
    Text hudSeparator("-------------------------------------", font, 14);
    Text hudScore("", font, 20);
    Text hudPowerups("", font, 18);
    Text hudTotalScore("", font, 18);
    Text hudHighScore("", font, 18);
    Text hudHint("Press SPACE to use power-up", font, 14);
    hudHint.setFillColor(Color::White);

    // -------------------------------
    // MAIN LOOP
    // -------------------------------
    while(window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        Event e;
        while(window.pollEvent(e)) {
            if(e.type == Event::Closed) {
                if (loggedIn) {
                    cerr << "DEBUG: Saving on window close for user: " << currentPlayer.username << endl;
                    cerr << "DEBUG: Current allTimeScore: " << points.getAllTimeScore() << endl;
                    
                    // Write debug log to file so we can verify this code was reached
                    ofstream debugLog("debug.log", ios::app);
                    debugLog << "SAVE EVENT: user=" << currentPlayer.username << " score=" << points.getAllTimeScore() << endl;
                    debugLog.close();
                    
                    playerProfile.setTotalPoints(points.getAllTimeScore());
                    playerProfile.saveToFile();
                    points.saveToFile(currentPlayer.username);
                    leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                    leaderboard.saveToFile("leaderboard.txt");
                    friendSystem.saveToFile("friends_data.txt");
                    saveAchievements(playerAchievements);  // Save achievements on window close
                } else {
                    cerr << "DEBUG: Not logged in, skipping save on window close" << endl;
                }
                window.close();
            }

            // ----------------------------
            // FULLSCREEN TOGGLE (keyboard)
            // ----------------------------
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::F11) {
                fullscreenMode = !fullscreenMode;
                recreateWindow(window, fullscreenMode, ts, N, M, scaleX, scaleY);
            }

            // Global ESC handling
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                if (gameState == STATE_PLAY_MULTIPLAYER && !multiplayerGame.gameEnded()) {
                    // Toggle pause menu in multiplayer
                    gamePaused = !gamePaused;
                    pauseMenuSelection = 0;  // Reset to Resume
                    if (gamePaused) {
                        // Capture current game state
                        if (currentGameSnapshot) delete currentGameSnapshot;
                        currentGameSnapshot = multiplayerGame.captureGameState("auto_save", currentPlayer.username.c_str());
                    }
                } else if (gameState == STATE_PAUSE_MENU) {
                    // Close pause menu, resume game
                    gamePaused = false;
                    gameState = STATE_PLAY_MULTIPLAYER;
                } else if (gameState == STATE_LEADERBOARD || gameState == STATE_LEVEL_MENU || gameState == STATE_FRIEND_SYSTEM || gameState == STATE_PLAYER_PROFILE || gameState == STATE_MODE_SELECT || gameState == STATE_THEME_SELECTION) {
                    gameState = STATE_MAIN_MENU;
                } else if (gameState == STATE_MATCHMAKING) {
                    // Cancel matchmaking and return to main menu
                    gameRoom.removePlayerFromQueue(playerWaitingForMatch);
                    playerWaitingForMatch = "";
                    gameState = STATE_MAIN_MENU;
                } else if (gameState == STATE_AUTH) {
                    // allow ESC to clear focus
                    focusedField = FIELD_NONE;
                }
            }

            // ----------------------------
            // PAUSE MENU INPUT
            // ----------------------------
            if (gamePaused && gameState == STATE_PLAY_MULTIPLAYER && !pauseLoadGameMode) {
                if (e.type == Event::KeyPressed) {
                    if (e.key.code == Keyboard::Up) {
                        pauseMenuSelection = (pauseMenuSelection - 1 + 3) % 3;
                    } else if (e.key.code == Keyboard::Down) {
                        pauseMenuSelection = (pauseMenuSelection + 1) % 3;
                    } else if (e.key.code == Keyboard::Return) {
                        // Execute selected option
                        if (pauseMenuSelection == 0) {
                            // Resume
                            gamePaused = false;
                        } else if (pauseMenuSelection == 1) {
                            // Save game and return to main menu
                            char saveID[64];
                            snprintf(saveID, 64, "%s_pause_%ld", currentPlayer.username.c_str(), std::time(nullptr));
                            if (currentGameSnapshot) {
                                saveLoadManager.saveGame(*currentGameSnapshot, saveID);
                                strcpy(currentSaveID, saveID);
                                fprintf(stderr, "[Pause] Game saved: %s\n", saveID);
                            }
                            // Go to main menu
                            gamePaused = false;
                            gameState = STATE_MAIN_MENU;
                        } else if (pauseMenuSelection == 2) {
                            // Quit to main menu without saving
                            gamePaused = false;
                            gameState = STATE_MAIN_MENU;
                        }
                    }
                }
                // Mouse click support for pause menu
                if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                    // Get pause menu dimensions
                    float menuWidth = 300.f;
                    float menuHeight = 200.f;
                    float menuX = (float)window.getSize().x / 2 - menuWidth / 2;
                    float menuY = (float)window.getSize().y / 2 - menuHeight / 2;
                    float optionY = menuY + 60.f;
                    float optionSpacing = 40.f;
                    float optionWidth = 200.f;
                    float optionHeight = 30.f;
                    
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                    
                    // Check each option
                    for (int i = 0; i < 3; i++) {
                        float clickX = optionY + (i * optionSpacing);
                        if (mousePos.x >= menuX + 10.f && mousePos.x <= menuX + menuWidth - 10.f &&
                            mousePos.y >= clickX - 5.f && mousePos.y <= clickX + optionHeight - 5.f) {
                            // Clicked on option i
                            pauseMenuSelection = i;
                            if (i == 0) {
                                // Resume
                                gamePaused = false;
                            } else if (i == 1) {
                                // Save game and return to main menu
                                char saveID[64];
                                snprintf(saveID, 64, "%s_pause_%ld", currentPlayer.username.c_str(), std::time(nullptr));
                                if (currentGameSnapshot) {
                                    saveLoadManager.saveGame(*currentGameSnapshot, saveID);
                                    strcpy(currentSaveID, saveID);
                                    fprintf(stderr, "[Pause] Game saved: %s\n", saveID);
                                }
                                // Go to main menu
                                gamePaused = false;
                                gameState = STATE_MAIN_MENU;
                            } else if (i == 2) {
                                // Quit to main menu without saving
                                gamePaused = false;
                                gameState = STATE_MAIN_MENU;
                            }
                            break;
                        }
                    }
                }
            }

            // ----------------------------
            // (Load Game moved to main menu)
            // ----------------------------

            // ----------------------------
            // AUTH (SFML-based) INPUT
            // ----------------------------
            if (gameState == STATE_AUTH) {
                // mouse clicks to focus boxes or click buttons
                if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                    Vector2f mouseLogical((float)e.mouseButton.x / scaleX, (float)e.mouseButton.y / scaleY);
                    if (boxUsername.getGlobalBounds().contains(mouseLogical)) {
                        focusedField = FIELD_USERNAME;
                    } else if (boxPassword.getGlobalBounds().contains(mouseLogical)) {
                        focusedField = FIELD_PASSWORD;
                    } else if (boxNickname.getGlobalBounds().contains(mouseLogical) && authMode == AUTH_REGISTER) {
                        focusedField = FIELD_NICKNAME;
                    } else if (boxEmail.getGlobalBounds().contains(mouseLogical) && authMode == AUTH_REGISTER) {
                        focusedField = FIELD_EMAIL;
                    } else {
                        // check submit button
                        FloatRect submitRect(btnAuthSubmit.getPosition().x, btnAuthSubmit.getPosition().y,
                                             btnAuthSubmit.getLocalBounds().width, btnAuthSubmit.getLocalBounds().height);
                        FloatRect switchRect(btnAuthSwitch.getPosition().x, btnAuthSwitch.getPosition().y,
                                             btnAuthSwitch.getLocalBounds().width, btnAuthSwitch.getLocalBounds().height);
                        if (submitRect.contains(mouseLogical)) {
                            // perform submit action
                            if (authMode == AUTH_LOGIN) {
                                Player p;
                                if (loginPlayer(inputUsername, inputPassword, p)) {
                                    currentPlayer = p;
                                    loggedIn = true;
                                    authStatus.setString("Login successful! Welcome " + (p.nickname.empty() ? p.username : p.nickname));
                                    
                                    // Load PointSystem data for this user
                                    cerr << "DEBUG: Before load (mouse click) - allTime=" << points.getAllTimeScore() 
                                         << " powerUps=" << points.getPowerUps() << endl;
                                    points.loadFromFile(currentPlayer.username);
                                    cerr << "DEBUG: After load (mouse click) - allTime=" << points.getAllTimeScore() 
                                         << " powerUps=" << points.getPowerUps() << endl;
                                    points.resetScore(); // only reset session score, keep power-ups/all-time score
                                    cerr << "DEBUG: After resetScore (mouse click) - allTime=" << points.getAllTimeScore() 
                                         << " powerUps=" << points.getPowerUps() << endl;
                                    
                                    // Initialize PlayerProfile for this user
                                    playerProfile = PlayerProfile(0, currentPlayer.username, currentPlayer.nickname);
                                    playerProfile.loadFromFile();
                                    playerProfile.setTotalPoints(points.getAllTimeScore());
                                    
                                    // Load leaderboard
                                    leaderboard.loadFromFile("leaderboard.txt");
                                    
                                    // Load friend system
                                    friendSystem.loadAllPlayersFromFile("players.txt");
                                    friendSystem.loadFromFile("friends_data.txt");
                                    
                                    // CRITICAL FIX: Register current player in friend system if not already registered
                                    if (friendSystem.findPlayerIndex(currentPlayer.username.c_str()) == -1) {
                                        friendSystem.registerPlayer(currentPlayer.username.c_str(), 0);
                                    }
                                    
                                    // Load saved games list once at startup
                                    playerSaveCount = saveLoadManager.listSavesForPlayer(currentPlayer.username.c_str(), playerSaves, 100);
                                    savesListLoaded = true;

                                    // Initialize and load achievements for this user
                                    std::cerr << "\n===== ACHIEVEMENT LOAD SEQUENCE FOR " << currentPlayer.username << " =====" << std::endl;
                                    initAchievementData(playerAchievements, currentPlayer.username, false);
                                    loadAchievements(playerAchievements);               // Load achievement unlock status FIRST
                                    loadPlayerStatsForAchievements(playerAchievements); // Load stats AFTER achievements
                                    std::cerr << "Achievement state after full load:" << std::endl;
                                    for (int i = 0; i < 4; i++) {
                                        std::cerr << "  Achievement " << i << ": " << (playerAchievements.unlocked[i] ? "UNLOCKED" : "LOCKED") << std::endl;
                                    }
                                    std::cerr << "====================================" << std::endl << std::endl;
                                    achievementsLoaded = true;
                                    
                                    gameState = STATE_MAIN_MENU; // proceed to main menu
                                } else {
                                    authStatus.setString("Invalid username or password.");
                                }
                            } else {
                                // register flow
                                if (inputUsername.empty() || inputPassword.empty()) {
                                    authStatus.setString("Username and password required.");
                                } else if (usernameExists(inputUsername)) {
                                    authStatus.setString("Username already exists.");
                                } else if (!validatePassword(inputPassword)) {
                                    authStatus.setString("Password must be >=8 chars with upper, lower, digit, special.");
                                } else {
                                    if (registerPlayer(inputUsername, inputPassword, inputNickname, inputEmail)) {
                                        // auto-login after register
                                        Player p;
                                        if (loginPlayer(inputUsername, inputPassword, p)) {
                                            currentPlayer = p;
                                            loggedIn = true;
                                            authStatus.setString("Registered. Welcome " + (p.nickname.empty() ? p.username : p.nickname));
                                            
                                            // Load PointSystem data for this user (new player = empty)
                                            points.loadFromFile(currentPlayer.username);
                                            points.resetScore();
                                            
                                            // Initialize PlayerProfile for this user
                                            playerProfile = PlayerProfile(0, currentPlayer.username, currentPlayer.nickname);
                                            playerProfile.loadFromFile();
                                            playerProfile.setTotalPoints(points.getAllTimeScore());
                                            
                                            // Load leaderboard
                                            leaderboard.loadFromFile("leaderboard.txt");
                                            
                                            // Load friend system
                                            friendSystem.loadAllPlayersFromFile("players.txt");
                                            friendSystem.loadFromFile("friends_data.txt");
                                            
                                            // CRITICAL FIX: Register current player in friend system if not already registered
                                            if (friendSystem.findPlayerIndex(currentPlayer.username.c_str()) == -1) {
                                                friendSystem.registerPlayer(currentPlayer.username.c_str(), 0);
                                            }
                                            
                                            // Initialize and load achievements for this user
                                            initAchievementData(playerAchievements, currentPlayer.username, true);
                                            loadAchievements(playerAchievements);               // Load achievement unlock status
                                            loadPlayerStatsForAchievements(playerAchievements); // Load stats AFTER achievements
                                            achievementsLoaded = true;
                                            
                                            gameState = STATE_AUTH; // proceed
                                            authMode =  AUTH_LOGIN;

                                            inputUsername.clear();
                                            inputPassword.clear();
                                            inputNickname.clear();
                                            inputEmail.clear();
                                        } else {
                                            authStatus.setString("Registered but failed to login. Try logging in.");
                                        }
                                    } else {
                                        authStatus.setString("Registration failed (file error?).");
                                    }
                                }
                            }
                        } else if (switchRect.contains(mouseLogical)) {
                            // toggle mode
                            if (authMode == AUTH_LOGIN) {
                                authMode = AUTH_REGISTER;
                                authTitle.setString("Register");
                                btnAuthSwitch.setString("Switch to Login");
                            } else {
                                authMode = AUTH_LOGIN;
                                authTitle.setString("Login");
                                btnAuthSwitch.setString("Switch to Register");
                            }
                            // clear inputs & status
                            inputPassword = ""; inputUsername = ""; inputNickname = ""; inputEmail = "";
                            authStatus.setString("");
                        } else {
                            focusedField = FIELD_NONE;
                        }
                    }
                }

                // text input for typed characters
                if (e.type == Event::TextEntered) {
                    Uint32 unicode = e.text.unicode;
                    if (unicode >= 32 && unicode < 127) { // printable ASCII
                        char c = static_cast<char>(unicode);
                        if (focusedField == FIELD_USERNAME) {
                            inputUsername.push_back(c);
                        } else if (focusedField == FIELD_PASSWORD) {
                            inputPassword.push_back(c);
                        } else if (focusedField == FIELD_NICKNAME && authMode == AUTH_REGISTER) {
                            inputNickname.push_back(c);
                        } else if (focusedField == FIELD_EMAIL && authMode == AUTH_REGISTER) {
                            inputEmail.push_back(c);
                        }
                    } else if (unicode == 8) { // backspace (some platforms)
                        // handled in KeyPressed below for portability
                    }
                }

                // key presses for navigation and backspace
                if (e.type == Event::KeyPressed) {
                    if (e.key.code == Keyboard::Tab) {
                        // rotate focus
                        if (focusedField == FIELD_USERNAME) focusedField = FIELD_PASSWORD;
                        else if (focusedField == FIELD_PASSWORD) focusedField = (authMode == AUTH_REGISTER ? FIELD_NICKNAME : FIELD_USERNAME);
                        else if (focusedField == FIELD_NICKNAME) focusedField = FIELD_EMAIL;
                        else if (focusedField == FIELD_EMAIL) focusedField = FIELD_USERNAME;
                        else focusedField = FIELD_USERNAME;
                    }
                    else if (e.key.code == Keyboard::BackSpace) {
                        if (focusedField == FIELD_USERNAME && !inputUsername.empty()) inputUsername.pop_back();
                        else if (focusedField == FIELD_PASSWORD && !inputPassword.empty()) inputPassword.pop_back();
                        else if (focusedField == FIELD_NICKNAME && !inputNickname.empty()) inputNickname.pop_back();
                        else if (focusedField == FIELD_EMAIL && !inputEmail.empty()) inputEmail.pop_back();
                    }
                    else if (e.key.code == Keyboard::Enter) {
                        // same as clicking submit
                        if (authMode == AUTH_LOGIN) {
                            Player p;
                            if (loginPlayer(inputUsername, inputPassword, p)) {
                                currentPlayer = p;
                                loggedIn = true;
                                authStatus.setString("Login successful! Welcome " + (p.nickname.empty() ? p.username : p.nickname));
                                
                                // Load PointSystem data for this user
                                cerr << "DEBUG: Before load - allTime=" << points.getAllTimeScore() 
                                     << " powerUps=" << points.getPowerUps() << endl;
                                points.loadFromFile(currentPlayer.username);
                                cerr << "DEBUG: After load - allTime=" << points.getAllTimeScore() 
                                     << " powerUps=" << points.getPowerUps() << endl;
                                points.resetScore(); // only reset session score, keep power-ups/all-time score
                                cerr << "DEBUG: After resetScore - allTime=" << points.getAllTimeScore() 
                                     << " powerUps=" << points.getPowerUps() << endl;
                                
                                // Log what was loaded
                                ofstream debugLog("debug.log", ios::app);
                                debugLog << "LOGIN: user=" << currentPlayer.username << " loaded_score=" << points.getAllTimeScore() 
                                        << " powerUps=" << points.getPowerUps() << endl;
                                debugLog.close();
                                
                                // Initialize PlayerProfile for this user
                                playerProfile = PlayerProfile(0, currentPlayer.username, currentPlayer.nickname);
                                playerProfile.loadFromFile();
                                playerProfile.setTotalPoints(points.getAllTimeScore());
                                
                                // Load leaderboard
                                leaderboard.loadFromFile("leaderboard.txt");
                                
                                // Load friend system
                                friendSystem.loadAllPlayersFromFile("players.txt");
                                friendSystem.loadFromFile("friends_data.txt");

                                // CRITICAL FIX: Register current player in friend system if not already registered
                                if (friendSystem.findPlayerIndex(currentPlayer.username.c_str()) == -1) {
                                    friendSystem.registerPlayer(currentPlayer.username.c_str(), 0);
                                }

                                // Load saved games list once at startup
                                playerSaveCount = saveLoadManager.listSavesForPlayer(currentPlayer.username.c_str(), playerSaves, 100);
                                savesListLoaded = true;

                                // Initialize and load achievements for this user
                                std::cerr << "\n===== ACHIEVEMENT LOAD SEQUENCE FOR " << currentPlayer.username << " =====" << std::endl;
                                initAchievementData(playerAchievements, currentPlayer.username, false);
                                loadAchievements(playerAchievements);               // Load achievement unlock status FIRST
                                loadPlayerStatsForAchievements(playerAchievements); // Load stats AFTER achievements
                                std::cerr << "Achievement state after full load:" << std::endl;
                                for (int i = 0; i < 4; i++) {
                                    std::cerr << "  Achievement " << i << ": " << (playerAchievements.unlocked[i] ? "UNLOCKED" : "LOCKED") << std::endl;
                                }
                                std::cerr << "====================================" << std::endl << std::endl;
                                achievementsLoaded = true;

                                gameState = STATE_MAIN_MENU; // proceed to main menu
                            } else {
                                authStatus.setString("Invalid username or password.");
                            }
                        } else {
                            if (inputUsername.empty() || inputPassword.empty()) {
                                authStatus.setString("Username and password required.");
                            } else if (usernameExists(inputUsername)) {
                                authStatus.setString("Username already exists.");

                            } else if (!validatePassword(inputPassword)) {
                                authStatus.setString("Password must be >=8 chars with upper, lower, digit, special.");
                            } else {
                                if (registerPlayer(inputUsername, inputPassword, inputNickname, inputEmail)) {
                                    Player p;
                                    if (loginPlayer(inputUsername, inputPassword, p)) {
                                        currentPlayer = p;
                                        
                                        loggedIn = true;
                                        authStatus.setString("Registered & logged in. Welcome " + (p.nickname.empty() ? p.username : p.nickname));
                                        
                                        // Load PointSystem data for this user (new player = empty)
                                        points.loadFromFile(currentPlayer.username);
                                        points.resetScore();
                                        
                                        // Initialize PlayerProfile for this user
                                        playerProfile = PlayerProfile(0, currentPlayer.username, currentPlayer.nickname);
                                        playerProfile.loadFromFile();
                                        playerProfile.setTotalPoints(points.getAllTimeScore());
                                        
                                        // Load leaderboard
                                        leaderboard.loadFromFile("leaderboard.txt");
                                        
                                        // Load friend system
                                        friendSystem.loadAllPlayersFromFile("players.txt");
                                        friendSystem.loadFromFile("friends_data.txt");
                                        
                                        // CRITICAL FIX: Register current player in friend system if not already registered
                                        if (friendSystem.findPlayerIndex(currentPlayer.username.c_str()) == -1) {
                                            friendSystem.registerPlayer(currentPlayer.username.c_str(), 0);
                                        }
                                        
                                        // Initialize and load achievements for this user
                                        initAchievementData(playerAchievements, currentPlayer.username, true);
                                        loadAchievements(playerAchievements);               // Load achievement unlock status
                                        loadPlayerStatsForAchievements(playerAchievements); // Load stats AFTER achievements
                                        achievementsLoaded = true;
                                        
                                        gameState = STATE_MAIN_MENU; // proceed
                                    } else {
                                        authStatus.setString("Registered but failed to login. Try logging in.");
                                    }
                                } else {
                                    authStatus.setString("Registration failed (file error?).");
                                }
                            }
                        }
                    }
                }
            } 

            // ----------------------------
            // MOUSE CLICK EVENTS for non-auth states
            // ----------------------------
            if (gameState != STATE_AUTH) {
                if(e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left){
                    Vector2f mouseLogical((float)e.mouseButton.x / scaleX, (float)e.mouseButton.y / scaleY);

                    if(gameState == STATE_MAIN_MENU){
                        if(btnStart.getGlobalBounds().contains(mouseLogical)) {
                            gameState = STATE_MODE_SELECT;
                        }
                        if(btnMatchmaking.getGlobalBounds().contains(mouseLogical)) {
                            // Start matchmaking
                            gameRoom.resetQueue();
                            
                            // Get player's current leaderboard rank
                            int playerRank = INT_MAX;
                            int playerScore = points.getAllTimeScore();
                            leaderboard.loadFromFile("leaderboard.txt");
                            LeaderboardEntry topPlayers[10];
                            int playerCount = 0;
                            leaderboard.getTopPlayers(topPlayers, playerCount, 10);
                            
                            for (int i = 0; i < playerCount; i++) {
                                if (topPlayers[i].username == currentPlayer.username) {
                                    playerRank = i + 1;
                                    break;
                                }
                            }
                            
                            // Add player to queue
                            gameRoom.addPlayerToQueue(currentPlayer.username, playerRank, playerScore, 0.0f);
                            
                            // Add AI opponents for matchmaking demo - different players from leaderboard
                            if (playerCount > 0) {
                                // Find first opponent (different from current player)
                                int opp1Idx = 0;
                                if (topPlayers[0].username == currentPlayer.username && playerCount > 1) {
                                    opp1Idx = 1;
                                }
                                gameRoom.addPlayerToQueue(topPlayers[opp1Idx].username, opp1Idx + 1, topPlayers[opp1Idx].totalScore, 0.5f);
                                
                                // Find second opponent (different from first)
                                if (playerCount > 1) {
                                    int opp2Idx = 1;
                                    if (topPlayers[1].username == currentPlayer.username || topPlayers[1].username == topPlayers[opp1Idx].username) {
                                        opp2Idx = 2;
                                    }
                                    if (opp2Idx < playerCount) {
                                        gameRoom.addPlayerToQueue(topPlayers[opp2Idx].username, opp2Idx + 1, topPlayers[opp2Idx].totalScore, 1.0f);
                                    }
                                }
                            }
                            
                            playerWaitingForMatch = currentPlayer.username;
                            matchmakingTimer = 0.0f;
                            gameState = STATE_MATCHMAKING;
                        }
                        if(btnMultiplayerQuick.getGlobalBounds().contains(mouseLogical)) {
                            multiplayerGame.initializeGame();
                            multiplayerGame.setPlayer1PowerUps(points.getPowerUps());
                            multiplayerGame.setPlayer2PowerUps(points.getPowerUps());
                            multiplayerActive = true;
                            gameState = STATE_PLAY_MULTIPLAYER;
                        }
                        if(btnLoadGame.getGlobalBounds().contains(mouseLogical)) {
                            // Reload saved games list (may have new saves from this session)
                            playerSaveCount = saveLoadManager.listSavesForPlayer(currentPlayer.username.c_str(), playerSaves, 100);
                            savedGamesMenuIndex = 0;
                            savedGamesScrollOffset = 0.0f;
                            savedGamesJustEntered = false;
                            gameState = STATE_SAVED_GAMES;
                        }
                        if(btnLevel.getGlobalBounds().contains(mouseLogical)) gameState = STATE_LEVEL_MENU;
                        if(btnThemes.getGlobalBounds().contains(mouseLogical)) gameState = STATE_THEME_SELECTION;
                        if(btnLeaderboard.getGlobalBounds().contains(mouseLogical)) gameState = STATE_LEADERBOARD;
                        if(btnFriends.getGlobalBounds().contains(mouseLogical)) gameState = STATE_FRIEND_SYSTEM;
                        if(btnProfile.getGlobalBounds().contains(mouseLogical)) gameState = STATE_PLAYER_PROFILE;
                        if(btnExit.getGlobalBounds().contains(mouseLogical)) {
                            if (loggedIn) {
                                playerProfile.setTotalPoints(points.getAllTimeScore());
                                playerProfile.saveToFile();
                                points.saveToFile(currentPlayer.username);
                                leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                                leaderboard.saveToFile("leaderboard.txt");
                                friendSystem.saveToFile("friends_data.txt");
                                saveAchievements(playerAchievements);
                            }
                            window.close();
                        }
                        if(btnFullscreen.getGlobalBounds().contains(mouseLogical)){
                            fullscreenMode = !fullscreenMode;
                            recreateWindow(window, fullscreenMode, ts, N, M, scaleX, scaleY);
                        }
                    }
                    if(gameState == STATE_LEVEL_MENU){
                        if(lvl1.getGlobalBounds().contains(mouseLogical)) selectedLevel = 1;
                        if(lvl2.getGlobalBounds().contains(mouseLogical)) selectedLevel = 2;
                        if(lvl3.getGlobalBounds().contains(mouseLogical)) selectedLevel = 3;
                        if(lvlBack.getGlobalBounds().contains(mouseLogical)) gameState = STATE_MAIN_MENU;
                    }
                    if(gameState == STATE_GAMEOVER){
                        if(btnRestart.getGlobalBounds().contains(mouseLogical)){
                            Game = true; x = y = dx = dy = 0;
                            for(int i=0;i<M;i++) for(int j=0;j<N;j++)
                                grid[i][j] = (i==0||j==0||i==M-1||j==N-1)?1:0;
                            for(int i=0;i<enemyCount;i++) enemies[i] = Enemy();
                            points.resetScore();
                            enemiesFrozen = false;
                            gameState = STATE_PLAY;
                        }
                        if(btnMainMenu.getGlobalBounds().contains(mouseLogical)) {
                            playerProfile.setTotalPoints(points.getAllTimeScore());
                            playerProfile.saveToFile();
                            points.saveToFile(currentPlayer.username);
                            leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                            leaderboard.saveToFile("leaderboard.txt");
                            friendSystem.saveToFile("friends_data.txt");
                            Game = true;
                            gameState = STATE_MAIN_MENU;
                        }
                        if(btnQuit.getGlobalBounds().contains(mouseLogical)) {
                            playerProfile.setTotalPoints(points.getAllTimeScore());
                            playerProfile.saveToFile();
                            points.saveToFile(currentPlayer.username);
                            leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                            leaderboard.saveToFile("leaderboard.txt");
                            friendSystem.saveToFile("friends_data.txt");
                            saveAchievements(playerAchievements);
                            window.close();
                        }
                    }
                    if(gameState == STATE_MODE_SELECT){
                        if(btnSinglePlayer.getGlobalBounds().contains(mouseLogical)) {
                            points.resetScore();
                            gameState = STATE_PLAY;
                        }
                        if(btnMultiplayer.getGlobalBounds().contains(mouseLogical)) {
                            multiplayerGame.initializeGame();
                            multiplayerGame.setPlayer1PowerUps(points.getPowerUps());
                            multiplayerGame.setPlayer2PowerUps(points.getPowerUps());
                            multiplayerActive = true;
                            gameState = STATE_PLAY_MULTIPLAYER;
                        }
                        if(btnModeBack.getGlobalBounds().contains(mouseLogical)) {
                            gameState = STATE_MAIN_MENU;
                        }
                    }
                    if(gameState == STATE_PLAY_MULTIPLAYER && !multiplayerGame.gameEnded()){
                        // Multiplayer gameplay events handled in gameplay section
                    } else if(gameState == STATE_PLAY_MULTIPLAYER && multiplayerGame.gameEnded()){
                        if(btnMPRestart.getGlobalBounds().contains(mouseLogical)){
                            multiplayerGame.initializeGame();
                            multiplayerGame.setPlayer1PowerUps(points.getPowerUps());
                            multiplayerGame.setPlayer2PowerUps(points.getPowerUps());
                            gameState = STATE_PLAY_MULTIPLAYER;
                        }
                        if(btnMPMainMenu.getGlobalBounds().contains(mouseLogical)) {
                            points.saveToFile(currentPlayer.username);
                            leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                            leaderboard.saveToFile("leaderboard.txt");
                            gameState = STATE_MAIN_MENU;
                        }
                        if(btnMPQuit.getGlobalBounds().contains(mouseLogical)) {
                            points.saveToFile(currentPlayer.username);
                            leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                            leaderboard.saveToFile("leaderboard.txt");
                            saveAchievements(playerAchievements);
                            window.close();
                        }
                    }
                }
            }

            // ----------------------------
            // FRIEND SYSTEM EVENTS (mouse & keyboard)
            // ----------------------------
            if (gameState == STATE_FRIEND_SYSTEM) {
                // Mouse events
                if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                    Vector2f mouseLogical((float)e.mouseButton.x / scaleX, (float)e.mouseButton.y / scaleY);
                    
                    // Click on search box
                    if (fsSearchBox.getGlobalBounds().contains(mouseLogical)) {
                        friendUIMode = FRIEND_SEARCH;
                    }
                    
                    // Click send button
                    if (fsSendButton.getGlobalBounds().contains(mouseLogical) && !friendSearchInput.empty() && loggedIn) {
                        if (friendSystem.sendFriendRequest(currentPlayer.username.c_str(), friendSearchInput.c_str())) {
                            friendSystem.saveToFile("friends_data.txt");
                            friendSearchInput = "✓ Request sent!";
                        } else {
                            friendSearchInput = "✗ Failed to send";
                        }
                    }
                    
                    // Handle accept/reject button clicks
                    char requestsList[50][50];
                    int requestCount = 0;
                    if (loggedIn) {
                        friendSystem.getPendingRequests(currentPlayer.username.c_str(), requestsList, requestCount);
                    }
                    
                    int reqsYStart = 140;
                    for (int i = 0; i < requestCount && i < 15; i++) {
                        // Accept button bounds: 530, y, 60, 25
                        FloatRect acceptBounds(530, reqsYStart + i * 50 + 18, 60, 25);
                        // Reject button bounds: 700, y, 60, 25
                        FloatRect rejectBounds(700, reqsYStart + i * 50 + 18, 60, 25);
                        
                        if (acceptBounds.contains(mouseLogical)) {
                            friendSystem.acceptFriendRequest(requestsList[i], currentPlayer.username.c_str());
                        }
                        if (rejectBounds.contains(mouseLogical)) {
                            friendSystem.rejectFriendRequest(requestsList[i], currentPlayer.username.c_str());
                        }
                    }
                }
                
                // Text input for search
                if (e.type == Event::TextEntered && friendUIMode == FRIEND_SEARCH) {
                    Uint32 unicode = e.text.unicode;
                    if (unicode >= 32 && unicode < 127) { // printable ASCII
                        char c = static_cast<char>(unicode);
                        if (friendSearchInput.length() < 20 && friendSearchInput.find("✓") == string::npos && friendSearchInput.find("✗") == string::npos) {
                            friendSearchInput.push_back(c);
                        }
                    } else if (unicode == 8 && !friendSearchInput.empty()) { // backspace
                        if (friendSearchInput.find("✓") == string::npos && friendSearchInput.find("✗") == string::npos) {
                            friendSearchInput.pop_back();
                        } else {
                            friendSearchInput.clear();
                        }
                    }
                }
                
                // Keyboard for search mode exit
                if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape && friendUIMode == FRIEND_SEARCH) {
                    friendUIMode = FRIEND_VIEW;
                    friendSearchInput.clear();
                }
            }

            // ----------------------------
            // KEYBOARD MENU NAVIGATION (non-auth)
            // ----------------------------
            if(e.type == Event::KeyPressed && gameState != STATE_AUTH){
                // ---------- MAIN MENU ----------
                if(gameState == STATE_MAIN_MENU){
                    if(e.key.code == Keyboard::Up) mainMenuIndex = (mainMenuIndex + 13 - 1) % 13;
                    if(e.key.code == Keyboard::Down) mainMenuIndex = (mainMenuIndex + 1) % 13;
                    if(e.key.code == Keyboard::Enter){
                        switch(mainMenuIndex){
                            case 0: gameState = STATE_MODE_SELECT; break;
                            case 1: 
                                {
                                    // Start matchmaking
                                    gameRoom.resetQueue();
                                    
                                    int playerRank = INT_MAX;
                                    int playerScore = points.getAllTimeScore();
                                    leaderboard.loadFromFile("leaderboard.txt");
                                    LeaderboardEntry topPlayers[10];
                                    int playerCount = 0;
                                    leaderboard.getTopPlayers(topPlayers, playerCount, 10);
                                    
                                    for (int i = 0; i < playerCount; i++) {
                                        if (topPlayers[i].username == currentPlayer.username) {
                                            playerRank = i + 1;
                                            break;
                                        }
                                    }
                                    
                                    // Add current player to queue
                                    gameRoom.addPlayerToQueue(currentPlayer.username, playerRank, playerScore, 0.0f);
                                    
                                    // Add AI opponents for matchmaking demo - different players from leaderboard
                                    if (playerCount > 0) {
                                        // Find first opponent (different from current player)
                                        int opp1Idx = 0;
                                        if (topPlayers[0].username == currentPlayer.username && playerCount > 1) {
                                            opp1Idx = 1;
                                        }
                                        gameRoom.addPlayerToQueue(topPlayers[opp1Idx].username, opp1Idx + 1, topPlayers[opp1Idx].totalScore, 0.5f);
                                        
                                        // Find second opponent (different from first)
                                        if (playerCount > 1) {
                                            int opp2Idx = 1;
                                            if (topPlayers[1].username == currentPlayer.username || topPlayers[1].username == topPlayers[opp1Idx].username) {
                                                opp2Idx = 2;
                                            }
                                            if (opp2Idx < playerCount) {
                                                gameRoom.addPlayerToQueue(topPlayers[opp2Idx].username, opp2Idx + 1, topPlayers[opp2Idx].totalScore, 1.0f);
                                            }
                                        }
                                    }
                                    
                                    playerWaitingForMatch = currentPlayer.username;
                                    matchmakingTimer = 0.0f;
                                    gameState = STATE_MATCHMAKING;
                                }
                                break;
                            case 2: 
                                multiplayerGame.initializeGame();
                                multiplayerGame.setPlayer1PowerUps(points.getPowerUps());
                                multiplayerGame.setPlayer2PowerUps(points.getPowerUps());
                                multiplayerActive = true;
                                gameState = STATE_PLAY_MULTIPLAYER; 
                                break;
                            case 3:
                                // Load saved games (reload list to catch new saves from this session)
                                playerSaveCount = saveLoadManager.listSavesForPlayer(currentPlayer.username.c_str(), playerSaves, 100);
                                savedGamesMenuIndex = 0;
                                savedGamesScrollOffset = 0.0f;
                                savedGamesJustEntered = true;
                                gameState = STATE_SAVED_GAMES;
                                break;
                            case 4: gameState = STATE_LEVEL_MENU; break;
                            case 5: gameState = STATE_THEME_SELECTION; break;
                            case 6: gameState = STATE_LEADERBOARD; break;
                            case 7: gameState = STATE_FRIEND_SYSTEM; break;
                            case 8: gameState = STATE_PLAYER_PROFILE; break;
                            case 9: 
                                // View Achievements
                                gameState = STATE_ACHIEVEMENTS;
                                achievementsScrollOffset = 0.0f;  // Reset scroll on entry
                                // Also refresh player stats from file (only if logged in with username set)
                                if (loggedIn && !currentPlayer.username.empty()) {
                                    loadPlayerStatsForAchievements(playerAchievements);
                                }
                                break;
                            case 10: 
                                // Sign Out
                                if (loggedIn) {
                                    playerProfile.setTotalPoints(points.getAllTimeScore());
                                    playerProfile.saveToFile();
                                    points.saveToFile(currentPlayer.username);
                                    leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                                    leaderboard.saveToFile("leaderboard.txt");
                                    friendSystem.saveToFile("friends_data.txt");
                                    saveAchievements(playerAchievements);
                                }
                                loggedIn = false;
                                currentPlayer.username = "";
                                currentPlayer.nickname = "";
                                gameState = STATE_AUTH;
                                break;
                            case 11: 
                                if (loggedIn) {
                                    playerProfile.setTotalPoints(points.getAllTimeScore());
                                    playerProfile.saveToFile();
                                    points.saveToFile(currentPlayer.username);
                                    leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                                    leaderboard.saveToFile("leaderboard.txt");
                                    friendSystem.saveToFile("friends_data.txt");
                                    saveAchievements(playerAchievements);
                                }
                                window.close(); 
                                break;
                            case 12:
                                fullscreenMode = !fullscreenMode;
                                recreateWindow(window, fullscreenMode, ts, N, M, scaleX, scaleY);
                                break;
                        }
                    }
                }
                // ---------- LEVEL MENU ----------
                if(gameState == STATE_LEVEL_MENU){
                    if(e.key.code == Keyboard::Up) levelMenuIndex = (levelMenuIndex + 4 - 1) % 4;
                    if(e.key.code == Keyboard::Down) levelMenuIndex = (levelMenuIndex + 1) % 4;
                    if(e.key.code == Keyboard::Enter){
                        switch(levelMenuIndex){
                            case 0: selectedLevel = 1; break;
                            case 1: selectedLevel = 2; break;
                            case 2: selectedLevel = 3; break;
                            case 3: gameState = STATE_MAIN_MENU; break;
                        }
                    }
                }
                // ---------- MODE SELECT MENU ----------
                if(gameState == STATE_MODE_SELECT){
                    if(e.key.code == Keyboard::Up) modeSelectIndex = (modeSelectIndex + 3 - 1) % 3;
                    if(e.key.code == Keyboard::Down) modeSelectIndex = (modeSelectIndex + 1) % 3;
                    if(e.key.code == Keyboard::Enter){
                        switch(modeSelectIndex){
                            case 0: 
                                points.resetScore();
                                gameState = STATE_PLAY; 
                                break;
                            case 1: 
                                multiplayerGame.initializeGame();
                                multiplayerGame.setPlayer1PowerUps(points.getPowerUps());
                                multiplayerGame.setPlayer2PowerUps(points.getPowerUps());
                                multiplayerActive = true;
                                gameState = STATE_PLAY_MULTIPLAYER; 
                                break;
                            case 2: 
                                gameState = STATE_MAIN_MENU; 
                                break;
                        }
                    }
                }
                // ---------- SAVED GAMES MENU ----------
                if(gameState == STATE_SAVED_GAMES){
                    if(e.key.code == Keyboard::Escape){
                        gameState = STATE_MAIN_MENU;
                    }
                    else if(e.key.code == Keyboard::Up){
                        savedGamesMenuIndex = (savedGamesMenuIndex + playerSaveCount - 1) % playerSaveCount;
                    }
                    else if(e.key.code == Keyboard::Down){
                        savedGamesMenuIndex = (savedGamesMenuIndex + 1) % playerSaveCount;
                    }
                    else if(e.key.code == Keyboard::Return){
                        // Load selected save (skip if just entered this state with RETURN key)
                        if (playerSaveCount > 0 && savedGamesJustEntered == false) {
                            SaveInfo& selectedSave = playerSaves[savedGamesMenuIndex];
                            resumeGameState = saveLoadManager.loadGame(selectedSave.saveID);
                            if (resumeGameState) {
                                // Restore game state and continue
                                multiplayerGame.restoreGameState(resumeGameState);
                                multiplayerActive = true;
                                gamePaused = false;
                                gameState = STATE_PLAY_MULTIPLAYER;
                                fprintf(stderr, "[SaveGame] Loaded game: %s\n", selectedSave.saveID);
                            } else {
                                fprintf(stderr, "[SaveGame] Failed to load: %s\n", selectedSave.saveID);
                            }
                        } else if (savedGamesJustEntered == true) {
                            // Skip this RETURN key press, it was used to navigate to this menu
                            savedGamesJustEntered = false;
                        }
                    }
                    else if(e.key.code == Keyboard::Delete){
                        // Delete selected save
                        if (playerSaveCount > 0) {
                            SaveInfo& selectedSave = playerSaves[savedGamesMenuIndex];
                            saveLoadManager.deleteSave(selectedSave.saveID);
                            
                            // Reload the saves list
                            playerSaveCount = saveLoadManager.listSavesForPlayer(currentPlayer.username.c_str(), playerSaves, 100);
                            if (savedGamesMenuIndex >= playerSaveCount) {
                                savedGamesMenuIndex = max(0, playerSaveCount - 1);
                            }
                        }
                    }
                }
                // ---------- GAME OVER MENU ----------
                if(gameState == STATE_GAMEOVER){
                    if(e.key.code == Keyboard::Up) gameOverMenuIndex = (gameOverMenuIndex + 3 - 1) % 3;
                    if(e.key.code == Keyboard::Down) gameOverMenuIndex = (gameOverMenuIndex + 1) % 3;
                    if(e.key.code == Keyboard::Enter){
                        switch(gameOverMenuIndex){
                            case 0:
                                Game = true; x = y = dx = dy = 0;
                                for(int i=0;i<M;i++) for(int j=0;j<N;j++)
                                    grid[i][j] = (i==0||j==0||i==M-1||j==N-1)?1:0;
                                for(int i=0;i<enemyCount;i++) enemies[i] = Enemy();
                                points.resetScore();
                                enemiesFrozen = false;
                                gameState = STATE_PLAY;
                                break;
                            case 1: 
                                playerProfile.setTotalPoints(points.getAllTimeScore());
                                playerProfile.saveToFile();
                                points.saveToFile(currentPlayer.username);
                                leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                                leaderboard.saveToFile("leaderboard.txt");
                                friendSystem.saveToFile("friends_data.txt");
                                gameState = STATE_MAIN_MENU; 
                                break;
                            case 2: 
                                playerProfile.setTotalPoints(points.getAllTimeScore());
                                playerProfile.saveToFile();
                                points.saveToFile(currentPlayer.username);
                                leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                                leaderboard.saveToFile("leaderboard.txt");
                                friendSystem.saveToFile("friends_data.txt");
                                window.close(); 
                                break;
                        }
                    }
                }
                // ---------- MULTIPLAYER GAME OVER MENU ----------
                if(gameState == STATE_PLAY_MULTIPLAYER && multiplayerGame.gameEnded() && multiplayerGame.getDeathMessageTimer() >= multiplayerGame.DEATH_MESSAGE_DURATION){
                    if(e.key.code == Keyboard::Up) mpGameOverMenuIndex = (mpGameOverMenuIndex + 3 - 1) % 3;
                    if(e.key.code == Keyboard::Down) mpGameOverMenuIndex = (mpGameOverMenuIndex + 1) % 3;
                    if(e.key.code == Keyboard::Enter){
                        switch(mpGameOverMenuIndex){
                            case 0:
                                // Restart
                                multiplayerGame.initializeGame();
                                multiplayerGame.setPlayer1PowerUps(points.getPowerUps());
                                multiplayerGame.setPlayer2PowerUps(points.getPowerUps());
                                gameState = STATE_PLAY_MULTIPLAYER;
                                break;
                            case 1:
                                // Main Menu - delete any saved game from this session
                                if (strlen(currentSaveID) > 0) {
                                    saveLoadManager.deleteSave(currentSaveID);
                                    memset(currentSaveID, 0, 64);
                                }
                                // Save achievements before returning to menu
                                saveAchievements(playerAchievements);
                                points.saveToFile(currentPlayer.username);
                                leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                                leaderboard.saveToFile("leaderboard.txt");
                                gameState = STATE_MAIN_MENU;
                                break;
                            case 2:
                                // Exit - delete any saved game from this session
                                if (strlen(currentSaveID) > 0) {
                                    saveLoadManager.deleteSave(currentSaveID);
                                    memset(currentSaveID, 0, 64);
                                }
                                // Update achievements before quitting
                                updateAndCheckAchievements(playerAchievements, points.getScore(), 0, 0);
                                points.saveToFile(currentPlayer.username);
                                leaderboard.updatePlayerScore(currentPlayer.username, points.getAllTimeScore());
                                leaderboard.saveToFile("leaderboard.txt");
                                window.close();
                                break;
                        }
                    }
                }

                // ---------- GAMEPLAY: Use Power-up (SPACE) ----------
                if (gameState == STATE_PLAY && e.key.code == Keyboard::Space) {
                    // Try to use a power-up from inventory
                    if (points.usePowerUp()) {
                        enemiesFrozen = true;
                        freezeClock.restart();
                    } else {
                        // optionally provide feedback (e.g., play a sound or flash HUD)
                    }
                }
                
                // ---------- MULTIPLAYER GAMEPLAY CONTROLS ----------
                if (gameState == STATE_PLAY_MULTIPLAYER) {
                    // Player 1 controls with arrow keys
                    if (e.key.code == Keyboard::Left) multiplayerGame.setPlayer1Controls(-1, 0);
                    if (e.key.code == Keyboard::Right) multiplayerGame.setPlayer1Controls(1, 0);
                    if (e.key.code == Keyboard::Up) multiplayerGame.setPlayer1Controls(0, -1);
                    if (e.key.code == Keyboard::Down) multiplayerGame.setPlayer1Controls(0, 1);
                    
                    // Player 2 controls with WASD
                    if (e.key.code == Keyboard::A) multiplayerGame.setPlayer2Controls(-1, 0);  // Left
                    if (e.key.code == Keyboard::D) multiplayerGame.setPlayer2Controls(1, 0);   // Right
                    if (e.key.code == Keyboard::W) multiplayerGame.setPlayer2Controls(0, -1);  // Up
                    if (e.key.code == Keyboard::S) multiplayerGame.setPlayer2Controls(0, 1);   // Down
                    
                    // Power-ups: E for Player1, P for Player2
                    if (e.key.code == Keyboard::E) {
                        multiplayerGame.usePowerUp(0);
                    }
                    if (e.key.code == Keyboard::P) {
                        multiplayerGame.usePowerUp(1);
                    }
                }
            }
        } // end event loop

        // caret blinking
        if (caretClock.getElapsedTime().asSeconds() > 0.5f) {
            showCaret = !showCaret;
            caretClock.restart();
        }

        // handle unfreezing after freezeDuration
        if (enemiesFrozen) {
            if (freezeClock.getElapsedTime().asSeconds() >= freezeDuration) {
                enemiesFrozen = false;
            }
        }

        window.clear();

        Vector2i mouseWinPos = Mouse::getPosition(window);
        Vector2f mouseLogical((float)mouseWinPos.x / scaleX, (float)mouseWinPos.y / scaleY);

        // ==========================
        // AUTH SCREEN
        // ==========================
        if (gameState == STATE_AUTH) {
            // background
            colorOffset += 0.5f;
            int r = (int)(30 + 30 * sin(colorOffset * 0.02));
            int g = (int)(30 + 30 * sin(colorOffset * 0.03));
            int b = (int)(80 + 30 * sin(colorOffset * 0.01));
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(r,g,b));
            window.draw(bg);

            // title
            authTitle.setOrigin(0,0);
            window.draw(authTitle);

            // draw labels and boxes
            window.draw(labelUsername);
            window.draw(labelPassword);
            if (authMode == AUTH_REGISTER) {
                window.draw(labelNickname);
                window.draw(labelEmail);
            }

            // highlight focused box
            boxUsername.setFillColor(focusedField == FIELD_USERNAME ? Color(220,220,180) : Color(200,200,200));
            boxPassword.setFillColor(focusedField == FIELD_PASSWORD ? Color(220,220,180) : Color(200,200,200));
            boxNickname.setFillColor(focusedField == FIELD_NICKNAME ? Color(220,220,180) : Color(200,200,200));
            boxEmail.setFillColor(focusedField == FIELD_EMAIL ? Color(220,220,180) : Color(200,200,200));

            window.draw(boxUsername);
            window.draw(boxPassword);
            if (authMode == AUTH_REGISTER) {
                window.draw(boxNickname);
                window.draw(boxEmail);
            }

            // prepare displayed text (mask password)
            textUsername.setString(inputUsername);
            string masked;
            for (size_t i = 0; i < inputPassword.size(); ++i) masked.push_back('*');
            textPassword.setString(masked);
            textNickname.setString(inputNickname);
            textEmail.setString(inputEmail);

            window.draw(textUsername);
            window.draw(textPassword);
            if (authMode == AUTH_REGISTER) {
                window.draw(textNickname);
                window.draw(textEmail);
            }

            // draw caret in focused field
            if (showCaret) {
                if (focusedField == FIELD_USERNAME) {
                    float cx = textUsername.getPosition().x + textUsername.getLocalBounds().width + 2;
                    RectangleShape caret(Vector2f(2, textUsername.getCharacterSize()+2));
                    caret.setPosition(cx, textUsername.getPosition().y - 2);
                    caret.setFillColor(Color::Black);
                    window.draw(caret);
                } else if (focusedField == FIELD_PASSWORD) {
                    float cx = textPassword.getPosition().x + textPassword.getLocalBounds().width + 2;
                    RectangleShape caret(Vector2f(2, textPassword.getCharacterSize()+2));
                    caret.setPosition(cx, textPassword.getPosition().y - 2);
                    caret.setFillColor(Color::Black);
                    window.draw(caret);
                } else if (focusedField == FIELD_NICKNAME && authMode == AUTH_REGISTER) {
                    float cx = textNickname.getPosition().x + textNickname.getLocalBounds().width + 2;
                    RectangleShape caret(Vector2f(2, textNickname.getCharacterSize()+2));
                    caret.setPosition(cx, textNickname.getPosition().y - 2);
                    caret.setFillColor(Color::Black);
                    window.draw(caret);
                } else if (focusedField == FIELD_EMAIL && authMode == AUTH_REGISTER) {
                    float cx = textEmail.getPosition().x + textEmail.getLocalBounds().width + 2;
                    RectangleShape caret(Vector2f(2, textEmail.getCharacterSize()+2));
                    caret.setPosition(cx, textEmail.getPosition().y - 2);
                    caret.setFillColor(Color::Black);
                    window.draw(caret);
                }
            }

            // buttons
            window.draw(btnAuthSubmit);
            window.draw(btnAuthSwitch);

            // status
            window.draw(authStatus);

            window.display();
            continue;
        }

        // ==========================
        // MAIN MENU
        // ==========================
        if(gameState == STATE_MAIN_MENU){
            colorOffset += 0.5f;
            int r = (int)(50 + 50 * sin(colorOffset * 0.02));
            int g = (int)(50 + 50 * sin(colorOffset * 0.03));
            int b = (int)(100 + 50 * sin(colorOffset * 0.01));
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(r,g,b));
            window.draw(bg);

            for(int i=0;i<50;i++){
                CircleShape c(particles[i].radius);
                c.setPosition(Vector2f(particles[i].pos.x * scaleX, particles[i].pos.y * scaleY));
                c.setFillColor(particles[i].color);
                window.draw(c);
                particles[i].pos += particles[i].vel;
                if(particles[i].pos.x < 0) particles[i].pos.x = N*ts;
                if(particles[i].pos.x > N*ts) particles[i].pos.x = 0;
                if(particles[i].pos.y < 0) particles[i].pos.y = M*ts;
                if(particles[i].pos.y > M*ts) particles[i].pos.y = 0;
            }

            Text title("XONIX", font, 64);
            title.setOrigin(title.getLocalBounds().width / 2, title.getLocalBounds().height / 2);
            title.setPosition((window.getSize().x / 2), 80);
            int titleColorOffset = (int)(200 + 55 * sin(colorOffset * 0.05));
            title.setFillColor(Color(titleColorOffset, titleColorOffset, 255));
            window.draw(title);

            // show welcome if logged in
            if (loggedIn) {
                string welcome = "Welcome, " + (currentPlayer.nickname.empty() ? currentPlayer.username : currentPlayer.nickname);
                Text welcomeText(welcome, font, 18);
                welcomeText.setPosition(20, 60);
                window.draw(welcomeText);
            }

            // Calculate scroll offset - keep selected item visible in center of screen
            // Maximum 6 buttons fit on typical screen, scroll to keep selected centered
            int maxVisibleButtons = 6;
            int totalButtons = 11;
            if (totalButtons > maxVisibleButtons) {
                int minScroll = max(0, mainMenuIndex - (maxVisibleButtons / 2));
                int maxScroll = max(0, totalButtons - maxVisibleButtons);
                mainMenuScrollOffset = min(minScroll, maxScroll) * 60.0f;
            } else {
                mainMenuScrollOffset = 0.0f;
            }

            // Update button positions with scroll offset
            btnStart.setPosition(180, 150 - mainMenuScrollOffset);
            btnMatchmaking.setPosition(180, 210 - mainMenuScrollOffset);
            btnMultiplayerQuick.setPosition(180, 270 - mainMenuScrollOffset);
            btnLoadGame.setPosition(180, 330 - mainMenuScrollOffset);
            btnLevel.setPosition(180, 390 - mainMenuScrollOffset);
            btnThemes.setPosition(180, 450 - mainMenuScrollOffset);
            btnLeaderboard.setPosition(180, 510 - mainMenuScrollOffset);
            btnFriends.setPosition(180, 570 - mainMenuScrollOffset);
            btnProfile.setPosition(180, 630 - mainMenuScrollOffset);
            btnAchievements.setPosition(180, 690 - mainMenuScrollOffset);
            btnSignOut.setPosition(180, 750 - mainMenuScrollOffset);
            btnExit.setPosition(180, 810 - mainMenuScrollOffset);

            // BUTTON HIGHLIGHTING (keyboard + mouse)
            btnStart.setFillColor((mainMenuIndex == 0 || btnStart.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnMatchmaking.setFillColor((mainMenuIndex == 1 || btnMatchmaking.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnMultiplayerQuick.setFillColor((mainMenuIndex == 2 || btnMultiplayerQuick.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnLoadGame.setFillColor((mainMenuIndex == 3 || btnLoadGame.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnLevel.setFillColor((mainMenuIndex == 4 || btnLevel.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnThemes.setFillColor((mainMenuIndex == 5 || btnThemes.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnLeaderboard.setFillColor((mainMenuIndex == 6 || btnLeaderboard.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnFriends.setFillColor((mainMenuIndex == 7 || btnFriends.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnProfile.setFillColor((mainMenuIndex == 8 || btnProfile.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnAchievements.setFillColor((mainMenuIndex == 9 || btnAchievements.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnSignOut.setFillColor((mainMenuIndex == 10 || btnSignOut.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnExit.setFillColor((mainMenuIndex == 11 || btnExit.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnFullscreen.setFillColor((mainMenuIndex == 12 || btnFullscreen.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);

            window.draw(btnStart); window.draw(btnMatchmaking); window.draw(btnMultiplayerQuick); window.draw(btnLoadGame); window.draw(btnLevel); window.draw(btnThemes); window.draw(btnLeaderboard); window.draw(btnFriends); window.draw(btnProfile); window.draw(btnAchievements); window.draw(btnSignOut); window.draw(btnExit); window.draw(btnFullscreen);
            window.display();
            continue;
        }

        // ==========================
        // LEVEL MENU
        // ==========================
        if(gameState == STATE_LEVEL_MENU){
            lvl1.setFillColor((levelMenuIndex == 0 || lvl1.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            lvl2.setFillColor((levelMenuIndex == 1 || lvl2.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            lvl3.setFillColor((levelMenuIndex == 2 || lvl3.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            lvlBack.setFillColor((levelMenuIndex == 3 || lvlBack.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            window.draw(lvl1); window.draw(lvl2); window.draw(lvl3); window.draw(lvlBack);
            window.display();
            continue;
        }

        // ==========================
        // MODE SELECTION
        // ==========================
        if(gameState == STATE_MODE_SELECT){
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(30, 30, 60));
            window.draw(bg);

            Text title("SELECT GAME MODE", font, 48);
            title.setPosition(100, 50);
            title.setFillColor(Color::Yellow);
            window.draw(title);

            btnSinglePlayer.setFillColor((modeSelectIndex == 0 || btnSinglePlayer.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnMultiplayer.setFillColor((modeSelectIndex == 1 || btnMultiplayer.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnModeBack.setFillColor((modeSelectIndex == 2 || btnModeBack.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);

            window.draw(btnSinglePlayer);
            window.draw(btnMultiplayer);
            window.draw(btnModeBack);

            window.display();
            continue;
        }

        // ==========================
        // SAVED GAMES MENU
        // ==========================
        if(gameState == STATE_SAVED_GAMES){
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(30, 30, 60));
            window.draw(bg);

            Text title("SAVED GAMES", font, 48);
            title.setPosition(100, 50);
            title.setFillColor(Color::Yellow);
            window.draw(title);

            if (playerSaveCount == 0) {
                Text noSaves("No saved games found", font, 28);
                noSaves.setPosition(150, 200);
                noSaves.setFillColor(Color::White);
                window.draw(noSaves);

                Text backText("Press ESC to go back", font, 20);
                backText.setPosition(150, 280);
                backText.setFillColor(Color::Cyan);
                window.draw(backText);
            } else {
                // Display saved games list
                int visibleCount = 5;  // Show 5 saves at a time
                if (playerSaveCount > visibleCount) {
                    int minScroll = max(0, savedGamesMenuIndex - (visibleCount / 2));
                    int maxScroll = max(0, playerSaveCount - visibleCount);
                    int scrollIdx = min(minScroll, maxScroll);
                    savedGamesScrollOffset = scrollIdx * 50.0f;
                }

                for (int i = 0; i < playerSaveCount; i++) {
                    float yPos = 150.0f + (i * 50.0f) - savedGamesScrollOffset;
                    
                    // Skip if off-screen
                    if (yPos < 100 || yPos > 600) continue;

                    SaveInfo& save = playerSaves[i];
                    
                    // Format timestamp
                    char timeStr[64];
                    time_t ts = save.timestamp;
                    struct tm* tm_info = localtime(&ts);
                    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", tm_info);

                    // Create display text
                    string saveDisplay = string(save.saveID) + " - P1:" + to_string(save.p1_score) + 
                                        " P2:" + to_string(save.p2_score) + " [" + timeStr + "]";
                    
                    Text saveText(saveDisplay, font, 18);
                    saveText.setPosition(150, yPos);
                    
                    if (i == savedGamesMenuIndex) {
                        saveText.setFillColor(Color::Yellow);
                    } else {
                        saveText.setFillColor(Color::White);
                    }
                    
                    window.draw(saveText);
                }

                // Instructions
                Text instructions("UP/DOWN to select, ENTER to load, DELETE to remove, ESC to back", font, 14);
                instructions.setPosition(50, 550);
                instructions.setFillColor(Color::Cyan);
                window.draw(instructions);
            }

            window.display();
            continue;
        }

        // ==========================
        // ACHIEVEMENTS MENU
        // ==========================
        if(gameState == STATE_ACHIEVEMENTS){
            // Background
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(20, 20, 40));
            window.draw(bg);

            // Title
            Text achTitle("ACHIEVEMENTS", font, 48);
            achTitle.setPosition(100, 30);
            achTitle.setFillColor(Color::Yellow);
            window.draw(achTitle);

            // Player name
            Text playerName("Player: " + playerAchievements.username, font, 16);
            playerName.setPosition(100, 90);
            playerName.setFillColor(Color::Cyan);
            window.draw(playerName);

            // Achievement stats
            Text stats("Total Points: " + to_string(playerAchievements.totalPoints) + " | " +
                      "Freeze Uses: " + to_string(playerAchievements.allTimeFreezeUses) + " | " +
                      "High Score: " + to_string(playerAchievements.highScore), font, 14);
            stats.setPosition(100, 120);
            stats.setFillColor(Color(200, 200, 200));
            window.draw(stats);

            // Handle scrolling
            if (Keyboard::isKeyPressed(Keyboard::Up)) {
                achievementsScrollOffset += 50.0f;
                if (achievementsScrollOffset > 0) achievementsScrollOffset = 0;
            }
            if (Keyboard::isKeyPressed(Keyboard::Down)) {
                achievementsScrollOffset -= 50.0f;
                // Limit scroll based on content height
                int maxScroll = ACH_TOTAL * 75 + 50;
                if (achievementsScrollOffset < -maxScroll) achievementsScrollOffset = -maxScroll;
            }

            // Display achievements with scroll offset
            int yOffset = 170;
            int achCount = 0;
            for (int i = 0; i < ACH_TOTAL; i++) {
                string achStatus = playerAchievements.unlocked[i] ? "[✓ UNLOCKED]" : "[  LOCKED  ]";
                string achName = getAchievementName((AchievementId)i);
                string achDesc = getAchievementDescription((AchievementId)i);

                float yPos = yOffset + i * 75 + achievementsScrollOffset;

                Text statusText(achStatus, font, 16);
                statusText.setPosition(100, yPos);
                statusText.setFillColor(playerAchievements.unlocked[i] ? Color::Green : Color(150, 150, 150));
                window.draw(statusText);

                Text nameText(achName, font, 22);
                nameText.setPosition(300, yPos);
                nameText.setFillColor(playerAchievements.unlocked[i] ? Color::Yellow : Color::White);
                window.draw(nameText);

                Text descText(achDesc, font, 12);
                descText.setPosition(300, yPos + 28);
                descText.setFillColor(Color(180, 180, 180));
                window.draw(descText);

                achCount++;
            }

            // Count unlocked
            int unlockedCount = 0;
            for (int i = 0; i < ACH_TOTAL; i++) {
                if (playerAchievements.unlocked[i]) unlockedCount++;
            }

            Text progress("Progress: " + to_string(unlockedCount) + "/" + to_string(ACH_TOTAL) + " achievements unlocked", font, 16);
            progress.setPosition(100, yOffset + ACH_TOTAL * 75 + 20 + achievementsScrollOffset);
            progress.setFillColor(Color::Cyan);
            window.draw(progress);

            // Instructions
            Text instructions("UP/DOWN to scroll | Press ESC to go back", font, 14);
            instructions.setPosition(100, 950);
            instructions.setFillColor(Color(150, 150, 150));
            window.draw(instructions);

            // Handle ESC to go back
            static bool escPressed = false;
            if (Keyboard::isKeyPressed(Keyboard::Escape)) {
                if (!escPressed) {
                    gameState = STATE_MAIN_MENU;
                    achievementsScrollOffset = 0.0f;  // Reset scroll on exit
                    escPressed = true;
                }
            } else {
                escPressed = false;
            }

            window.display();
            continue;
        }

        // ==========================
        // MATCHMAKING / GAME ROOM
        // ==========================
        if(gameState == STATE_MATCHMAKING){
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(20, 20, 50));
            window.draw(bg);

            // Update matchmaking
            matchmakingTimer += time;
            gameRoom.updateMatchmaking(matchmakingTimer);

            // Title
            mmTitle.setString("MATCHMAKING");
            mmTitle.setPosition(150, 100);
            mmTitle.setFillColor(Color::Yellow);
            window.draw(mmTitle);

            // Check if match was created FIRST
            if(gameRoom.isMatchActive()){
                MatchPair match = gameRoom.getActiveMatch();
                
                // Show match found message
                Text matchFound("✓ MATCH FOUND!", font, 36);
                matchFound.setPosition(150, 200);
                matchFound.setFillColor(Color::Green);
                window.draw(matchFound);
                
                // Show opponent info
                Text opponentInfo("Opponent: " + match.player2.username + " (Rank " + 
                                to_string(match.player2.leaderboardRank) + ")", font, 20);
                opponentInfo.setPosition(150, 270);
                opponentInfo.setFillColor(Color::Cyan);
                window.draw(opponentInfo);
                
                // Show "Starting game..." with animation
                static float startAnimation = 0.0f;
                startAnimation += time;
                
                Text startingGame("Starting game", font, 18);
                int dots = ((int)(startAnimation * 2) % 4);
                for(int i = 0; i < dots; i++) startingGame.setString(startingGame.getString() + ".");
                startingGame.setPosition(150, 320);
                startingGame.setFillColor(Color::Yellow);
                window.draw(startingGame);
                
                // After brief delay, start the game
                static float matchDelay = 0.0f;
                matchDelay += time;
                if(matchDelay > 1.5f) {
                    fprintf(stderr, "[Main] Match active detected! Starting game: %s vs %s\n", 
                            match.player1.username.c_str(), match.player2.username.c_str());
                    
                    // Initialize multiplayer game with the matched players
                    multiplayerGame.initializeGame();
                    multiplayerGame.setPlayer1PowerUps(points.getPowerUps());
                    multiplayerGame.setPlayer2PowerUps(points.getPowerUps());
                    multiplayerActive = true;
                    gameState = STATE_PLAY_MULTIPLAYER;
                    matchDelay = 0.0f;
                }
            } else {
                // Still searching for match
                mmStatus.setString("Searching for opponent...");
                mmStatus.setPosition(150, 200);
                mmStatus.setFillColor(Color::Cyan);
                window.draw(mmStatus);

                // Queue info
                int queueSize = gameRoom.getQueueSize();
                mmQueueInfo.setString("Waiting players: " + to_string(queueSize + 1) + " (including you)");
                mmQueueInfo.setPosition(150, 260);
                mmQueueInfo.setFillColor(Color::White);
                window.draw(mmQueueInfo);

                // Cancel info
                mmCancel.setString("Press ESC to cancel matchmaking");
                mmCancel.setPosition(150, 320);
                mmCancel.setFillColor(Color(200, 200, 200));
                window.draw(mmCancel);

                // Countdown timer
                int seconds = (int)matchmakingTimer;
                mmCountdown.setString("Wait time: " + to_string(seconds) + "s");
                mmCountdown.setPosition(150, 370);
                mmCountdown.setFillColor(Color::Green);
                window.draw(mmCountdown);

                // Animated dots
                static float dotAnimation = 0.0f;
                dotAnimation += time;
                int dotCount = ((int)(dotAnimation * 3) % 4);
                string dots(dotCount, '.');
                Text animDots(dots, font, 20);
                animDots.setPosition(380, 200);
                animDots.setFillColor(Color::Cyan);
                window.draw(animDots);
            }

            window.display();
            continue;
        }

        // ==========================
        // THEME SELECTION
        // ==========================
        if(gameState == STATE_THEME_SELECTION){
            static int selectedThemeIdx = 0;
            static Theme* displayThemes = nullptr;
            static int themeCount = 0;
            static bool themeListInitialized = false;
            static Clock themeKeyDelay;

            // Initialize theme list on first entry
            if (!themeListInitialized) {
                displayThemes = playerInventory.getAllThemesInOrder(themeCount);
                selectedThemeIdx = 0;
                themeListInitialized = true;
                themeKeyDelay.restart();
            }

            // Handle keyboard input with delay to prevent rapid repeat
            if (themeKeyDelay.getElapsedTime().asMilliseconds() > 150) {
                if (Keyboard::isKeyPressed(Keyboard::Up)) {
                    selectedThemeIdx = (selectedThemeIdx - 1 + themeCount) % themeCount;
                    themeKeyDelay.restart();
                }
                if (Keyboard::isKeyPressed(Keyboard::Down)) {
                    selectedThemeIdx = (selectedThemeIdx + 1) % themeCount;
                    themeKeyDelay.restart();
                }
                if (Keyboard::isKeyPressed(Keyboard::Return)) {
                    bool success = playerInventory.selectTheme(displayThemes[selectedThemeIdx].themeId);
                    if (success) {
                        playerInventory.saveInventoryToFile();
                        // Update current theme color
                        currentThemeColor = parseHexColor(playerInventory.getCurrentThemeColorCode());
                    }
                    themeKeyDelay.restart();
                }
                if (Keyboard::isKeyPressed(Keyboard::Escape)) {
                    gameState = STATE_MAIN_MENU;
                    themeKeyDelay.restart();
                }
            }
            
            // Reset state when exiting
            if (gameState != STATE_THEME_SELECTION) {
                themeListInitialized = false;
                if (displayThemes != nullptr) {
                    delete[] displayThemes;
                    displayThemes = nullptr;
                }
            }

            // Simple background
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(20, 20, 40));
            window.draw(bg);

            // Title
            Text themeTitle("THEME SELECTION", font, 32);
            themeTitle.setPosition(150, 30);
            themeTitle.setFillColor(Color::Yellow);
            window.draw(themeTitle);

            // Instructions
            Text themeInfo("Use UP/DOWN to browse | ENTER to select | ESC to go back", font, 14);
            themeInfo.setPosition(100, 90);
            themeInfo.setFillColor(Color(150, 150, 150));
            window.draw(themeInfo);

            // Display themes
            int yOffset = 150;
            for (int i = 0; i < themeCount; i++) {
                string themeDisplay = displayThemes[i].themeName;
                if (displayThemes[i].isLocked) {
                    themeDisplay += " [LOCKED]";
                }

                Text themeText(themeDisplay, font, 18);
                themeText.setPosition(150, yOffset + i * 50);

                // Highlight selected theme
                if (i == selectedThemeIdx) {
                    themeText.setFillColor(Color::Yellow);
                    
                    // Draw selection box
                    RectangleShape selBox(Vector2f(400, 40));
                    selBox.setPosition(130, yOffset + i * 50 - 5);
                    selBox.setFillColor(Color::Transparent);
                    selBox.setOutlineColor(Color::Green);
                    selBox.setOutlineThickness(2);
                    window.draw(selBox);
                } else {
                    themeText.setFillColor(displayThemes[i].isLocked ? Color(100, 100, 100) : Color::White);
                }
                window.draw(themeText);

                // Show theme description and color
                Text themeDesc(displayThemes[i].description, font, 12);
                themeDesc.setPosition(170, yOffset + i * 50 + 22);
                themeDesc.setFillColor(Color(180, 180, 180));
                window.draw(themeDesc);
            }

            // Current selection info
            if (selectedThemeIdx < themeCount && displayThemes != nullptr) {
                Text currentInfo("Color: " + displayThemes[selectedThemeIdx].colorCode, font, 16);
                currentInfo.setPosition(150, yOffset + themeCount * 50 + 50);
                currentInfo.setFillColor(Color::Cyan);
                window.draw(currentInfo);

                if (displayThemes[selectedThemeIdx].isLocked) {
                    Text lockMsg("This theme is locked. Unlock by achieving more points!", font, 14);
                    lockMsg.setPosition(150, yOffset + themeCount * 50 + 100);
                    lockMsg.setFillColor(Color(255, 100, 100));
                    window.draw(lockMsg);
                }
            }

            Text themeHint("Press ESC to go back", font, 14);
            themeHint.setPosition(100, 900);
            themeHint.setFillColor(Color(150, 150, 150));
            window.draw(themeHint);

            window.display();
            continue;
        }

        // ==========================
        // LEADERBOARD
        // ==========================
        if(gameState == STATE_LEADERBOARD){
            // Simple background
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(20, 20, 40));
            window.draw(bg);

            // Title
            Text lbTitle("LEADERBOARD - Top 10", font, 32);
            lbTitle.setPosition(150, 30);
            lbTitle.setFillColor(Color::Yellow);
            window.draw(lbTitle);

            // Column headers
            Text lbHeaders("Rank    Player                    Score", font, 18);
            lbHeaders.setPosition(100, 100);
            lbHeaders.setFillColor(Color::Cyan);
            window.draw(lbHeaders);

            LeaderboardEntry topPlayers[10];
            int playerCount = 0;
            leaderboard.getTopPlayers(topPlayers, playerCount, 10);
            
            int yOffset = 150;
            if (playerCount == 0) {
                Text lbEmpty("No players on leaderboard yet.", font, 16);
                lbEmpty.setPosition(150, yOffset + 100);
                lbEmpty.setFillColor(Color(180, 180, 180));
                window.draw(lbEmpty);
            } else {
                for (int i = 0; i < playerCount && i < 10; i++) {
                    string rankStr = to_string(i + 1) + ". " + topPlayers[i].username + 
                                   string(40 - topPlayers[i].username.length(), ' ') + 
                                   to_string(topPlayers[i].totalScore);
                    Text rankText(rankStr, font, 16);
                    rankText.setPosition(100, yOffset + i * 35);
                    rankText.setFillColor(i == 0 ? Color::Yellow : (i == 1 ? Color(192, 192, 192) : (i == 2 ? Color(205, 127, 50) : Color::White)));
                    window.draw(rankText);
                }
            }

            Text lbHint("Press ESC to go back", font, 14);
            lbHint.setPosition(100, 900);
            lbHint.setFillColor(Color(150, 150, 150));
            window.draw(lbHint);

            window.display();
            continue;
        }

        // ==========================
        // FRIEND SYSTEM
        // ==========================
        if(gameState == STATE_FRIEND_SYSTEM){
            // Simple background
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(20, 20, 40));
            window.draw(bg);

            // Title
            Text fsTitle("FRIEND SYSTEM", font, 32);
            fsTitle.setPosition(200, 15);
            fsTitle.setFillColor(Color::Yellow);
            window.draw(fsTitle);

            Text userTag("(@" + currentPlayer.username + ")", font, 13);
            userTag.setPosition(300, 55);
            userTag.setFillColor(Color(200, 200, 200));
            window.draw(userTag);

            // Vertical separator
            RectangleShape vSeparator(Vector2f(2, 800));
            vSeparator.setPosition(480, 100);
            vSeparator.setFillColor(Color(100, 100, 100));
            window.draw(vSeparator);

            // ===== LEFT COLUMN: SEARCH & SEND =====
            Text searchTitle("ADD FRIENDS", font, 16);
            searchTitle.setPosition(100, 100);
            searchTitle.setFillColor(Color::Cyan);
            window.draw(searchTitle);

            // Draw search box
            fsSearchBox.setFillColor(friendUIMode == FRIEND_SEARCH ? Color(100, 150, 200) : Color(80, 120, 150));
            fsSearchBox.setPosition(100, 140);
            fsSearchBox.setSize(Vector2f(280, 35));
            window.draw(fsSearchBox);
            
            Text searchPlaceholder("Click to search username...", font, 12);
            if (friendSearchInput.empty()) {
                searchPlaceholder.setString("Click to search username...");
                searchPlaceholder.setFillColor(Color(150, 150, 150));
            } else {
                searchPlaceholder.setString(friendSearchInput.substr(0, 30));
                searchPlaceholder.setFillColor(Color::White);
            }
            searchPlaceholder.setPosition(110, 148);
            window.draw(searchPlaceholder);
            
            // Draw send button
            fsSendButton.setPosition(100, 185);
            fsSendButton.setSize(Vector2f(280, 35));
            fsSendButton.setFillColor(!friendSearchInput.empty() ? Color(80, 200, 80) : Color(60, 140, 60));
            window.draw(fsSendButton);
            Text sendBtnText("SEND REQUEST", font, 14);
            sendBtnText.setPosition(130, 192);
            sendBtnText.setFillColor(Color::White);
            window.draw(sendBtnText);

            // ===== YOUR FRIENDS SECTION (LEFT) =====
            Text friendsTitle("YOUR FRIENDS", font, 16);
            friendsTitle.setPosition(100, 250);
            friendsTitle.setFillColor(Color::Green);
            window.draw(friendsTitle);
            
            char friendsList[50][50];
            int friendCount = 0;
            if (loggedIn) {
                friendSystem.getFriendsList(currentPlayer.username.c_str(), friendsList, friendCount);
            }
            
            int friendsYStart = 290;
            if (friendCount == 0) {
                Text noFriends("No friends yet :(", font, 12);
                noFriends.setPosition(100, friendsYStart);
                noFriends.setFillColor(Color(180, 180, 180));
                window.draw(noFriends);
            } else {
                for (int i = 0; i < friendCount && i < 20; i++) {
                    Text friendText("• " + string(friendsList[i]), font, 12);
                    friendText.setPosition(110, friendsYStart + i * 20);
                    friendText.setFillColor(Color(100, 255, 100));
                    window.draw(friendText);
                }
                if (friendCount > 20) {
                    Text moreText("+ " + to_string(friendCount - 20) + " more", font, 11);
                    moreText.setPosition(110, friendsYStart + 20 * 20);
                    moreText.setFillColor(Color(150, 200, 150));
                    window.draw(moreText);
                }
            }

            // ===== RIGHT COLUMN: PENDING REQUESTS =====
            Text reqTitle("PENDING REQUESTS", font, 16);
            reqTitle.setPosition(530, 100);
            reqTitle.setFillColor(Color::Yellow);
            window.draw(reqTitle);
            
            char requestsList[50][50];
            int requestCount = 0;
            if (loggedIn) {
                friendSystem.getPendingRequests(currentPlayer.username.c_str(), requestsList, requestCount);
            }
            
            int reqsYStart = 140;
            if (requestCount == 0) {
                Text noRequests("No pending requests", font, 12);
                noRequests.setPosition(530, reqsYStart);
                noRequests.setFillColor(Color(180, 180, 180));
                window.draw(noRequests);
            } else {
                for (int i = 0; i < requestCount && i < 15; i++) {
                    // Draw background box for request
                    RectangleShape reqBox(Vector2f(260, 45));
                    reqBox.setPosition(520, reqsYStart + i * 50);
                    reqBox.setFillColor(Color(40, 40, 60));
                    window.draw(reqBox);

                    // From text
                    Text reqText("From: " + string(requestsList[i]), font, 11);
                    reqText.setPosition(530, reqsYStart + i * 50 + 5);
                    reqText.setFillColor(Color(255, 200, 100));
                    window.draw(reqText);
                    
                    // Accept button
                    RectangleShape acceptBtn(Vector2f(60, 25));
                    acceptBtn.setPosition(530, reqsYStart + i * 50 + 18);
                    acceptBtn.setFillColor(Color(50, 180, 50));
                    window.draw(acceptBtn);
                    Text acceptText("Accept", font, 10);
                    acceptText.setPosition(538, reqsYStart + i * 50 + 21);
                    acceptText.setFillColor(Color::White);
                    window.draw(acceptText);
                    
                    // Reject button
                    RectangleShape rejectBtn(Vector2f(60, 25));
                    rejectBtn.setPosition(700, reqsYStart + i * 50 + 18);
                    rejectBtn.setFillColor(Color(180, 50, 50));
                    window.draw(rejectBtn);
                    Text rejectText("Reject", font, 10);
                    rejectText.setPosition(708, reqsYStart + i * 50 + 21);
                    rejectText.setFillColor(Color::White);
                    window.draw(rejectText);
                    
                    // Handle clicks on buttons - mouse tracking for hover
                    Vector2i mouseWinPos = Mouse::getPosition(window);
                    Vector2f mouseLogical((float)mouseWinPos.x / scaleX, (float)mouseWinPos.y / scaleY);
                    
                    // Hover effects
                    if (acceptBtn.getGlobalBounds().contains(mouseLogical)) {
                        acceptBtn.setFillColor(Color(100, 220, 100));
                    }
                    if (rejectBtn.getGlobalBounds().contains(mouseLogical)) {
                        rejectBtn.setFillColor(Color(220, 100, 100));
                    }
                }
                if (requestCount > 15) {
                    Text moreReqs("+ " + to_string(requestCount - 15) + " more", font, 11);
                    moreReqs.setPosition(530, reqsYStart + 15 * 50);
                    moreReqs.setFillColor(Color(200, 200, 100));
                    window.draw(moreReqs);
                }
            }

            // ===== FOOTER =====
            Text fsHint("ESC to go back | Type in search & click Send to add friends", font, 11);
            fsHint.setPosition(80, 920);
            fsHint.setFillColor(Color(150, 150, 150));
            window.draw(fsHint);

            window.display();
            continue;
        }

        // ==========================
        // PLAYER PROFILE
        // ==========================
        if(gameState == STATE_PLAYER_PROFILE){
            // Simple background
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            bg.setFillColor(Color(20, 20, 40));
            window.draw(bg);

            // Title
            Text ppTitle("PLAYER PROFILE", font, 32);
            ppTitle.setPosition(150, 30);
            ppTitle.setFillColor(Color::Yellow);
            window.draw(ppTitle);

            // Player info
            Text ppInfo("", font, 16);
            ppInfo.setFillColor(Color::White);
            if (loggedIn) {
                string profileText = "Username: " + currentPlayer.username + "\n\n";
                profileText += "Nickname: " + (currentPlayer.nickname.empty() ? "(not set)" : currentPlayer.nickname) + "\n\n";
                profileText += "Email: " + (currentPlayer.email.empty() ? "(not set)" : currentPlayer.email) + "\n\n";
                profileText += "Total Points: " + to_string(points.getAllTimeScore()) + "\n\n";
                profileText += "High Score: " + to_string(points.getHighScore()) + "\n\n";
                profileText += "Power-ups: " + to_string(points.getPowerUps());
                ppInfo.setString(profileText);
            } else {
                ppInfo.setString("Not logged in");
            }
            ppInfo.setPosition(120, 120);
            window.draw(ppInfo);

            Text ppHint("Press ESC to go back", font, 14);
            ppHint.setPosition(100, 900);
            ppHint.setFillColor(Color(150, 150, 150));
            window.draw(ppHint);

            window.display();
            continue;
        }

        // ==========================
        // GAME PLAY
        // ==========================
        if(gameState == STATE_PLAY){
            if(!Game){ gameState = STATE_GAMEOVER; continue; }

            if(Keyboard::isKeyPressed(Keyboard::Left)){ dx = -1; dy = 0; }
            if(Keyboard::isKeyPressed(Keyboard::Right)){ dx = 1; dy = 0; }
            if(Keyboard::isKeyPressed(Keyboard::Up)){ dx = 0; dy = -1; }
            if(Keyboard::isKeyPressed(Keyboard::Down)){ dx = 0; dy = 1; }

            if(timer > delay){
                x += dx; y += dy;
                if(x < 0) x = 0; if(x > N-1) x = N-1;
                if(y < 0) y = 0; if(y > M-1) y = M-1;
                if(grid[y][x] == 2) Game = false;
                if(grid[y][x] == 0) grid[y][x] = 2;
                timer = 0;
            }

            // move enemies only if not frozen
            if (!enemiesFrozen) {
                for(int i=0;i<enemyCount;i++) enemies[i].move();
            }

            if(grid[y][x] == 1){
                dx = dy = 0;

                int prevGrid[M][N];
                for (int i = 0; i < M; ++i)
                    for (int j = 0; j < N; ++j)
                        prevGrid[i][j] = grid[i][j];

                // run drop (this will mark some cells -1)
                for(int i=0;i<enemyCount;i++) drop(enemies[i].y/ts, enemies[i].x/ts);

                // convert cells and count how many were newly captured
                int capturedCount = 0;
                for(int i=0;i<M;i++) {
                    for(int j=0;j<N;j++) {
                        if(grid[i][j] == -1) {
                            grid[i][j] = 0;
                        } else {
                            grid[i][j] = 1;
                        }
                        // A newly captured tile is one that wasn't 1 before, but is 1 now
                        if (prevGrid[i][j] != 1 && grid[i][j] == 1) capturedCount++;
                    }
                }

                // Update points using the integrated PointSystem
                if (capturedCount > 0) {
                    int pointsEarned = points.handleTileCapture(capturedCount);
                    
                    // Log points earned
                    ofstream debugLog("debug.log", ios::app);
                    debugLog << "POINTS_EARNED: count=" << capturedCount << " earned=" << pointsEarned << " total=" << points.getAllTimeScore() << endl;
                    debugLog.close();
                    // Optional: create particles, animations, or small popup showing pointsEarned
                    // For now, we keep the HUD updated every frame (below)
                }
            }

            // collision with enemies (if an enemy hits player's trail)
            for(int i=0;i<enemyCount;i++)
                if(grid[enemies[i].y/ts][enemies[i].x/ts] == 2) Game = false;

            // draw grid tiles
            for(int i=0;i<M;i++)
                for(int j=0;j<N;j++){
                    if(grid[i][j] == 0) continue;
                    if(grid[i][j] == 1) sTile.setTextureRect(IntRect(0,0,ts,ts));
                    if(grid[i][j] == 2) sTile.setTextureRect(IntRect(54,0,ts,ts));
                    sTile.setPosition(j * ts * scaleX, i * ts * scaleY);
                    window.draw(sTile);
                }

            // draw player
            sTile.setTextureRect(IntRect(36, 0, ts, ts));
            sTile.setPosition(x * ts * scaleX, y * ts * scaleY);
            window.draw(sTile);

            // draw enemies (rotate for effect)
            sEnemy.rotate(10);
            for(int i=0;i<enemyCount;i++){
                sEnemy.setPosition(enemies[i].x * scaleX, enemies[i].y * scaleY);
                window.draw(sEnemy);
            }

            // ----------------------------
            // Draw HUD (right side)
            // ----------------------------
            // Display username and score info
            static bool hudDebugLogged = false;
            if (!hudDebugLogged) {
                cerr << "DEBUG: First HUD draw - allTime=" << points.getAllTimeScore() 
                     << " powerUps=" << points.getPowerUps() 
                     << " score=" << points.getScore() << endl;
                hudDebugLogged = true;
            }
            
            hudUsername.setString(currentPlayer.nickname.empty() ? currentPlayer.username : currentPlayer.nickname);
            hudSeparator.setString("-------------------------------------");
            hudScore.setString("Score: " + to_string(points.getScore()));
            hudPowerups.setString("Power-ups: " + to_string(points.getPowerUps()));
            hudTotalScore.setString("Total Score: " + to_string(points.getAllTimeScore()));
            hudHighScore.setString("High Score: " + to_string(points.getHighScore()));
            
            // Position HUD items on right side (pixel coords)
            float hudX = (float)window.getSize().x - 220.0f; 
            float hudY = 20.0f;
            
            hudUsername.setPosition(hudX, hudY);
            hudSeparator.setPosition(hudX, hudY + 25.f);
            hudScore.setPosition(hudX, hudY + 40.f);
            hudPowerups.setPosition(hudX, hudY + 60.f);
            hudTotalScore.setPosition(hudX, hudY + 80.f);
            hudHighScore.setPosition(hudX, hudY + 100.f);
            hudHint.setPosition(hudX, hudY + 125.f);

            // Apply theme color to HUD elements
            hudUsername.setFillColor(currentThemeColor);
            hudSeparator.setFillColor(currentThemeColor);
            hudScore.setFillColor(currentThemeColor);
            hudPowerups.setFillColor(currentThemeColor);
            hudTotalScore.setFillColor(currentThemeColor);
            hudHighScore.setFillColor(currentThemeColor);
            hudHint.setFillColor(Color::White);

            window.draw(hudUsername);
            window.draw(hudSeparator);
            window.draw(hudScore);
            window.draw(hudPowerups);
            window.draw(hudTotalScore);
            window.draw(hudHighScore);
            window.draw(hudHint);

            // If freeze is active, show countdown
            if (enemiesFrozen) {
                float remaining = freezeDuration - freezeClock.getElapsedTime().asSeconds();
                if (remaining < 0) remaining = 0;
                Text freezeText(("Enemies frozen: " + to_string((int)ceil(remaining)) + "s"), font, 16);
                freezeText.setPosition(hudX, 110.f);
                freezeText.setFillColor(Color::Cyan);
                window.draw(freezeText);
            }

            window.display();
            continue;
        }

        // ==========================
        // MULTIPLAYER GAME PLAY
        // ==========================
        if(gameState == STATE_PLAY_MULTIPLAYER && !multiplayerGame.gameEnded()){
            // Normal gameplay when at least one player is alive

            // Update game
            multiplayerGame.update(time);

            // Draw grid
            for(int i=0;i<M;i++){
                for(int j=0;j<N;j++){
                    int cell = multiplayerGame.getGrid(i, j);
                    if(cell == 0) continue;
                    if(cell == 1) sTile.setTextureRect(IntRect(0,0,ts,ts));      // Border
                    if(cell == 2) sTile.setTextureRect(IntRect(54,0,ts,ts));     // Player 1 trail
                    if(cell == 3) sTile.setTextureRect(IntRect(72,0,ts,ts));     // Player 2 trail
                    sTile.setPosition(j * ts * scaleX, i * ts * scaleY);
                    window.draw(sTile);
                }
            }

            // Draw Player 1
            const GamePlayer& p1 = multiplayerGame.getPlayer1();
            if(p1.alive){
                sTile.setTextureRect(IntRect(36, 0, ts, ts));
                sTile.setPosition(p1.x * ts * scaleX, p1.y * ts * scaleY);
                window.draw(sTile);
            }

            // Draw Player 2
            const GamePlayer& p2 = multiplayerGame.getPlayer2();
            if(p2.alive){
                sTile.setTextureRect(IntRect(54, 0, ts, ts));
                sTile.setPosition(p2.x * ts * scaleX, p2.y * ts * scaleY);
                window.draw(sTile);
            }

            // Draw enemies (rotate for effect)
            sEnemy.rotate(10);
            for(int i=0; i<multiplayerGame.getEnemyCount(); i++){
                const MPEnemy& enemy = multiplayerGame.getEnemies()[i];
                sEnemy.setPosition(enemy.x * scaleX, enemy.y * scaleY);
                window.draw(sEnemy);
            }

            // Draw HUD - Player 1 (left side) and Player 2 (right side)
            // ========== PLAYER 1 HUD (LEFT) ==========
            Text p1Title("PLAYER 1", font, 18);
            p1Title.setPosition(15, 8);
            p1Title.setFillColor(Color::Yellow);
            p1Title.setStyle(Text::Bold);
            window.draw(p1Title);

            Text p1Controls("Arrows • E=PU", font, 12);
            p1Controls.setPosition(15, 32);
            p1Controls.setFillColor(Color::White);
            window.draw(p1Controls);

            // P1 Score Box - Large and prominent
            Text p1Score("SCORE", font, 12);
            p1Score.setPosition(15, 50);
            p1Score.setFillColor(currentThemeColor);
            p1Score.setStyle(Text::Bold);
            window.draw(p1Score);

            Text p1ScoreValue(to_string(p1.score), font, 28);
            p1ScoreValue.setPosition(15, 65);
            p1ScoreValue.setFillColor(Color::Green);
            p1ScoreValue.setStyle(Text::Bold);
            window.draw(p1ScoreValue);

            // P1 Power-ups
            Text p1PU("Power-ups: " + to_string(p1.powerUps), font, 14);
            p1PU.setPosition(15, 100);
            p1PU.setFillColor(Color::Magenta);
            p1PU.setStyle(Text::Bold);
            window.draw(p1PU);

            // P1 Status
            Text p1Frozen("", font, 14);
            if(multiplayerGame.isPlayerFrozen(0)) {
                p1Frozen.setString("❄ FROZEN!");
                p1Frozen.setFillColor(Color::Cyan);
                p1Frozen.setStyle(Text::Bold);
                p1Frozen.setPosition(15, 120);
                window.draw(p1Frozen);
            }

            // ========== PLAYER 2 HUD (RIGHT) ==========
            Text p2Title("PLAYER 2", font, 18);
            p2Title.setPosition((float)window.getSize().x - 200, 8);
            p2Title.setFillColor(Color::Cyan);
            p2Title.setStyle(Text::Bold);
            window.draw(p2Title);

            Text p2Controls("WASD • P=PU", font, 12);
            p2Controls.setPosition((float)window.getSize().x - 200, 32);
            p2Controls.setFillColor(Color::White);
            window.draw(p2Controls);

            // P2 Score Box - Large and prominent
            Text p2Score("SCORE", font, 12);
            p2Score.setPosition((float)window.getSize().x - 200, 50);
            p2Score.setFillColor(currentThemeColor);
            p2Score.setStyle(Text::Bold);
            window.draw(p2Score);

            Text p2ScoreValue(to_string(p2.score), font, 28);
            p2ScoreValue.setPosition((float)window.getSize().x - 200, 65);
            p2ScoreValue.setFillColor(Color::Cyan);
            p2ScoreValue.setStyle(Text::Bold);
            window.draw(p2ScoreValue);

            // P2 Power-ups
            Text p2PU("Power-ups: " + to_string(p2.powerUps), font, 14);
            p2PU.setPosition((float)window.getSize().x - 200, 100);
            p2PU.setFillColor(Color::Magenta);
            p2PU.setStyle(Text::Bold);
            window.draw(p2PU);

            // P2 Status
            Text p2Frozen("", font, 14);
            if(multiplayerGame.isPlayerFrozen(1)) {
                p2Frozen.setString("❄ FROZEN!");
                p2Frozen.setFillColor(Color::Cyan);
                p2Frozen.setStyle(Text::Bold);
                p2Frozen.setPosition((float)window.getSize().x - 200, 120);
                window.draw(p2Frozen);
            }

            // Timer
            Text timer("Time: " + to_string((int)multiplayerGame.getGameTimer()) + "s", font, 20);
            timer.setPosition((float)window.getSize().x / 2 - 50, 10);
            timer.setFillColor(currentThemeColor);
            window.draw(timer);

            // Freeze indicator
            if(multiplayerGame.areEnemiesFrozen()) {
                Text freezeText("ENEMIES FROZEN!", font, 16);
                freezeText.setPosition((float)window.getSize().x / 2 - 100, 50);
                freezeText.setFillColor(Color::Magenta);
                window.draw(freezeText);
            }

            // Display death message if player just died
            string deathMsg = multiplayerGame.getDeathMessage();
            if(!deathMsg.empty() && multiplayerGame.getDeathMessageTimer() < multiplayerGame.DEATH_MESSAGE_DURATION) {
                Text deathText(deathMsg, font, 36);
                deathText.setOrigin(deathText.getLocalBounds().width/2, deathText.getLocalBounds().height/2);
                deathText.setPosition((float)window.getSize().x / 2, (float)window.getSize().y / 2);
                deathText.setFillColor(Color::Red);
                window.draw(deathText);
            }

            // ==========================
            // PAUSE MENU RENDERING
            // ==========================
            if (gamePaused) {
                // Semi-transparent overlay
                RectangleShape overlay(Vector2f(window.getSize().x, window.getSize().y));
                overlay.setFillColor(Color(0, 0, 0, 128));
                window.draw(overlay);
                
                if (!pauseLoadGameMode) {
                    // ===== PAUSE MENU =====
                    float menuWidth = 300.f;
                    float menuHeight = 200.f;
                    float menuX = (float)window.getSize().x / 2 - menuWidth / 2;
                    float menuY = (float)window.getSize().y / 2 - menuHeight / 2;
                    
                    RectangleShape menuBg(Vector2f(menuWidth, menuHeight));
                    menuBg.setPosition(menuX, menuY);
                    menuBg.setFillColor(Color::Black);
                    menuBg.setOutlineColor(currentThemeColor);
                    menuBg.setOutlineThickness(3.f);
                    window.draw(menuBg);
                    
                    // Title
                    Text pauseTitle("PAUSED", font, 28);
                    pauseTitle.setFillColor(currentThemeColor);
                    pauseTitle.setStyle(Text::Bold);
                    pauseTitle.setOrigin(pauseTitle.getLocalBounds().width/2, 0);
                    pauseTitle.setPosition(menuX + menuWidth/2, menuY + 15.f);
                    window.draw(pauseTitle);
                    
                    // Menu options - 3 options only (Resume, Save, Quit)
                    vector<string> options = {"Resume", "Save Game", "Quit to Menu"};
                    float optionY = menuY + 60.f;
                    float optionSpacing = 40.f;
                    
                    for (int i = 0; i < 3; i++) {
                        Text option(options[i], font, 18);
                        option.setPosition(menuX + 30.f, optionY + (i * optionSpacing));
                        
                        if (i == pauseMenuSelection) {
                            // Highlight selected option
                            option.setFillColor(Color::Yellow);
                            option.setStyle(Text::Bold);
                            Text selector("> ", font, 18);
                            selector.setFillColor(Color::Yellow);
                            selector.setPosition(menuX + 10.f, optionY + (i * optionSpacing));
                            window.draw(selector);
                        } else {
                            option.setFillColor(Color::White);
                        }
                        window.draw(option);
                    }
                    
                    // Instructions
                    Text instructions("UP/DOWN or Mouse, ENTER/Click to confirm, ESC to resume", font, 12);
                    instructions.setFillColor(Color(150, 150, 150));
                    instructions.setOrigin(instructions.getLocalBounds().width/2, 0);
                    instructions.setPosition(menuX + menuWidth/2, menuY + menuHeight - 25.f);
                    window.draw(instructions);
                }
            }

            window.display();
            continue;
        }

        // ==========================
        // MULTIPLAYER DEATH MESSAGE SCREEN (waiting for 2 seconds)
        // ==========================
        if(gameState == STATE_PLAY_MULTIPLAYER && multiplayerGame.gameEnded() && multiplayerGame.getDeathMessageTimer() < multiplayerGame.DEATH_MESSAGE_DURATION){
            // Continue updating to advance the death message timer
            multiplayerGame.update(time);
            
            // Draw the game grid in background
            for(int i=0;i<M;i++){
                for(int j=0;j<N;j++){
                    int cell = multiplayerGame.getGrid(i, j);
                    if(cell == 0) continue;
                    if(cell == 1) sTile.setTextureRect(IntRect(0,0,ts,ts));      // Border
                    if(cell == 2) sTile.setTextureRect(IntRect(54,0,ts,ts));     // Player 1 trail
                    if(cell == 3) sTile.setTextureRect(IntRect(72,0,ts,ts));     // Player 2 trail
                    sTile.setPosition(j * ts * scaleX, i * ts * scaleY);
                    window.draw(sTile);
                }
            }

            // Display death message prominently
            string deathMsg = multiplayerGame.getDeathMessage();
            Text deathText(deathMsg, font, 48);
            deathText.setOrigin(deathText.getLocalBounds().width/2, deathText.getLocalBounds().height/2);
            deathText.setPosition((float)window.getSize().x / 2, (float)window.getSize().y / 2);
            deathText.setFillColor(Color::Red);
            window.draw(deathText);

            window.display();
            continue;
        }

        // ==========================
        // MULTIPLAYER GAME OVER
        // ==========================
        if(gameState == STATE_PLAY_MULTIPLAYER && multiplayerGame.gameEnded() && multiplayerGame.getDeathMessageTimer() >= multiplayerGame.DEATH_MESSAGE_DURATION){
            // Update achievements with the match results ONCE
            const GamePlayer& endP1 = multiplayerGame.getPlayer1();
            const GamePlayer& endP2 = multiplayerGame.getPlayer2();
            int maxMatchScore = max(endP1.score, endP2.score);
            
            // Use a gameEnded state check to update achievements only once
            static int lastGameEndScore = -1;
            if (lastGameEndScore != maxMatchScore || lastGameEndScore == -1) {
                updateAndCheckAchievements(playerAchievements, maxMatchScore, 0, 0);
                lastGameEndScore = maxMatchScore;
                cout << "[Achievements] Updated with match score: " << maxMatchScore << endl;
            }
            
            colorOffset += 0.5f;
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            int r=(int)(50+50*sin(colorOffset*0.02));
            int g=(int)(50+50*sin(colorOffset*0.03));
            int b=(int)(100+50*sin(colorOffset*0.01));
            bg.setFillColor(Color(r,g,b));
            window.draw(bg);

            Text title("GAME OVER", font, 40);
            title.setOrigin(title.getLocalBounds().width/2, title.getLocalBounds().height/2);
            title.setPosition(window.getSize().x / 2, 25);
            title.setFillColor(Color::Yellow);
            window.draw(title);

            // Show winner
            int winner = multiplayerGame.getWinner();
            const GamePlayer& p1 = multiplayerGame.getPlayer1();
            const GamePlayer& p2 = multiplayerGame.getPlayer2();

            string resultText;
            Color resultColor;
            if(winner == 1){
                resultText = "PLAYER 1 WINS!";
                resultColor = Color::Yellow;
            } else if(winner == 2){
                resultText = "PLAYER 2 WINS!";
                resultColor = Color::Cyan;
            } else {
                resultText = "TIE!";
                resultColor = Color::Magenta;
            }

            // WINNER TEXT
            Text resultDisplay(resultText, font, 32);
            resultDisplay.setOrigin(resultDisplay.getLocalBounds().width/2, resultDisplay.getLocalBounds().height/2);
            resultDisplay.setPosition((float)window.getSize().x / 2, 75);
            resultDisplay.setFillColor(resultColor);
            window.draw(resultDisplay);

            Text scoreCompare("P1: " + to_string(p1.score) + "  vs  P2: " + to_string(p2.score), font, 20);
            scoreCompare.setOrigin(scoreCompare.getLocalBounds().width/2, scoreCompare.getLocalBounds().height/2);
            scoreCompare.setPosition((float)window.getSize().x / 2, 120);
            scoreCompare.setFillColor(Color::Yellow);
            window.draw(scoreCompare);

            // Additional stats
            Text p1Stats("Player 1 - Score: " + to_string(p1.score), font, 14);
            p1Stats.setPosition(50, 170);
            p1Stats.setFillColor(Color::Yellow);
            window.draw(p1Stats);

            Text p2Stats("Player 2 - Score: " + to_string(p2.score), font, 14);
            p2Stats.setPosition((float)window.getSize().x - 300, 170);
            p2Stats.setFillColor(Color::Yellow);
            window.draw(p2Stats);

            // Position buttons in the center of the screen, clearly visible
            float btnCenterX = (float)window.getSize().x / 2;
            float btnStartY = 250.0f;  // Moved up from 300.0f
            float btnSpacing = 60.0f;
            
            btnMPRestart.setPosition(btnCenterX - btnMPRestart.getLocalBounds().width / 2, btnStartY);
            btnMPMainMenu.setPosition(btnCenterX - btnMPMainMenu.getLocalBounds().width / 2, btnStartY + btnSpacing);
            btnMPQuit.setPosition(btnCenterX - btnMPQuit.getLocalBounds().width / 2, btnStartY + btnSpacing * 2);

            btnMPRestart.setFillColor((mpGameOverMenuIndex == 0 || btnMPRestart.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnMPMainMenu.setFillColor((mpGameOverMenuIndex == 1 || btnMPMainMenu.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnMPQuit.setFillColor((mpGameOverMenuIndex == 2 || btnMPQuit.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);

            window.draw(btnMPRestart);
            window.draw(btnMPMainMenu);
            window.draw(btnMPQuit);

            window.display();
            continue;
        }

        // ==========================
        // GAME OVER
        // ==========================
        if(gameState == STATE_GAMEOVER){
            // Update achievements for single-player game
            static int lastSinglePlayerScore = -1;
            if (lastSinglePlayerScore != points.getScore() || lastSinglePlayerScore == -1) {
                updateAndCheckAchievements(playerAchievements, points.getScore(), 0, 0);
                lastSinglePlayerScore = points.getScore();
                cout << "[Achievements] Updated with single-player score: " << points.getScore() << endl;
            }
            
            colorOffset += 0.5f;
            RectangleShape bg(Vector2f(window.getSize().x, window.getSize().y));
            int r=(int)(50+50*sin(colorOffset*0.02));
            int g=(int)(50+50*sin(colorOffset*0.03));
            int b=(int)(100+50*sin(colorOffset*0.01));
            bg.setFillColor(Color(r,g,b));
            window.draw(bg);

            for(int i=0;i<50;i++){
                CircleShape c(particles[i].radius);
                c.setPosition(Vector2f(particles[i].pos.x * scaleX, particles[i].pos.y * scaleY));
                c.setFillColor(particles[i].color);
                window.draw(c);
                particles[i].pos += particles[i].vel;
                if(particles[i].pos.x < 0) particles[i].pos.x = N*ts;
                if(particles[i].pos.x > N*ts) particles[i].pos.x = 0;
                if(particles[i].pos.y < 0) particles[i].pos.y = M*ts;
                if(particles[i].pos.y > M*ts) particles[i].pos.y = 0;
            }

            Text title("GAME OVER", font, 64);
            title.setOrigin(title.getLocalBounds().width/2, title.getLocalBounds().height/2);
            title.setPosition(window.getSize().x / 2, 80);
            int titleColorOffset = (int)(255 + 0.5 * 50 * sin(colorOffset * 0.05));
            title.setFillColor(Color(255, titleColorOffset % 256, titleColorOffset % 256));
            window.draw(title);

            btnRestart.setFillColor((gameOverMenuIndex == 0 || btnRestart.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnMainMenu.setFillColor((gameOverMenuIndex == 1 || btnMainMenu.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);
            btnQuit.setFillColor((gameOverMenuIndex == 2 || btnQuit.getGlobalBounds().contains(mouseLogical)) ? Color::Yellow : Color::White);

            window.draw(btnRestart); window.draw(btnMainMenu); window.draw(btnQuit);
            window.display();
            continue;
        }
    }

    return 0;
}





