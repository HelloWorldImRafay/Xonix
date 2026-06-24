                                    // =================================
                                    // |    24i-2008 Abdul Raffay      |
                                    // | 24i-2130  Muhammad Hamza Adil |
                                    // |       " XONIX GAME "          |
                                    // |      INVENTORY CPP FILE       |
                                    // =================================


#include "Inventory.h"
#include <iostream>
using namespace std;



// INITIALIZE DEFAULT THEMES FUNCTION
// Default themes ko inventory mai add krta hai
void Inventory::initializeDefaultThemes() {
    Theme themes[] = {
        Theme(1, "Classic Blue", "Traditional blue and white theme", "#1E90FF", false),
        Theme(2, "Dark Mode", "Elegant dark theme for easy on the eyes", "#1A1A1A", false),
        Theme(3, "Forest Green", "Nature-inspired green theme", "#228B22", false),
        Theme(4, "Sunset Orange", "Warm orange and yellow tones", "#FF8C00", false),
        Theme(5, "Purple Dreams", "Mystical purple theme", "#9932CC", false),
        Theme(6, "Cyberpunk Neon", "Neon-lit futuristic theme", "#FF10F0", false),  
        Theme(7, "Ocean Wave", "Cool cyan and blue theme", "#00CED1", false),
        Theme(8, "Fire Red", "Intense red theme", "#DC143C", false),
        Theme(9, "Mint Fresh", "Cool mint green theme", "#98FF98", false),
        Theme(10, "Gold Premium", "Luxurious gold theme", "#FFD700", false),  
    };

    
    int themeCount = sizeof(themes) / sizeof(themes[0]);
    for (int i = 0; i < themeCount; i++) {
        themeTree.insert(themes[i]);
    }
    
    currentThemeId = 1;
}


// CONSTRUCTOR AND DESTRUCTOR FOR INVENTORY
            Inventory::Inventory() : currentThemeId(1), saveFilePath("") {
                initializeDefaultThemes();
            }

            Inventory::~Inventory() {
                themeTree.clear();
            }



// INITIALIZE FUNCTION
// Inventory ko initialize krta hai file path k sath ya default themes k sath
            void Inventory::initialize(const string& filePath) {
                saveFilePath = filePath;
                if (!loadFromFile()) {
                    initializeDefaultThemes();
                }
            }


// ADD THEME FUNCTION
// Naya theme inventory mai add krta hai
            void Inventory::addTheme(const Theme& theme) {
                themeTree.insert(theme);
            }


// REMOVE THEME FUNCTION
// Theme inventory se remove krta hai agar wo current theme nahi hai
            void Inventory::removeTheme(int themeId) {
                if (themeId != currentThemeId) {  
                    themeTree.deleteTheme(themeId);
                }
            }

// GET THEME FUNCTION
// Theme ko themeId k basis par return krta hai
            Theme* Inventory::getTheme(int themeId) {
                return themeTree.search(themeId);
            }

// GET THEME BY NAME FUNCTION
// Theme ko themeName k basis par return krta hai
            Theme* Inventory::getThemeByName(const string& themeName) {
                return themeTree.searchByName(themeName);
            }


// GET CURRENT THEME FUNCTION
// Current selected theme ko return krta hai
            Theme* Inventory::getCurrentTheme() {
                return getTheme(currentThemeId);
            }


// GET ALL THEMES IN ORDER FUNCTION
// Sare themes ko sorted array mai return krta hai
            Theme* Inventory::getAllThemesInOrder(int& count) {
                count = themeTree.getTotalThemes();
                
                if (count <= 0) {
                    return nullptr;
                }
                
                Theme* themesArray = new Theme[count];
                int actualCount = 0;
                themeTree.getAllThemesToArray(themesArray, count, actualCount);
                
                count = actualCount;
                return themesArray;
            }


// GET TOTAL THEMES COUNT FUNCTION
// Total themes ki count return krta hai
            int Inventory::getTotalThemesCount() {
                return themeTree.getTotalThemes();
            }

// GET TREE HEIGHT FUNCTION
// Theme tree ki height return krta hai
            int Inventory::getTreeHeight() {
                return themeTree.getTreeHeight();
            }


// SELECT THEME FUNCTION
// Theme ko select krta hai agar wo locked nahi hai
            bool Inventory::selectTheme(int themeId) {
                Theme* theme = getTheme(themeId);
                if (theme != nullptr) {
                    if (theme->isLocked) {
                        cout << "Theme " << themeId << " is locked and cannot be selected." << endl;
                        return false;
                    }
                    currentThemeId = themeId;
                    return true;
                }
                return false;
            }

// GET CURRENT THEME ID FUNCTION
// Current selected theme ka ID return krta hai
            int Inventory::getCurrentThemeId() {
                return currentThemeId;
            }

// GET CURRENT THEME COLOR CODE FUNCTION
// Current selected theme ka color code return krta hai
            string Inventory::getCurrentThemeColorCode() {
                Theme* current = getCurrentTheme();
                if (current) {
                    return current->colorCode;
                }
                return "#1E90FF";  
            }

// SAVE INVENTORY TO FILE FUNCTION
// Inventory ko specified file mai save krta hai
            void Inventory::saveInventoryToFile() {
                if (saveFilePath.empty()) {
                    cout << "Save file path not set." << endl;
                    return;
                }


                ofstream file(saveFilePath);
                if (!file.is_open()) {
                    cout << "Failed to open file for saving: " << saveFilePath << endl;
                    return;
                }
                
                file << "CURRENT_THEME:" << currentThemeId << "\n";
                
                int themeCount = 0;
                Theme* themes = getAllThemesInOrder(themeCount);
                file << "TOTAL_THEMES:" << themeCount << "\n";
                
                if (themes != nullptr) {
                    for (int i = 0; i < themeCount; i++) {
                        file << "THEME_ID:" << themes[i].themeId << "|"
                            << "NAME:" << themes[i].themeName << "|"
                            << "DESC:" << themes[i].description << "|"
                            << "COLOR:" << themes[i].colorCode << "|"
                            << "LOCKED:" << (themes[i].isLocked ? 1 : 0) << "\n";
                    }
                    delete[] themes;
                }
                
                file.close();
                cout << "Inventory saved to: " << saveFilePath << endl;
            }

// LOAD INVENTORY FROM FILE FUNCTION
// Inventory ko specified file se load krta hai

            bool Inventory::loadFromFile() {
                if (saveFilePath.empty()) {
                    return false;
                }
                
                ifstream file(saveFilePath);
                if (!file.is_open()) {
                    return false;  
                }
                
                string line;
                int totalThemes = 0;
                
                while (getline(file, line)) {
                    if (line.empty()) continue;
                    
                    if (line.find("CURRENT_THEME:") == 0) {
                        currentThemeId = stoi(line.substr(14));
                    } else if (line.find("TOTAL_THEMES:") == 0) {
                        totalThemes = stoi(line.substr(13));
                    } else if (line.find("THEME_ID:") == 0) {
                        int themeId = 0, locked = 0;
                        string name = "", desc = "", color = "";
                        
                        size_t pos = 0;
                        string token;
                        string data = line;
                        
                        while ((pos = data.find("|")) != string::npos) {
                            token = data.substr(0, pos);
                            
                            if (token.find("THEME_ID:") == 0) {
                                themeId = stoi(token.substr(9));
                            } else if (token.find("NAME:") == 0) {
                                name = token.substr(5);
                            } else if (token.find("DESC:") == 0) {
                                desc = token.substr(5);
                            } else if (token.find("COLOR:") == 0) {
                                color = token.substr(6);
                            }
                            
                            data.erase(0, pos + 1);
                        }
                        
                        if (data.find("LOCKED:") == 0) {
                            locked = stoi(data.substr(7));
                        }
                        
                        Theme theme(themeId, name, desc, color, locked == 1);
                        themeTree.insert(theme);
                    }
                }
                
                file.close();
                return true;
            }


// DISPLAY ALL THEMES FUNCTION
// Sare themes ko console par display krta hai
            void Inventory::displayAllThemes() {
                int themeCount = 0;
                Theme* themes = getAllThemesInOrder(themeCount);
                
                cout << "\n=== THEME INVENTORY ===\n";
                cout << "Total Themes: " << themeCount << " | Tree Height: " << getTreeHeight() << "\n\n";
                
                if (themes != nullptr) {
                    for (int i = 0; i < themeCount; i++) {
                        displayTheme(themes[i]);
                    }
                    delete[] themes;
                }
                
                cout << "\nCurrent Theme: " << currentThemeId << "\n";
            }

            void Inventory::displayTheme(const Theme& theme) {
                cout << "[ID: " << theme.themeId << "] " << theme.themeName << "\n";
                cout << "  Description: " << theme.description << "\n";
                cout << "  Color Code: " << theme.colorCode << "\n";
                cout << "  Status: " << (theme.isLocked ? "🔒 LOCKED" : "✓ UNLOCKED") << "\n\n";
            }

            void Inventory::clear() {
                themeTree.clear();
                currentThemeId = 1;
            }
