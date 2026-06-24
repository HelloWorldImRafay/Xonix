

                                        // =================================
                                        // |    24i-2008 Abdul Raffay      |
                                        // | 24i-2130  Muhammad Hamza Adil |
                                        // |        " XONIX GAME "         |
                                        // |     GAME ROOM HEADER FILE     |
                                        // =================================


#ifndef GAMEROOM_H
#define GAMEROOM_H

#include <string>
#include <climits>
#include "Leaderboard.h"
using namespace std;


// STRUCT FOR QUEUED PLAYER
// Matchmaking queue mai player ka data store krta hai
    struct QueuedPlayer {
            string username;
            int leaderboardRank;  
            int leaderboardScore; 
            float timeJoined;     
            
            QueuedPlayer() 
                : username(""), leaderboardRank(INT_MAX), leaderboardScore(0), timeJoined(0.0f) {}
            
            QueuedPlayer(const string& user, int rank, int score, float joinTime)
                : username(user), leaderboardRank(rank), leaderboardScore(score), timeJoined(joinTime) {}
            
            bool operator<(const QueuedPlayer& other) const {
                if (leaderboardRank != other.leaderboardRank) {
                    return leaderboardRank > other.leaderboardRank;  
                }
                return timeJoined > other.timeJoined;
            }
        };


// STRUCT FOR MATCH PAIR
// Do players ka match pair store krta hai    
    struct MatchPair {
            QueuedPlayer player1;
            QueuedPlayer player2;
            float matchStartTime;
            
            MatchPair()
                : player1(), player2(), matchStartTime(0.0f) {}
            
            MatchPair(const QueuedPlayer& p1, const QueuedPlayer& p2, float startTime)
                : player1(p1), player2(p2), matchStartTime(startTime) {}
        };


// STRUCT FOR QUEUE NODE
// Queue ko Manually implement krne k liye linked list ka node       
    struct QueueNode {
            QueuedPlayer player;
            QueueNode* next;
            
            QueueNode(const QueuedPlayer& p)
                : player(p), next(nullptr) {}
        };

        
// CLASS FOR GAME ROOM
// Matchmaking queue or active match ko manage krta hai      
    class GameRoom {
        private:
            QueueNode* queueHead;                        
            int queueSize;                              
            MatchPair currentMatch;                      
            bool hasActiveMatch;
            
            static const int MAX_WAIT_TIME;            
            float globalTimer;                          
            void insertInOrder(QueueNode*& head, const QueuedPlayer& player);
            void deleteQueue(QueueNode*& head);
            
        public:
            GameRoom();
            ~GameRoom();
            void addPlayerToQueue(const string& username, int leaderboardRank, int leaderboardScore, float currentTime);
            void removePlayerFromQueue(const string& username);
            void updateMatchmaking(float deltaTime);
            bool tryCreateMatch();
            MatchPair getActiveMatch() const;
            bool hasPlayerInQueue(const string& username) const;
            int getQueueSize() const;
            bool isMatchActive() const { return hasActiveMatch; }
            void clearActiveMatch();
            void resetQueue();
        };

        #endif
