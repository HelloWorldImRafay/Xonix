                                        // =================================
                                        // |    24i-2008 Abdul Raffay      |
                                        // | 24i-2130  Muhammad Hamza Adil |
                                        // |       " XONIX GAME "          |
                                        // |   PLAYER PROFILE CPP FILE     |
                                        // =================================


#include "PlayerProfile.h"
#include <fstream>
#include <cstring>
#include <iostream>

// DEFUALT CONSTRUCTOR FOR PLAYER PROFILE
            PlayerProfile::PlayerProfile()
                : playerId(0), username(""), nickname(""), email(""), totalPoints(0), friendsCount(0), matchCount(0) {}

// PARAMETERIZED CONSTRUCTOR FOR PLAYER PROFILE
            PlayerProfile::PlayerProfile(int id, const string& name, const string& nick)
                : playerId(id), username(name), nickname(nick), email(""), totalPoints(0), friendsCount(0), matchCount(0) {}

// GET PLAYER ID FUNCTION
// Player ID return krta hai
            int PlayerProfile::getPlayerId() const {
                return playerId;
            }


// GET USERNAME FUNCTION
// Username return krta hai
            string PlayerProfile::getUsername() const {
                return username;
            }

// GET NICKNAME FUNCTION
// Nickname return krta hai
            string PlayerProfile::getNickname() const {
                return nickname;
            }


// GET EMAIL FUNCTION
// Email return krta hai
            string PlayerProfile::getEmail() const {
                return email;
            }


// GET TOTAL POINTS FUNCTION
// Total points return krta hai
            int PlayerProfile::getTotalPoints() const {
                return totalPoints;
            }


// GET FRIENDS COUNT FUNCTION
// Friends count return krta hai
            int PlayerProfile::getFriendsCount() const {
                return friendsCount;
            }

// GET FRIEND FUNCTION
// Specified index par friend ka username return krta hai
            string PlayerProfile::getFriend(int index) const {
                if (index >= 0 && index < friendsCount) {
                    return friendsList[index];
                }
                return "";
            }

// GET MATCH COUNT FUNCTION
// Match count return krta hai
            int PlayerProfile::getMatchCount() const {
                return matchCount;
            }

// GET MATCH FUNCTION
// Specified index par match record return krta hai
            MatchRecord PlayerProfile::getMatch(int index) const {
                if (index >= 0 && index < matchCount) {
                    return matchHistory[index];
                }
                return MatchRecord();
            }

// GET WINS FUNCTION
// Total wins return krta hai
            int PlayerProfile::getWins() const {
                int wins = 0;
                for (int i = 0; i < matchCount; i++) {
                    if (matchHistory[i].won) wins++;
                }
                return wins;
            }


// GET LOSSES FUNCTION
// Total losses return krta hai
            int PlayerProfile::getLosses() const {
                int losses = 0;
                for (int i = 0; i < matchCount; i++) {
                    if (!matchHistory[i].won) losses++;
                }
                return losses;
            }

// SETTERS FOR NICKNAME, EMAIL, TOTAL POINTS
// Nickname, email or total points set krta hai
            void PlayerProfile::setNickname(const string& nick) {
                nickname = nick;
            }

            void PlayerProfile::setEmail(const string& em) {
                email = em;
            }

            void PlayerProfile::setTotalPoints(int points) {
                totalPoints = (points >= 0) ? points : 0;
            }


// ADD POINTS FUNCTION
// Total points mai specified points add krta hai
            void PlayerProfile::addPoints(int points) {
                totalPoints += points;
                if (totalPoints < 0) totalPoints = 0;
            }

            void PlayerProfile::addFriend(const string& friendUsername) {
                if (friendsCount >= MAX_FRIENDS) return;
                
                for (int i = 0; i < friendsCount; i++) {
                    if (friendsList[i] == friendUsername) return; 
                }
                
                friendsList[friendsCount] = friendUsername;
                friendsCount++;
            }


// REMOVE FRIEND FUNCTION
// Specified friend ko friends list se remove krta hai
            void PlayerProfile::removeFriend(const string& friendUsername) {
                for (int i = 0; i < friendsCount; i++) {
                    if (friendsList[i] == friendUsername) {
                        for (int j = i; j < friendsCount - 1; j++) {
                            friendsList[j] = friendsList[j + 1];
                        }
                        friendsCount--;
                        return;
                    }
                }
            }

// IS FRIEND FUNCTION
// Check krta hai specified username friend list mein hai ya nahi
            bool PlayerProfile::isFriend(const string& friendUsername) const {
                for (int i = 0; i < friendsCount; i++) {
                    if (friendsList[i] == friendUsername) {
                        return true;
                    }
                }
                return false;
            }

// RECORD MATCH FUNCTION
// Naya match record add krta hai match history mein
            void PlayerProfile::recordMatch(const string& opponent, bool won, int pointsEarned) {
                if (matchCount >= MAX_MATCHES) {
                    // Shift older matches out, keep last 10
                    for (int i = 0; i < MAX_MATCHES - 1; i++) {
                        matchHistory[i] = matchHistory[i + 1];
                    }
                    matchCount = MAX_MATCHES - 1;
                }
                
                matchHistory[matchCount] = MatchRecord(opponent, won, pointsEarned);
                matchCount++;
                totalPoints += pointsEarned;
                if (totalPoints < 0) totalPoints = 0;
            }

// SAVE TO FILE FUNCTION
// Player profile ko file mein save krta hai
            void PlayerProfile::saveToFile() const {
                if (username.empty()) {
                    cerr << "ERROR: Cannot save profile - username is empty!" << endl;
                    return;
                }
                
                string filename = username + "_profile.txt";
                ofstream file(filename);
                
                if (!file.is_open()) {
                    cerr << "Error: Could not save profile for " << username << endl;
                    return;
                }
                
                file << playerId << "\n";
                file << username << "\n";
                file << nickname << "\n";
                file << email << "\n";
                file << totalPoints << "\n";
                
                file << friendsCount << "\n";
                for (int i = 0; i < friendsCount; i++) {
                    file << friendsList[i] << "\n";
                }
                
                file << matchCount << "\n";
                for (int i = 0; i < matchCount; i++) {
                    file << matchHistory[i].opponent << "|" << matchHistory[i].won << "|" << matchHistory[i].pointsEarned << "\n";
                }
                
                file.flush();
                file.close();
                
                ofstream verifyLog("files_saved.log", ios::app);
                verifyLog << filename << " (totalPoints=" << totalPoints << ")" << endl;
                verifyLog.flush();
                verifyLog.close();
                
                cerr << "DEBUG: Saved profile to " << filename << " - totalPoints=" << totalPoints << endl;
            }


// LOAD FROM FILE FUNCTION
// Player profile ko file se load krta hai 
            void PlayerProfile::loadFromFile() {
                string filename = username + "_profile.txt";
                ifstream file(filename);
                
                if (!file.is_open()) {
                    // File doesn't exist yet, profile will use default values
                    return;
                }
                
                file >> playerId;
                file.ignore();
                string loadedUsername;
                getline(file, loadedUsername);  // Read but don't overwrite username
                getline(file, nickname);
                getline(file, email);
                file >> totalPoints;
                
                int count = 0;
                file >> count;
                file.ignore();
                
                friendsCount = 0;
                for (int i = 0; i < count && i < MAX_FRIENDS; i++) {
                    string friendName;
                    getline(file, friendName);
                    if (!friendName.empty()) {
                        friendsList[friendsCount] = friendName;
                        friendsCount++;
                    }
                }
                
                int matchesCount = 0;
                file >> matchesCount;
                file.ignore();
                
                matchCount = 0;
                for (int i = 0; i < matchesCount && i < MAX_MATCHES; i++) {
                    string line;
                    getline(file, line);
                    
                    size_t pos1 = line.find('|');
                    size_t pos2 = line.find('|', pos1 + 1);
                    
                    if (pos1 != string::npos && pos2 != string::npos) {
                        string opponent = line.substr(0, pos1);
                        bool won = stoi(line.substr(pos1 + 1, pos2 - pos1 - 1)) != 0;
                        int pointsEarned = stoi(line.substr(pos2 + 1));
                        
                        matchHistory[matchCount] = MatchRecord(opponent, won, pointsEarned);
                        matchCount++;
                    }
                }
                
                file.close();
            }


// LOAD FROM PLAYERS FILE FUNCTION
// Player details ko "players.txt" file se load krta hai
            void PlayerProfile::loadFromPlayersFile(const string& playersFilePath) {
                ifstream file(playersFilePath);
                
                if (!file.is_open()) {
                    cerr << "Error: Could not open " << playersFilePath << endl;
                    return;
                }
                
                string line;
                while (getline(file, line)) {
                    size_t pos1 = line.find('|');
                    size_t pos2 = line.find('|', pos1 + 1);
                    size_t pos3 = line.find('|', pos2 + 1);
                    size_t pos4 = line.find('|', pos3 + 1);
                    size_t pos5 = line.find('|', pos4 + 1);
                    
                    if (pos1 != string::npos && pos2 != string::npos && pos3 != string::npos && 
                        pos4 != string::npos && pos5 != string::npos) {
                        int id = stoi(line.substr(0, pos1));
                        string user = line.substr(pos1 + 1, pos2 - pos1 - 1);
                        string nick = line.substr(pos3 + 1, pos4 - pos3 - 1);
                        string em = line.substr(pos4 + 1, pos5 - pos4 - 1);
                        
                        if (user == username) {
                            playerId = id;
                            nickname = nick;
                            email = em;
                            break;
                        }
                    }
                }
                
                file.close();
            }
