

                                    // =================================
                                    // |    24i-2008 Abdul Raffay      |
                                    // | 24i-2130  Muhammad Hamza Adil |
                                    // |       " XONIX GAME "          |
                                    // |     POINT SYSTEM CPP FILE     |
                                    // =================================



#include "PointSystem.h"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;


// CONSTRUCTOR FOR POINT SYSTEM
// Initial values set krta hai
    PointSystem::PointSystem()
    {
        score = 0;
        rewardCounter = 0;
        bonusThreshold = 10;
        bonusMultiplier = 2;

        powerUps = 0;
        nextPowerUpScore = 50;

        allTimeScore = 0;
        highScore = 0;
    }

// HANDLE TILE CAPTURE FUNCTION
// Tile capture krne par score calculate krta hai
    int PointSystem::handleTileCapture(int tileCount)
    {
        int pointsEarned = tileCount;

        if (tileCount > bonusThreshold)
        {
            pointsEarned *= bonusMultiplier;
            rewardCounter++;

            if (rewardCounter == 3)
            {
                bonusThreshold = 5;
                bonusMultiplier = 2;
            }
            else if (rewardCounter == 5)
            {
                bonusThreshold = 5;
                bonusMultiplier = 4;
            }
        }

        score += pointsEarned;
        allTimeScore += pointsEarned;
        if (score > highScore) highScore = score;

        checkPowerUp();

        return pointsEarned;
    }

// CHECK POWER-UP FUNCTION
// Check krta hai ke power-up dena hai ya nahi

    void PointSystem::checkPowerUp()
    {
        while (allTimeScore >= nextPowerUpScore)
        {
            powerUps++;

            if (nextPowerUpScore == 50)
                nextPowerUpScore = 70;
            else
                nextPowerUpScore += 30;
        }
    }

// USE POWER-UP FUNCTION
// Power-up use krta hai agar available ho
    bool PointSystem::usePowerUp()
    {
        if (powerUps > 0)
        {
            powerUps--;
            return true;
        }
        return false;
    }

// RESET SCORE FUNCTION
// Score ko reset krta hai
    void PointSystem::resetScore()
    {
        score = 0;
        bonusThreshold = 10;
        bonusMultiplier = 2;
        rewardCounter = 0;
    }

// SAVE TO FILE FUNCTION
// Player data ko file mai save krta hai
    void PointSystem::saveToFile(const string &username)
    {
        string filename = username + "_data.txt";
        ofstream fout(filename);
        
        if (!fout) {
            cerr << "ERROR: Cannot open file " << filename << " for writing!" << endl;
            return;
        }
        
        fout << allTimeScore << " " << highScore << " " << powerUps << " " << nextPowerUpScore << endl;
        fout.flush();
        fout.close();
        
        ofstream verifyLog("files_saved.log", ios::app);
        verifyLog << filename << " (score=" << allTimeScore << ")" << endl;
        verifyLog.flush();
        verifyLog.close();
        
        cerr << "DEBUG: Saved points to " << filename << " - allTime=" << allTimeScore 
                << " file_path_attempt=" << filename << endl;
    }

    void PointSystem::loadFromFile(const string &username)
    {
        string filename = username + "_data.txt";
        ifstream fin(filename);
        
        if (!fin) {
            cerr << "DEBUG: File not found: " << filename << " (first time player)" << endl;
            allTimeScore = 0;
            highScore = 0;
            powerUps = 0;
            nextPowerUpScore = 50;
            return;
        }
        
        fin >> allTimeScore >> highScore >> powerUps >> nextPowerUpScore;
        fin.close();
        
        cerr << "DEBUG: Loaded points from " << filename << " - allTime=" << allTimeScore 
                << " powerUps=" << powerUps << " nextPowerUpScore=" << nextPowerUpScore << endl;
    }

// GETTERS OF POINT SYSTEM
// private attributes ko access krne k liye
    int PointSystem::getScore() const { return score; }
    int PointSystem::getPowerUps() const { return powerUps; }
    int PointSystem::getRewardCounter() const { return rewardCounter; }
    int PointSystem::getBonusThreshold() const { return bonusThreshold; }
    int PointSystem::getBonusMultiplier() const { return bonusMultiplier; }
    int PointSystem::getAllTimeScore() const { return allTimeScore; }
    int PointSystem::getHighScore() const { return highScore; }
