                   
                                        // =================================
                                        // |24i-2008 Abdul Raffay          |
                                        // |24i-2130  Muhammad Hamza Adil  |
                                        // |       " XONIX GAME "          |
                                        // |   LOGIN SYSTEM HEADER FILE    |
                                        // =================================


#ifndef LOGINSYSTEM_H
#define LOGINSYSTEM_H
#include <string>
using namespace std;


// Player ki details store krne k liye struct bnaya hai

    struct Player {
        int id;
        string username;
        string password;
        string nickname;
        string email;
        string timestamp;
    };

// Login register k basic functions bnaye hain

    bool registerPlayer(const string& username, const string& password, const string& nickname = "", const string& email = "");
    bool loginPlayer(const string& username, const string& password, Player& player);
    bool usernameExists(const string& username);
    bool validatePassword(const string& password);
    void split(const string& line, string fields[], int maxFields);

    #endif
