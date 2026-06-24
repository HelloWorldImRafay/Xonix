
                                            // =================================
                                            // |    24i-2008 Abdul Raffay      |
                                            // | 24i-2130  Muhammad Hamza Adil |
                                            // |       " XONIX GAME "          |
                                            // |     INVENTORY HEADER FILE     |
                                            // =================================

#ifndef INVENTORY_H
#define INVENTORY_H

#include "AVLTree.h"
#include <string>
#include <fstream>
using namespace std;


// CLASS FOR INVENTORY MANAGEMENT
// Player ke themes ko manage krne k liye Inventory class
    class Inventory {
        private:
            AVLTree themeTree;           
            int currentThemeId;         
            string saveFilePath;   
            
            void initializeDefaultThemes();
            bool loadFromFile();
            
        public:
            Inventory();
            ~Inventory();
            
            void initialize(const string& filePath = "inventory_data.txt");
            void addTheme(const Theme& theme);
            void removeTheme(int themeId);
            Theme* getTheme(int themeId);
            Theme* getThemeByName(const string& themeName);
            Theme* getCurrentTheme();
            Theme* getAllThemesInOrder(int& count);  
            int getTotalThemesCount();
            int getTreeHeight(); 
            bool selectTheme(int themeId);
            int getCurrentThemeId();
            string getCurrentThemeColorCode();  
            void saveInventoryToFile();
            void loadInventoryFromFile(); 
            void displayAllThemes();
            void displayTheme(const Theme& theme);
            void clear();
        };

#endif
