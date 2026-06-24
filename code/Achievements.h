#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <filesystem>
using namespace std;

// ========================================
// ACHIEVEMENT ENUMERATION
// ========================================
enum AchievementId {
    ACH_RISING_STAR = 0,        // Reach 500 total points
    ACH_FREEZE_MASTER,          // Use Freeze 10 times total
    ACH_SCORE_TITAN,            // Hit 2000 score in a single run
    ACH_ALL_TIME_SCORER,        // Accumulate 5000 total points
    ACH_TOTAL                   // Total count (must be last)
};

// ========================================
// ACHIEVEMENT DATA STRUCTURE
// ========================================
struct AchievementData {
    string username;

    // Tracking counters
    int totalPoints;
    int allTimeFreezeUses;
    int highScore;

    // Unlock status for each achievement
    bool unlocked[ACH_TOTAL];
};

// ========================================
// FUNCTION DECLARATIONS
// ========================================

/**
 * Initialize achievement data with default values.
 * @param data Reference to AchievementData structure to initialize
 * @param username Player's username
 * @param resetUnlocks If true, initialize all achievements as locked (for new users).
 *                     If false, leave unlocked[] unchanged (for existing users on login).
 */
void initAchievementData(AchievementData &data, const string &username, bool resetUnlocks = true);

/**
 * Load achievements from file (data/<username>_ach.txt).
 * @param data Reference to AchievementData structure to populate
 * @return true if file loaded successfully, false if file doesn't exist
 */
bool loadAchievements(AchievementData &data);

/**
 * Load user's total points and high score from PointSystem data file.
 * Updates the AchievementData structure with current player stats.
 * @param data Reference to AchievementData structure to update
 */
void loadPlayerStatsForAchievements(AchievementData &data);

/**
 * Save achievements to file (data/<username>_ach.txt).
 * @param data Reference to AchievementData structure to save
 */
void saveAchievements(const AchievementData &data);

/**
 * Update achievement counters and check for newly unlocked achievements.
 * @param data Reference to AchievementData structure to update
 * @param pointsThisMatch Points earned in current match
 * @param freezeUsedThisMatch Number of times Freeze was used
 * @param powerUpsUsedThisMatch Total powerups used in current match
 * @return Number of newly unlocked achievements
 */
int updateAndCheckAchievements(
    AchievementData &data,
    int pointsThisMatch,
    int freezeUsedThisMatch,
    int powerUpsUsedThisMatch
);

/**
 * Print all unlocked achievements to console.
 * @param data Reference to AchievementData structure to print from
 */
void printUnlockedAchievements(const AchievementData &data);

// ========================================
// HELPER FUNCTIONS
// ========================================

/**
 * Get the name of an achievement by ID.
 * @param id Achievement ID
 * @return Achievement name as string
 */
string getAchievementName(AchievementId id);

/**
 * Get the description of an achievement by ID.
 * @param id Achievement ID
 * @return Achievement description as string
 */
string getAchievementDescription(AchievementId id);

#endif // ACHIEVEMENTS_H
