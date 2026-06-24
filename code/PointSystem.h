

                                        // =================================
                                        // |    24i-2008 Abdul Raffay      |
                                        // | 24i-2130  Muhammad Hamza Adil |
                                        // |       " XONIX GAME "          |
                                        // |   POINT SYSTEM HEADER FILE    |
                                        // =================================


#ifndef POINT_SYSTEM_H
#define POINT_SYSTEM_H

#include <string>


// CLASS FOR POINT SYSTEM
// Player ke score or power-ups ko manage krta hai

    class PointSystem{
    private:
        int score;
        int rewardCounter;
        int bonusThreshold;
        int bonusMultiplier;
        int powerUps;
        int nextPowerUpScore;
        int allTimeScore;
        int highScore;

        void checkPowerUp();

    public:

        PointSystem();
        int handleTileCapture(int tileCount);
        bool usePowerUp();
        void resetScore();
        void saveToFile(const std::string &username);
        void loadFromFile(const std::string &username);
        int getScore() const;
        int getPowerUps() const;
        int getRewardCounter() const;
        int getBonusThreshold() const;
        int getBonusMultiplier() const;
        int getAllTimeScore() const;
        int getHighScore() const;
    };

    #endif
