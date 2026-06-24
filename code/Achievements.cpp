#include "Achievements.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
// ========================================
// HELPER: Get Achievement Names
// ========================================
std::string getAchievementName(AchievementId id) {
    switch (id) {
        case ACH_RISING_STAR:       return "Rising Star";
        case ACH_FREEZE_MASTER:     return "Freeze Master";
        case ACH_SCORE_TITAN:       return "Score Titan";
        case ACH_ALL_TIME_SCORER:   return "All-Time Scorer";
        default:                    return "Unknown Achievement";
    }
}

// ========================================
// HELPER: Get Achievement Descriptions
// ========================================
std::string getAchievementDescription(AchievementId id) {
    switch (id) {
        case ACH_RISING_STAR:       return "Reach a total of 500 points.";
        case ACH_FREEZE_MASTER:     return "Use Freeze 10 times total.";
        case ACH_SCORE_TITAN:       return "Hit 2000 score in a single run.";
        case ACH_ALL_TIME_SCORER:   return "Accumulate 5000 total points.";
        default:                    return "Unknown achievement condition.";
    }
}

// ========================================
// IMPLEMENTATION: Initialize Achievement Data
// ========================================
void initAchievementData(AchievementData &data, const std::string &username, bool resetUnlocks) {
    std::cout << "[Achievements] DEBUG: initAchievementData called for: " << username << " with resetUnlocks=" << resetUnlocks << std::endl;
    
    data.username = username;
    data.totalPoints = 0;
    data.allTimeFreezeUses = 0;
    data.highScore = 0;

    // Only reset unlocked array when resetUnlocks == true (new user)
    // When false, preserve existing achievement unlock state for login
    if (resetUnlocks) {
        std::cout << "[Achievements] DEBUG: Resetting all achievements to LOCKED (new user)" << std::endl;
        for (int i = 0; i < ACH_TOTAL; i++) {
            data.unlocked[i] = false;
        }
    } else {
        std::cout << "[Achievements] DEBUG: PRESERVING existing achievements (login - about to load from file)" << std::endl;
    }
}

// ========================================
// IMPLEMENTATION: Load Achievements from File
// ========================================
bool loadAchievements(AchievementData &data) {
    // Construct file path: data/<username>_ach.txt
    std::string filePath = "data/" + data.username + "_ach.txt";

    std::cout << "[Achievements] DEBUG: Attempting to load from: " << filePath << std::endl;

    // Open file for reading
    std::ifstream file(filePath.c_str());

    // Check if file exists and is open
    if (!file.is_open()) {
        std::cout << "[Achievements] File not found: " << filePath << std::endl;
        return false;
    }

    std::cout << "[Achievements] DEBUG: File opened successfully" << std::endl;

    // Read username (should match)
    std::string tempUsername;
    file >> tempUsername;

    // Read counters
    file >> data.totalPoints;
    file >> data.allTimeFreezeUses;
    file >> data.highScore;

    std::cout << "[Achievements] DEBUG: Loaded counters - totalPoints=" << data.totalPoints 
              << " freeze=" << data.allTimeFreezeUses << " highScore=" << data.highScore << std::endl;

    // Read unlock status (4 bool values - after removing Power Cascade)
    for (int i = 0; i < ACH_TOTAL; i++) {
        int value;
        file >> value;
        data.unlocked[i] = (value == 1);
        std::cout << "[Achievements] DEBUG: Achievement " << i << " = " << (data.unlocked[i] ? "UNLOCKED" : "LOCKED") << std::endl;
    }

    // Close file
    file.close();

    std::cout << "[Achievements] SUCCESS: Loaded achievements for: " << data.username 
              << " (totalPoints=" << data.totalPoints << ", highScore=" << data.highScore << ")" << std::endl;
    return true;
}

// ========================================
// IMPLEMENTATION: Load Player Stats from PointSystem File
// ========================================
void loadPlayerStatsForAchievements(AchievementData &data) {
    // First, try to load totalPoints from PlayerProfile file: <username>_profile.txt
    std::string profilePath = data.username + "_profile.txt";
    std::ifstream profileFile(profilePath.c_str());
    
    int totalPoints = 0;
    if (profileFile.is_open()) {
        int playerId;
        std::string username, nickname, email;
        
        // Read in order: playerId, username, nickname, email, totalPoints
        profileFile >> playerId;
        profileFile.ignore();
        std::getline(profileFile, username);
        std::getline(profileFile, nickname);
        std::getline(profileFile, email);
        profileFile >> totalPoints;
        
        profileFile.close();
        
        std::cout << "[Achievements] Loaded totalPoints from profile: " << profilePath 
                  << " (points=" << totalPoints << ")" << std::endl;
    } else {
        std::cout << "[Achievements] Profile file not found: " << profilePath << std::endl;
        totalPoints = 0;
    }
    
    // Second, load highScore from PointSystem file: <username>_data.txt
    std::string pointsPath = data.username + "_data.txt";
    std::ifstream pointsFile(pointsPath.c_str());
    
    int highScore = 0;
    if (pointsFile.is_open()) {
        int allTimeScore, powerUps, nextPowerUpScore;
        
        // Read in order: allTimeScore, highScore, powerUps, nextPowerUpScore
        pointsFile >> allTimeScore >> highScore >> powerUps >> nextPowerUpScore;
        pointsFile.close();
        
        std::cout << "[Achievements] Loaded highScore from PointSystem: " << pointsPath 
                  << " (highScore=" << highScore << ")" << std::endl;
    } else {
        std::cout << "[Achievements] PointSystem file not found: " << pointsPath << std::endl;
        highScore = 0;
    }
    
    // Update achievement data with current player stats
    data.totalPoints = totalPoints;
    data.highScore = highScore;
    
    std::cout << "[Achievements] Updated stats for: " << data.username 
              << " (totalPoints=" << data.totalPoints << ", highScore=" << data.highScore << ")" << std::endl;
}

// ========================================
// IMPLEMENTATION: Save Achievements to File
// ========================================
void saveAchievements(const AchievementData &data) {
    // Create data directory if it doesn't exist (portable way)
    try {
        std::filesystem::create_directories("data");
    } catch (const std::exception& e) {
        std::cout << "[Achievements] WARNING: Could not create data directory: " << e.what() << std::endl;
    }

    // Construct file path: data/<username>_ach.txt
    std::string filePath = "data/" + data.username + "_ach.txt";

    std::cout << "[Achievements] DEBUG: Saving to: " << filePath << std::endl;

    // Open file for writing (overwrites existing)
    std::ofstream file(filePath.c_str());

    // Check if file is open
    if (!file.is_open()) {
        std::cout << "[Achievements] ERROR: Could not open file for writing: " << filePath << std::endl;
        return;
    }

    // Write username
    file << data.username << std::endl;

    // Write counters
    file << data.totalPoints << std::endl;
    file << data.allTimeFreezeUses << std::endl;
    file << data.highScore << std::endl;

    // Write unlock status (space-separated 0s and 1s)
    for (int i = 0; i < ACH_TOTAL; i++) {
        file << (data.unlocked[i] ? 1 : 0);
        if (i < ACH_TOTAL - 1) {
            file << " ";
        }
    }
    file << std::endl;

    // Close file
    file.close();

    std::cout << "[Achievements] Saved achievements for: " << data.username << " to " << filePath << std::endl;
}

// ========================================
// IMPLEMENTATION: Update and Check Achievements
// ========================================
int updateAndCheckAchievements(
    AchievementData &data,
    int pointsThisMatch,
    int freezeUsedThisMatch,
    int powerUpsUsedThisMatch
) {
    int newlyUnlockedCount = 0;

    // Update counters
    data.totalPoints += pointsThisMatch;
    data.allTimeFreezeUses += freezeUsedThisMatch;

    // Update high score if current match score is higher
    if (pointsThisMatch > data.highScore) {
        data.highScore = pointsThisMatch;
    }

    // ====== ACHIEVEMENT 1: RISING STAR ======
    // Condition: totalPoints >= 500
    if (!data.unlocked[ACH_RISING_STAR] && data.totalPoints >= 500) {
        data.unlocked[ACH_RISING_STAR] = true;
        newlyUnlockedCount++;
        std::cout << "[Achievement] Unlocked: " << getAchievementName(ACH_RISING_STAR) << std::endl;
    }

    // ====== ACHIEVEMENT 2: FREEZE MASTER ======
    // Condition: allTimeFreezeUses >= 10
    if (!data.unlocked[ACH_FREEZE_MASTER] && data.allTimeFreezeUses >= 10) {
        data.unlocked[ACH_FREEZE_MASTER] = true;
        newlyUnlockedCount++;
        std::cout << "[Achievement] Unlocked: " << getAchievementName(ACH_FREEZE_MASTER) << std::endl;
    }

    // ====== ACHIEVEMENT 3: SCORE TITAN ======
    // Condition: pointsThisMatch >= 2000 OR highScore >= 2000
    if (!data.unlocked[ACH_SCORE_TITAN] && (pointsThisMatch >= 2000 || data.highScore >= 2000)) {
        data.unlocked[ACH_SCORE_TITAN] = true;
        newlyUnlockedCount++;
        std::cout << "[Achievement] Unlocked: " << getAchievementName(ACH_SCORE_TITAN) << std::endl;
    }

    // ====== ACHIEVEMENT 4: ALL-TIME SCORER ======
    // Condition: totalPoints >= 5000
    if (!data.unlocked[ACH_ALL_TIME_SCORER] && data.totalPoints >= 5000) {
        data.unlocked[ACH_ALL_TIME_SCORER] = true;
        newlyUnlockedCount++;
        std::cout << "[Achievement] Unlocked: " << getAchievementName(ACH_ALL_TIME_SCORER) << std::endl;
    }

    // Save achievements immediately after updating (always save to persist counters)
    saveAchievements(data);

    return newlyUnlockedCount;
}

// ========================================
// IMPLEMENTATION: Print Unlocked Achievements
// ========================================
void printUnlockedAchievements(const AchievementData &data) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "ACHIEVEMENTS FOR: " << data.username << std::endl;
    std::cout << "========================================" << std::endl;

    int unlockedCount = 0;

    for (int i = 0; i < ACH_TOTAL; i++) {
        if (data.unlocked[i]) {
            std::cout << "[UNLOCKED] " << getAchievementName((AchievementId)i) << std::endl;
            std::cout << "           " << getAchievementDescription((AchievementId)i) << std::endl;
            unlockedCount++;
        } else {
            std::cout << "[LOCKED]   " << getAchievementName((AchievementId)i) << std::endl;
            std::cout << "           " << getAchievementDescription((AchievementId)i) << std::endl;
        }
    }

    std::cout << "\nProgress: " << unlockedCount << "/" << ACH_TOTAL << " achievements unlocked" << std::endl;
    std::cout << "========================================\n" << std::endl;
}
