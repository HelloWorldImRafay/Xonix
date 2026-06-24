
                                    // =================================
                                    // |    24i-2008 Abdul Raffay      |
                                    // | 24i-2130  Muhammad Hamza Adil |
                                    // |       " XONIX GAME "          |
                                    // |       THEME HEADER FILE       |
                                    // =================================

#ifndef THEME_H
#define THEME_H
#include <string>
using namespace std;

// STRUCT FOR THEME
// Game themes ka data store krta hai
    struct Theme {
            int themeId;              
            string themeName;    
            string description; 
            string colorCode;    
            bool isLocked;            
            
            Theme() : themeId(0), themeName(""), description(""), colorCode(""), isLocked(true) {}
            
            Theme(int id, const string& name, const string& desc, 
                const string& color, bool locked = false)
                : themeId(id), themeName(name), description(desc), 
                colorCode(color), isLocked(locked) {}
            
            bool operator<(const Theme& other) const {
                return themeId < other.themeId;
            }
            
            bool operator>(const Theme& other) const {
                return themeId > other.themeId;
            }
            
            bool operator==(const Theme& other) const {
                return themeId == other.themeId;
            }
        };

        #endif 
