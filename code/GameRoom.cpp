
                                   
                                   // =================================
                                    // |    24i-2008 Abdul Raffay      |
                                    // | 24i-2130  Muhammad Hamza Adil |
                                    // |       " XONIX GAME "          |
                                    // |       GAME ROOM CPP FILE      |
                                    // =================================


#include "GameRoom.h"
#include <iostream>
using namespace std;



// MAX WAIT TIME CONSTANT
const int GameRoom::MAX_WAIT_TIME = 15;  


// CONSTRUCTOR AND DESTRUCTOR FOR GAMEROOM
        GameRoom::GameRoom()
            : queueHead(nullptr), queueSize(0), hasActiveMatch(false), globalTimer(0.0f) {
        }

        GameRoom::~GameRoom() {
            deleteQueue(queueHead);
        }

// INSERT IN ORDER FUNCTION
// Linked list mai player ko uski priority k hisab se insert krta hai
        void GameRoom::insertInOrder(QueueNode*& head, const QueuedPlayer& player) {
            QueueNode* newNode = new QueueNode(player);
            
            if (head == nullptr || player < head->player) {
                newNode->next = head;
                head = newNode;
                return;
            }
            
            QueueNode* current = head;
            while (current->next != nullptr && current->next->player < player) {
                current = current->next;
            }
            
            newNode->next = current->next;
            current->next = newNode;
        }


// DELETE QUEUE FUNCTION
// Linked list queue ko delete krta hai 
        void GameRoom::deleteQueue(QueueNode*& head) {
            while (head != nullptr) {
                QueueNode* temp = head;
                head = head->next;
                delete temp;
            }
        }


// ADD PLAYER TO QUEUE FUNCTION
// Naye player ko matchmaking queue mai add krta hai
        void GameRoom::addPlayerToQueue(const string& username, int leaderboardRank, int leaderboardScore, float currentTime) {
            QueuedPlayer newPlayer(username, leaderboardRank, leaderboardScore, currentTime);
            
            // Insert into ordered linked list
            insertInOrder(queueHead, newPlayer);
            queueSize++;
            
            cout << "[GameRoom] Player " << username << " added to queue (Rank: " << leaderboardRank 
                      << ", Score: " << leaderboardScore << "). Queue size: " << queueSize << endl;
        }

// REMOVE PLAYER FROM QUEUE FUNCTION
// Specified player ko matchmaking queue se remove krta hai
        void GameRoom::removePlayerFromQueue(const string& username) {
            if (queueHead == nullptr) {
                return;
            }
            
            if (queueHead->player.username == username) {
                QueueNode* temp = queueHead;
                queueHead = queueHead->next;
                delete temp;
                queueSize--;
            } else {
                QueueNode* current = queueHead;
                while (current->next != nullptr) {
                    if (current->next->player.username == username) {
                        QueueNode* temp = current->next;
                        current->next = temp->next;
                        delete temp;
                        queueSize--;
                        break;
                    }
                    current = current->next;
                }
            }
            
            cout << "[GameRoom] Player " << username << " removed from queue. New queue size: " << queueSize << endl;
        }


// UPDATE MATCHMAKING FUNCTION
// Matchmaking process ko update krta hai har frame mai
        void GameRoom::updateMatchmaking(float deltaTime) {
            globalTimer += deltaTime;
            
            if (!hasActiveMatch && queueSize >= 2) {
                tryCreateMatch();
            }
        }


// TRY CREATE MATCH FUNCTION
// Queue se do players ko nikal kar match create krta hai
        bool GameRoom::tryCreateMatch() {
            if (queueSize < 2 || queueHead == nullptr) {
                return false;
            }
            
            QueuedPlayer player1 = queueHead->player;
            QueueNode* temp1 = queueHead;
            queueHead = queueHead->next;
            delete temp1;
            queueSize--;
            
            QueuedPlayer player2 = queueHead->player;
            QueueNode* temp2 = queueHead;
            queueHead = queueHead->next;
            delete temp2;
            queueSize--;
            
            currentMatch = MatchPair(player1, player2, globalTimer);
            hasActiveMatch = true;
            
            cout << "[GameRoom] MATCH CREATED!\n";
            cout << "  Player 1: " << player1.username << " (Rank: " << player1.leaderboardRank 
                      << ", Score: " << player1.leaderboardScore << ")\n";
            cout << "  Player 2: " << player2.username << " (Rank: " << player2.leaderboardRank 
                      << ", Score: " << player2.leaderboardScore << ")\n";
            cout << "  Queue size after match: " << queueSize << endl;
            
            return true;
        }


// GET ACTIVE MATCH FUNCTION
// Current active match ko return krta hai
        MatchPair GameRoom::getActiveMatch() const {
            return currentMatch;
        }


// HAS PLAYER IN QUEUE FUNCTION
// Check krta hai ke specified player queue mai hai ya nahi
        bool GameRoom::hasPlayerInQueue(const string& username) const {
            QueueNode* current = queueHead;
            while (current != nullptr) {
                if (current->player.username == username) {
                    return true;
                }
                current = current->next;
            }
            return false;
        }


// GET QUEUE SIZE FUNCTION
// Current queue size return krta hai
        int GameRoom::getQueueSize() const {
            return queueSize;
        }

// CLEAR ACTIVE MATCH FUNCTION
// Active match ko clear krta hai
        void GameRoom::clearActiveMatch() {
            hasActiveMatch = false;
            currentMatch = MatchPair();
            cout << "[GameRoom] Active match cleared" << endl;
        }


// RESET QUEUE FUNCTION 
// Poore matchmaking queue ko reset krta hai
        void GameRoom::resetQueue() {
            deleteQueue(queueHead);
            queueSize = 0;
            hasActiveMatch = false;
            globalTimer = 0.0f;
            cout << "[GameRoom] Queue reset" << endl;
        }
