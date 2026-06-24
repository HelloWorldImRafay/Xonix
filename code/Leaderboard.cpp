
                                // =================================
                                // |    24i-2008 Abdul Raffay      |
                                // | 24i-2130  Muhammad Hamza Adil |
                                // |       " XONIX GAME "          |
                                // |     LEADERBOARD CPP FILE      |
                                // =================================


#include "Leaderboard.h"
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;


// CONSTRUCTOR FOR LEADERBOARD
// Heap size ko 0 set krta hai
        Leaderboard::Leaderboard() : heapSize(0) {}

// GET PARENT, LEFT CHILD, RIGHT CHILD, SWAP, HEAPIFY UP/DOWN, FIND PLAYER INDEX FUNCTIONS 
// Ye sab Min-Heap ki helper functions hain

// Humy parent index, left child index, right child index return krti hain
        int Leaderboard::getParent(int index) const {
            return (index - 1) / 2;
        }

        int Leaderboard::getLeftChild(int index) const {
            return 2 * index + 1;
        }

        int Leaderboard::getRightChild(int index) const {
            return 2 * index + 2;
        }

// Humy simple do elements ko swap krdeta hai
        void Leaderboard::swap(int i, int j) {
            LeaderboardEntry temp = heap[i];
            heap[i] = heap[j];
            heap[j] = temp;
        }

// Humy heapify up krta hai jab naya element add hota hai ya score update hota hai
        void Leaderboard::heapifyUp(int index) {
            while (index > 0 && heap[index].totalScore < heap[getParent(index)].totalScore) {
                swap(index, getParent(index));
                index = getParent(index);
            }
        }
// Humy heapify down krta hai jab root element replace hota hai ya score update hota hai
        void Leaderboard::heapifyDown(int index) {
            int smallest = index;
            int left = getLeftChild(index);
            int right = getRightChild(index);
            
            if (left < heapSize && heap[left].totalScore < heap[smallest].totalScore) {
                smallest = left;
            }
            
            if (right < heapSize && heap[right].totalScore < heap[smallest].totalScore) {
                smallest = right;
            }
            
            if (smallest != index) {
                swap(index, smallest);
                heapifyDown(smallest);
            }
        }

// Humy player ka index find krta hai heap mai username ke basis par
        int Leaderboard::findPlayerIndex(const string& username) const {
            for (int i = 0; i < heapSize; i++) {
                if (heap[i].username == username) {
                    return i;
                }
            }
            return -1;
        }


// ADD PLAYER FUNCTION
// Naya player add krta hai leaderboard mai agar wo top 10 mai aa jata hai
        void Leaderboard::addPlayer(const string& username, int totalScore) {
            if (heapSize < MAX_HEAP_SIZE) {
                heap[heapSize] = LeaderboardEntry(username, totalScore);
                heapifyUp(heapSize);
                heapSize++;
            }
            else if (totalScore > heap[0].totalScore) {
                heap[0] = LeaderboardEntry(username, totalScore);
                heapifyDown(0);
            }
        }


// UPDATE PLAYER SCORE FUNCTION
// Player ka score update krta hai agar wo already leaderboard mai hai
        void Leaderboard::updatePlayerScore(const string& username, int totalScore) {
            int index = findPlayerIndex(username);
            
            if (index != -1) {
                int oldScore = heap[index].totalScore;
                heap[index].totalScore = totalScore;
                
                if (totalScore < oldScore) {
                    heapifyUp(index);
                } else if (totalScore > oldScore) {
                    heapifyDown(index);
                }
            } else {
                addPlayer(username, totalScore);
            }
        }

// GET ENTRY FUNCTION
// Specific index par leaderboard entry return krta hai
        LeaderboardEntry Leaderboard::getEntry(int index) const {
            if (index >= 0 && index < heapSize) {
                return heap[index];
            }
            return LeaderboardEntry("", 0);
        }

// GET PLAYER COUNT AND SCORE FUNCTIONS
// Total players count krta hai or specific player ka score return krta hai
        int Leaderboard::getPlayerCount() const {
            return heapSize;
        }

        int Leaderboard::getPlayerScore(const string& username) const {
            int index = findPlayerIndex(username);
            if (index != -1) {
                return heap[index].totalScore;
            }
            return 0;
        }


// GET TOP PLAYERS FUNCTION
// Top 10 players ko sorted order mai return krta hai
        void Leaderboard::getTopPlayers(LeaderboardEntry* topPlayers, int& count, int topCount) const {
            LeaderboardEntry sorted[MAX_HEAP_SIZE];
            for (int i = 0; i < heapSize; i++) {
                sorted[i] = heap[i];
            }
            
            for (int i = 0; i < heapSize - 1; i++) {
                for (int j = 0; j < heapSize - i - 1; j++) {
                    if (sorted[j].totalScore < sorted[j + 1].totalScore) {
                        // Swap
                        LeaderboardEntry temp = sorted[j];
                        sorted[j] = sorted[j + 1];
                        sorted[j + 1] = temp;
                    }
                }
            }
            
            count = (topCount < heapSize) ? topCount : heapSize;
            for (int i = 0; i < count; i++) {
                topPlayers[i] = sorted[i];
            }
        }

// DISPLAY TOP PLAYERS FUNCTION
// Top players ko console par display krta hai
        void Leaderboard::displayTopPlayers(int topCount) const {
            cout << "\n";
            cout << "================================ TOP " << topCount << " LEADERBOARD ================================\n";
            cout << setw(5) << "Rank" << setw(25) << "Username" << setw(15) << "Score\n";
            cout << "==============================================================================\n";
            
            LeaderboardEntry topPlayers[MAX_HEAP_SIZE];
            int count = 0;
            getTopPlayers(topPlayers, count, topCount);
            
            for (int i = 0; i < count; i++) {
                cout << setw(5) << (i + 1)
                    << setw(25) << topPlayers[i].username
                    << setw(15) << topPlayers[i].totalScore << "\n";
            }
            
            cout << "==============================================================================\n";
        }


// SAVE TO FILE FUNCTION
// Leaderboard ko file mai save krta hai "Leaderboard.txt" mai
        void Leaderboard::saveToFile(const string& filename) const {
            ofstream file(filename);
            
            if (!file.is_open()) {
                cerr << "Error: Could not save leaderboard to " << filename << endl;
                return;
            }
            
            for (int i = 0; i < heapSize; i++) {
                file << heap[i].username << "|" << heap[i].totalScore << "\n";
            }
            
            file.flush(); 
            file.close();
            
            ofstream verifyLog("files_saved.log", ios::app);
            verifyLog << filename << " (entries=" << heapSize << ")" << endl;
            verifyLog.flush();
            verifyLog.close();
            
            cerr << "DEBUG: Saved leaderboard to " << filename << " with " << heapSize << " entries" << endl;
        }

// LOAD FROM FILE FUNCTION
// Leaderboard ko file se load krta hai "Leaderboard.txt" se
        void Leaderboard::loadFromFile(const string& filename) {
            ifstream file(filename);
            
            if (!file.is_open()) {
                return;
            }
            
            string line;
            heapSize = 0;
            
            LeaderboardEntry tempEntries[MAX_HEAP_SIZE * 2]; 
            int tempCount = 0;
            
            while (getline(file, line) && tempCount < MAX_HEAP_SIZE * 2) {
                size_t pos = line.find('|');
                
                if (pos != string::npos) {
                    string username = line.substr(0, pos);
                    int score = stoi(line.substr(pos + 1));
                    
                    tempEntries[tempCount] = LeaderboardEntry(username, score);
                    tempCount++;
                }
            }
            
            file.close();
            
            for (int i = 0; i < tempCount; i++) {
                addPlayer(tempEntries[i].username, tempEntries[i].totalScore);
            }
        }

