                                        // =================================
                                        // |    24i-2008 Abdul Raffay      |
                                        // | 24i-2130  Muhammad Hamza Adil |
                                        // |       " XONIX GAME "          |
                                        // |   FRIEND SYSTEM HEADER FILE   |
                                        // =================================

#ifndef FRIEND_SYSTEM_H
#define FRIEND_SYSTEM_H
#include <cstring>

// STRUCT FOR FRIEND NODE
// Linked list mai friend ka username store krta hai
    struct FriendNode {
            char username[50];
            FriendNode* next;
            
            FriendNode(const char* name = "") : next(nullptr) {
                if (name) {
                    strncpy(username, name, 49);
                    username[49] = '\0';
                } else {
                    username[0] = '\0';
                }
            }
        };

// STRUCT FOR FRIEND REQUEST NODE
// Linked list mai friend request ka data store krta hai
    struct FriendRequestNode {
            char fromUsername[50];
            char toUsername[50];
            FriendRequestNode* next;
            
            FriendRequestNode(const char* from = "", const char* to = "") : next(nullptr) {
                if (from) {
                    strncpy(fromUsername, from, 49);
                    fromUsername[49] = '\0';
                } else {
                    fromUsername[0] = '\0';
                }
                
                if (to) {
                    strncpy(toUsername, to, 49);
                    toUsername[49] = '\0';
                } else {
                    toUsername[0] = '\0';
                }
            }
        };
// STRUCT FOR PLAYER DATA
// Player ka data store krta hai jisme friends or requests ki linked lists hoti hain
    struct PlayerData {
            char username[50];
            int playerId;
            FriendNode* friendsHead;           // Linked list of friends ka head
            FriendRequestNode* requestsHead;   // Linked list of friend requests ka head
            
            PlayerData(const char* name = "", int id = 0) : playerId(id), friendsHead(nullptr), requestsHead(nullptr) {
                if (name) {
                    strncpy(username, name, 49);
                    username[49] = '\0';
                } else {
                    username[0] = '\0';
                }
            }
        };

// HASH TABLE CLASS
// Simple hash table for fast username to player index mapping
    class HashTable {
        private:
            static const int HASH_SIZE = 1000;
            int hashMap[HASH_SIZE]; // Isme player indices store hongi
            char usedUsernames[HASH_SIZE][50]; // Isme usernames store hongi 2D array mai
            int usedCount; // Kitne usernames use ho chuke hain
            
            int hashFunction(const char* username) const; 
            
        public:
            HashTable();
            void insert(const char* username, int playerIndex);
            int search(const char* username) const;
            void remove(const char* username);
        };

// FRIEND SYSTEM CLASS
// Friend system ko manage krta hai
    class FriendSystem {
        private:
            static const int MAX_PLAYERS = 500;
            
            PlayerData players[MAX_PLAYERS];
            int playerCount;
            HashTable playerHash;
            
            void deleteLinkedList(FriendNode*& head);
            void deleteRequestList(FriendRequestNode*& head);
            bool areFriends(const char* username1, const char* username2) const;
            bool hasRequest(const char* fromUser, const char* toUser) const;
            FriendRequestNode* findRequest(const char* fromUser, const char* toUser, FriendRequestNode*& prev);
            
        public:
            FriendSystem();
            ~FriendSystem();
            bool registerPlayer(const char* username, int playerId);
            int findPlayerIndex(const char* username) const;
            bool sendFriendRequest(const char* fromUsername, const char* toUsername);
            bool acceptFriendRequest(const char* fromUsername, const char* toUsername);
            bool rejectFriendRequest(const char* fromUsername, const char* toUsername);
            int getFriendCount(const char* username) const;
            bool getFriendsList(const char* username, char friendsList[][50], int& count) const;
            bool isFriend(const char* username1, const char* username2) const;
            int getPendingRequestCount(const char* username) const;
            bool getPendingRequests(const char* username, char requestsList[][50], int& count) const;
            void saveToFile(const char* filename = "friends_data.txt") const;
            void loadFromFile(const char* filename = "friends_data.txt");
            void loadAllPlayersFromFile(const char* filename = "players.txt");
            void displayPlayerInfo(const char* username) const;
        };

        #endif
