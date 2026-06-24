
                                    // =================================
                                    // |    24i-2008 Abdul Raffay      |
                                    // | 24i-2130  Muhammad Hamza Adil |
                                    // |       " XONIX GAME "          |
                                    // |    LEADERBOARD HEADER FILE    |
                                    // =================================


#ifndef LEADERBOARD_H
#define LEADERBOARD_H
#include <string>
using namespace std;


// STRUCT FOR LEADERBOARD ENTRY
// Player ka username or total score ko Leaderboard mai store krta hai
    struct LeaderboardEntry {
        string username;
        int totalScore;
        
        LeaderboardEntry(const string& name = "", int score = 0)
            : username(name), totalScore(score) {}
        
        bool operator<(const LeaderboardEntry& other) const {
            return totalScore < other.totalScore;
        }
        
        bool operator>(const LeaderboardEntry& other) const {
            return totalScore > other.totalScore;
        }
    };

// CLASS FOR LEADERBOARD MANAGEMENT
// Min-Heap use krta hai top 10 players ko maintain krne k liye
    class Leaderboard {
    private:
        static const int MAX_HEAP_SIZE = 10; 
        LeaderboardEntry heap[MAX_HEAP_SIZE];
        int heapSize;
        int getParent(int index) const;
        int getLeftChild(int index) const;
        int getRightChild(int index) const;
        void heapifyUp(int index);
        void heapifyDown(int index);
        void swap(int i, int j);
        int findPlayerIndex(const string& username) const;
        
    public:
        Leaderboard();
        void updatePlayerScore(const string& username, int totalScore);
        void addPlayer(const string& username, int totalScore);
        void displayTopPlayers(int topCount = 10) const;
        LeaderboardEntry getEntry(int index) const;
        void getTopPlayers(LeaderboardEntry* topPlayers, int& count, int topCount = 10) const;
        int getPlayerCount() const;
        int getPlayerScore(const string& username) const;    
        void saveToFile(const string& filename = "leaderboard.txt") const;
        void loadFromFile(const string& filename = "leaderboard.txt");
    };

    #endif
