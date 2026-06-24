

                                    // =================================
                                    // |    24i-2008 Abdul Raffay      |
                                    // | 24i-2130  Muhammad Hamza Adil |
                                    // |        " XONIX GAME "         |
                                    // |  PLAYER PROFILE HEADER FILE   |
                                    // =================================


#ifndef PLAYER_PROFILE_H
#define PLAYER_PROFILE_H

#include <string>
#include <ctime>
using namespace std;


// STRUCT FOR MATCH RECORD
// Player ke matches ka record store krta hai
    struct MatchRecord {
            string opponent;
            bool won;
            int pointsEarned;
            
            MatchRecord(const string& opp = "", bool w = false, int pts = 0)
                : opponent(opp), won(w), pointsEarned(pts) {}
        };


// CLASS FOR PLAYER PROFILE
// Player ka profile manage krta hai
    class PlayerProfile {
        private:
            static const int MAX_MATCHES = 10;
            int playerId;
            string username;
            string nickname;
            string email;
            int totalPoints;
            static const int MAX_FRIENDS = 50;
            string friendsList[MAX_FRIENDS];
            int friendsCount; 
            MatchRecord matchHistory[MAX_MATCHES];
            int matchCount;
            
        public:
            PlayerProfile();
            PlayerProfile(int id, const string& name, const string& nick = "");
            int getPlayerId() const;
            string getUsername() const;
            string getNickname() const;
            string getEmail() const;
            int getTotalPoints() const;
            int getFriendsCount() const;
            string getFriend(int index) const;
            int getMatchCount() const;
            MatchRecord getMatch(int index) const;
            int getWins() const;
            int getLosses() const;
            void setNickname(const string& nick);
            void setEmail(const string& em);
            void setTotalPoints(int points);
            void addPoints(int points);
            void addFriend(const string& friendUsername);
            void removeFriend(const string& friendUsername);
            bool isFriend(const string& friendUsername) const;
            void recordMatch(const string& opponent, bool won, int pointsEarned);
            void saveToFile() const;
            void loadFromFile();
            void loadFromPlayersFile(const string& playersFilePath = "players.txt");
        };

#endif