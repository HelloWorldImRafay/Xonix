                                    // =================================
                                    // |    24i-2008 Abdul Raffay      |
                                    // | 24i-2130  Muhammad Hamza Adil |
                                    // |       " XONIX GAME "          |
                                    // |     FRIEND SYSTEM CPP FILE    |
                                    // =================================


#include "FriendSystem.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
using namespace std;


// HASH TABLE IMPLEMENTATION
// Isme usernames ko hash krke player indices store krta hai
            int HashTable::hashFunction(const char* username) const {
                int hash = 0;
                for (int i = 0; username[i] != '\0' && i < 50; i++) {
                    hash = (hash * 31 + username[i]) % HASH_SIZE; // Formula hashing ka
                }
                return hash < 0 ? -hash : hash;
            }

// CONSTRUCTOR, INSERT, SEARCH, REMOVE FUNCTIONS FOR HASH TABLE
// Ye helper functions hain hash table k liye
            HashTable::HashTable() : usedCount(0) {
                for (int i = 0; i < HASH_SIZE; i++) {
                    hashMap[i] = -1;
                }
            }


// INSERT FUNCTION
// Username or player index ko hash table mai insert krta hai
            void HashTable::insert(const char* username, int playerIndex) {
                int hash = hashFunction(username);
                int searchIndex = hash;
                
                while (hashMap[searchIndex] != -1) {
                    if (strcmp(usedUsernames[searchIndex], username) == 0) {
                        hashMap[searchIndex] = playerIndex;
                        return;
                    }
                    searchIndex = (searchIndex + 1) % HASH_SIZE;
                }
                
                hashMap[searchIndex] = playerIndex;
                strncpy(usedUsernames[searchIndex], username, 49);
                usedUsernames[searchIndex][49] = '\0';
                usedCount++;
            }

// SEARCH FUNCTION
// Username k basis par player index return krta hai
            int HashTable::search(const char* username) const {
                int hash = hashFunction(username);
                int searchIndex = hash;
                
                while (hashMap[searchIndex] != -1) {
                    if (strcmp(usedUsernames[searchIndex], username) == 0) {
                        return hashMap[searchIndex];
                    }
                    searchIndex = (searchIndex + 1) % HASH_SIZE;
                }
                
                return -1; 
            }

// REMOVE FUNCTION
// Username ko hash table se remove krta hai
            void HashTable::remove(const char* username) {
                int hash = hashFunction(username);
                int searchIndex = hash;
                
                while (hashMap[searchIndex] != -1) {
                    if (strcmp(usedUsernames[searchIndex], username) == 0) {
                        hashMap[searchIndex] = -1;
                        usedUsernames[searchIndex][0] = '\0';
                        return;
                    }
                    searchIndex = (searchIndex + 1) % HASH_SIZE;
                }
            }


// FRIEND SYSTEM CONSTRUCTOR AND DESTRUCTOR
// Friend system ko initialize krta hai or memory free krta hai destructor mai
            FriendSystem::FriendSystem() : playerCount(0) {}

            FriendSystem::~FriendSystem() {
                for (int i = 0; i < playerCount; i++) {
                    deleteLinkedList(players[i].friendsHead);
                    deleteRequestList(players[i].requestsHead);
                }
            }

// Delete linked list k manual functions
// Friends or requests ki linked lists ko delete krta hai
            void FriendSystem::deleteLinkedList(FriendNode*& head) {
                while (head != nullptr) {
                    FriendNode* temp = head;
                    head = head->next;
                    delete temp;
                }
            }

            void FriendSystem::deleteRequestList(FriendRequestNode*& head) {
                while (head != nullptr) {
                    FriendRequestNode* temp = head;
                    head = head->next;
                    delete temp;
                }
            }

// REGISTER PLAYER FUNCTION
// Naya player register krta hai agar username available ho
            bool FriendSystem::registerPlayer(const char* username, int playerId) {
                if (playerCount >= MAX_PLAYERS) {
                    cerr << "Player limit reached!\n";
                    return false;
                }
                
                if (playerHash.search(username) != -1) {
                    cerr << "Username already exists!\n";
                    return false;
                }
                
                players[playerCount] = PlayerData(username, playerId);
                playerHash.insert(username, playerCount);
                playerCount++;
                
                return true;
            }

// FIND PLAYER INDEX FUNCTION
// Username k basis par player index return krta hai
            int FriendSystem::findPlayerIndex(const char* username) const {
                return playerHash.search(username);
            }

// SEND FRIEND REQUEST FUNCTION
// Ek player dusre player ko friend request bhej sakta hai
            bool FriendSystem::sendFriendRequest(const char* fromUsername, const char* toUsername) {
                int fromIdx = findPlayerIndex(fromUsername);
                int toIdx = findPlayerIndex(toUsername);
                
                // CRITICAL FIX: If recipient not registered, register them automatically
                if (toIdx == -1) {
                    registerPlayer(toUsername, 0);  // Register with default ID
                    toIdx = findPlayerIndex(toUsername);
                }
                
                if (fromIdx == -1) {
                    cerr << "Sender not found!\n";
                    return false;
                }
                
                if (strcmp(fromUsername, toUsername) == 0) {
                    cerr << "Cannot send request to yourself!\n";
                    return false;
                }
                
                if (areFriends(fromUsername, toUsername)) {
                    cerr << "Already friends!\n";
                    return false;
                }
                
                if (hasRequest(fromUsername, toUsername)) {
                    cerr << "Friend request already sent!\n";
                    return false;
                }
                
                FriendRequestNode* newRequest = new FriendRequestNode(fromUsername, toUsername);
                newRequest->next = players[toIdx].requestsHead;
                players[toIdx].requestsHead = newRequest;
                
                cout << "Friend request sent from " << fromUsername << " to " << toUsername << "!\n";
                
                // CRITICAL FIX: Save immediately to persist the request
                saveToFile("friends_data.txt");
                
                return true;
            }

// ACCEPT FRIEND REQUEST FUNCTION
// Ek player ke friend request ko accept krta hai
            bool FriendSystem::acceptFriendRequest(const char* fromUsername, const char* toUsername) {
                int fromIdx = findPlayerIndex(fromUsername);
                int toIdx = findPlayerIndex(toUsername);
                
                if (fromIdx == -1 || toIdx == -1) {
                    cerr << "One or both users not found!\n";
                    return false;
                }
                
                FriendRequestNode* prev = nullptr;
                FriendRequestNode* current = players[toIdx].requestsHead;
                bool found = false;
                
                while (current != nullptr) {
                    if (strcmp(current->fromUsername, fromUsername) == 0 && 
                        strcmp(current->toUsername, toUsername) == 0) {
                        found = true;
                        break;
                    }
                    prev = current;
                    current = current->next;
                }
                
                if (!found) {
                    cerr << "Friend request not found!\n";
                    return false;
                }
                
                if (prev == nullptr) {
                    players[toIdx].requestsHead = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                
                FriendNode* newFriend1 = new FriendNode(fromUsername);
                newFriend1->next = players[toIdx].friendsHead;
                players[toIdx].friendsHead = newFriend1;
                
                FriendNode* newFriend2 = new FriendNode(toUsername);
                newFriend2->next = players[fromIdx].friendsHead;
                players[fromIdx].friendsHead = newFriend2;
                
                cout << fromUsername << " and " << toUsername << " are now friends!\n";
                
                // CRITICAL FIX: Save immediately to persist the friendship
                saveToFile("friends_data.txt");
                
                return true;
            }

// REJECT FRIEND REQUEST FUNCTION
// Ek player ke friend request ko reject krskta hai
            bool FriendSystem::rejectFriendRequest(const char* fromUsername, const char* toUsername) {
                int toIdx = findPlayerIndex(toUsername);
                
                if (toIdx == -1) {
                    cerr << "User not found!\n";
                    return false;
                }
                
                FriendRequestNode* prev = nullptr;
                FriendRequestNode* current = players[toIdx].requestsHead;
                
                while (current != nullptr) {
                    if (strcmp(current->fromUsername, fromUsername) == 0 && 
                        strcmp(current->toUsername, toUsername) == 0) {
                        break;
                    }
                    prev = current;
                    current = current->next;
                }
                
                if (current == nullptr) {
                    cerr << "Friend request not found!\n";
                    return false;
                }
                
                if (prev == nullptr) {
                    players[toIdx].requestsHead = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                
                cout << "Friend request rejected!\n";
                
                // CRITICAL FIX: Save immediately to persist the rejection
                saveToFile("friends_data.txt");
                
                return true;
            }


// ARE FRIENDS FUNCTION
// Edge Case Jo check krta hai ke do players friends hain ya nahi
            bool FriendSystem::areFriends(const char* username1, const char* username2) const {
                int idx1 = findPlayerIndex(username1);
                
                if (idx1 == -1) {
                    return false;
                }
                
                FriendNode* current = players[idx1].friendsHead;
                while (current != nullptr) {
                    if (strcmp(current->username, username2) == 0) {
                        return true;
                    }
                    current = current->next;
                }
                
                return false;
            }

// HAS REQUEST FUNCTION
// Check krta hai ke ek player ne dusre player ko friend request bheji hai ya nahi
            bool FriendSystem::hasRequest(const char* fromUser, const char* toUser) const {
                int toIdx = findPlayerIndex(toUser);
                
                if (toIdx == -1) {
                    return false;
                }
                
                FriendRequestNode* current = players[toIdx].requestsHead;
                while (current != nullptr) {
                    if (strcmp(current->fromUsername, fromUser) == 0) {
                        return true;
                    }
                    current = current->next;
                }
                
                return false;
            }

// GET FRIEND COUNT FUNCTION
// Ek player ke friends ki count return krta hai
            int FriendSystem::getFriendCount(const char* username) const {
                int idx = findPlayerIndex(username);
                
                if (idx == -1) {
                    return 0;
                }
                
                int count = 0;
                FriendNode* current = players[idx].friendsHead;
                while (current != nullptr) {
                    count++;
                    current = current->next;
                }
                
                return count;
            }

// GET FRIENDS LIST FUNCTION
// Ek player ke friends ki list return krta hai
            bool FriendSystem::getFriendsList(const char* username, char friendsList[][50], int& count) const {
                int idx = findPlayerIndex(username);
                
                if (idx == -1) {
                    count = 0;
                    return false;
                }
                
                count = 0;
                FriendNode* current = players[idx].friendsHead;
                while (current != nullptr && count < 500) {
                    strncpy(friendsList[count], current->username, 49);
                    friendsList[count][49] = '\0';
                    count++;
                    current = current->next;
                }
                
                return true;
            }
// IS FRIEND FUNCTION
// Edge Case jo check krta hai ke do players friends hain ya nahi
            bool FriendSystem::isFriend(const char* username1, const char* username2) const {
                return areFriends(username1, username2);
            }

// GET PENDING REQUEST COUNT FUNCTION
// Ek player ke pending friend requests ki count return krta hai
            int FriendSystem::getPendingRequestCount(const char* username) const {
                int idx = findPlayerIndex(username);
                
                if (idx == -1) {
                    return 0;
                }
                
                int count = 0;
                FriendRequestNode* current = players[idx].requestsHead;
                while (current != nullptr) {
                    count++;
                    current = current->next;
                }
                
                return count;
            }
// GET PENDING REQUESTS FUNCTION
// Ek player ke pending friend requests ki list return krta hai
            bool FriendSystem::getPendingRequests(const char* username, char requestsList[][50], int& count) const {
                int idx = findPlayerIndex(username);
                
                if (idx == -1) {
                    count = 0;
                    return false;
                }
                
                count = 0;
                FriendRequestNode* current = players[idx].requestsHead;
                while (current != nullptr && count < 500) {
                    strncpy(requestsList[count], current->fromUsername, 49);
                    requestsList[count][49] = '\0';
                    count++;
                    current = current->next;
                }
                
                return true;
            }

// SAVE TO FILE FUNCTION
// Friend system data ko file mai save krta hai "friends_data.txt" mai
            void FriendSystem::saveToFile(const char* filename) const {
                ofstream file(filename);
                
                if (!file.is_open()) {
                    cerr << "Error: Could not save friend data to " << filename << endl;
                    return;
                }
                
                file << playerCount << "\n";
                
                for (int i = 0; i < playerCount; i++) {
                    file << players[i].username << "|" << players[i].playerId << "|";
                    
                    FriendNode* friendCurrent = players[i].friendsHead;
                    int friendCount = 0;
                    FriendNode* temp = friendCurrent;
                    while (temp != nullptr) {
                        friendCount++;
                        temp = temp->next;
                    }
                    
                    file << friendCount;
                    friendCurrent = players[i].friendsHead;
                    while (friendCurrent != nullptr) {
                        file << ":" << friendCurrent->username;
                        friendCurrent = friendCurrent->next;
                    }
                    
                    file << "|";
                    FriendRequestNode* reqCurrent = players[i].requestsHead;
                    int requestCount = 0;
                    FriendRequestNode* reqTemp = reqCurrent;
                    while (reqTemp != nullptr) {
                        requestCount++;
                        reqTemp = reqTemp->next;
                    }
                    
                    file << requestCount;
                    reqCurrent = players[i].requestsHead;
                    while (reqCurrent != nullptr) {
                        file << ":" << reqCurrent->fromUsername;
                        reqCurrent = reqCurrent->next;
                    }
                    
                    file << "\n";
                }
                
                file.flush();
                file.close();
                
                ofstream verifyLog("files_saved.log", ios::app);
                verifyLog << filename << " (players=" << playerCount << ")" << endl;
                verifyLog.flush();
                verifyLog.close();
                
                cerr << "DEBUG: Saved friend data to " << filename << " with " << playerCount << " players" << endl;
            }

// LOAD FROM FILE FUNCTION
// Friend system data ko file se load krta hai "friends_data.txt" se
            void FriendSystem::loadFromFile(const char* filename) {
                ifstream file(filename);
                
                if (!file.is_open()) {
                    return;
                }
                
                int count;
                file >> count;
                file.ignore();
                
                for (int i = 0; i < count && playerCount < MAX_PLAYERS; i++) {
                    char line[1000];
                    file.getline(line, 1000);
                    
                    // CRITICAL FIX: Use proper string parsing with clear sections
                    char* copy = new char[strlen(line) + 1];
                    strcpy(copy, line);
                    
                    // Split by | delimiter
                    char* sections[4] = {nullptr, nullptr, nullptr, nullptr};
                    int sectionCount = 0;
                    char* sect = strtok(copy, "|");
                    while (sect != nullptr && sectionCount < 4) {
                        sections[sectionCount++] = sect;
                        sect = strtok(nullptr, "|");
                    }
                    
                    if (sectionCount < 4) {
                        delete[] copy;
                        continue;
                    }
                    
                    // Section 0: username
                    char username[50];
                    strncpy(username, sections[0], 49);
                    username[49] = '\0';
                    
                    // Section 1: playerId
                    int playerId = atoi(sections[1]);
                    registerPlayer(username, playerId);
                    
                    // Section 2: friends (format: count:name1:name2:...)
                    int friendCount = atoi(sections[2]);
                    if (friendCount > 0) {
                        char* friendList = sections[2] + strlen(sections[2]) - 1;
                        while (*friendList && *friendList != ':') friendList--;
                        
                        if (*friendList == ':') {
                            char* friendPos = sections[2];
                            char* firstColon = strchr(friendPos, ':');
                            while (firstColon != nullptr && friendCount > 0) {
                                firstColon++;
                                char* nextColon = strchr(firstColon, ':');
                                char friendName[50];
                                
                                if (nextColon == nullptr) {
                                    // Last friend
                                    strncpy(friendName, firstColon, 49);
                                    friendName[49] = '\0';
                                    // Remove any trailing whitespace/newline
                                    int len = strlen(friendName);
                                    while (len > 0 && (friendName[len-1] == '\n' || friendName[len-1] == '\r' || friendName[len-1] == ' ')) {
                                        friendName[len-1] = '\0';
                                        len--;
                                    }
                                } else {
                                    strncpy(friendName, firstColon, nextColon - firstColon);
                                    friendName[nextColon - firstColon] = '\0';
                                }
                                
                                int playerIdx = findPlayerIndex(username);
                                if (playerIdx != -1) {
                                    FriendNode* newFriend = new FriendNode(friendName);
                                    newFriend->next = players[playerIdx].friendsHead;
                                    players[playerIdx].friendsHead = newFriend;
                                }
                                
                                firstColon = nextColon;
                                friendCount--;
                            }
                        }
                    }
                    
                    // Section 3: requests (format: count:requester1:requester2:...)
                    int requestCount = atoi(sections[3]);
                    if (requestCount > 0) {
                        char* reqPos = sections[3];
                        char* firstColon = strchr(reqPos, ':');
                        while (firstColon != nullptr && requestCount > 0) {
                            firstColon++;
                            char* nextColon = strchr(firstColon, ':');
                            char requesterName[50];
                            
                            if (nextColon == nullptr) {
                                // Last requester
                                strncpy(requesterName, firstColon, 49);
                                requesterName[49] = '\0';
                                // Remove any trailing whitespace/newline
                                int len = strlen(requesterName);
                                while (len > 0 && (requesterName[len-1] == '\n' || requesterName[len-1] == '\r' || requesterName[len-1] == ' ')) {
                                    requesterName[len-1] = '\0';
                                    len--;
                                }
                            } else {
                                strncpy(requesterName, firstColon, nextColon - firstColon);
                                requesterName[nextColon - firstColon] = '\0';
                            }
                            
                            int playerIdx = findPlayerIndex(username);
                            if (playerIdx != -1) {
                                FriendRequestNode* newRequest = new FriendRequestNode(requesterName, username);
                                newRequest->next = players[playerIdx].requestsHead;
                                players[playerIdx].requestsHead = newRequest;
                                
                                cerr << "DEBUG: Loaded request from " << requesterName << " to " << username << endl;
                            }
                            
                            firstColon = nextColon;
                            requestCount--;
                        }
                    }
                    
                    delete[] copy;
                }
                
                file.close();
                cerr << "DEBUG: loadFromFile completed. Total players: " << playerCount << endl;
            }

// LOAD ALL PLAYERS FROM FILE FUNCTION
// Sare players ko "players.txt" file se load krta hai
            void FriendSystem::loadAllPlayersFromFile(const char* filename) {
                ifstream file(filename);
                
                if (!file.is_open()) {
                    return;
                }
                
                string line;
                while (getline(file, line)) {
                    if (line.empty()) continue;
                    
                    size_t pos1 = line.find('|');
                    if (pos1 == string::npos) continue;
                    
                    size_t pos2 = line.find('|', pos1 + 1);
                    if (pos2 == string::npos) continue;
                    
                    string username = line.substr(pos1 + 1, pos2 - pos1 - 1);
                    
                    int id = atoi(line.substr(0, pos1).c_str());
                    
                    if (findPlayerIndex(username.c_str()) == -1) {
                        registerPlayer(username.c_str(), id);
                    }
                }
                
                file.close();
            }

// DISPLAY PLAYER INFO FUNCTION
// Ek player ka detailed info display krta hai

            void FriendSystem::displayPlayerInfo(const char* username) const {
                int idx = findPlayerIndex(username);
                
                if (idx == -1) {
                    cout << "Player not found!\n";
                    return;
                }
                
                cout << "\n===== PLAYER INFO =====\n";
                cout << "Username: " << players[idx].username << "\n";
                cout << "Player ID: " << players[idx].playerId << "\n";
                
                cout << "\nFriends (" << getFriendCount(username) << "):\n";
                FriendNode* current = players[idx].friendsHead;
                if (current == nullptr) {
                    cout << "  No friends yet\n";
                }
                while (current != nullptr) {
                    cout << "  - " << current->username << "\n";
                    current = current->next;
                }
                
                cout << "\nPending Requests (" << getPendingRequestCount(username) << "):\n";
                FriendRequestNode* reqCurrent = players[idx].requestsHead;
                if (reqCurrent == nullptr) {
                    cout << "  No pending requests\n";
                }
                while (reqCurrent != nullptr) {
                    cout << "  - From: " << reqCurrent->fromUsername << "\n";
                    reqCurrent = reqCurrent->next;
                }
                cout << "=====================\n\n";
            }
