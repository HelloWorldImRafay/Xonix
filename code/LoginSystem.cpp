
                                        // =================================
                                        // |    24i-2008 Abdul Raffay      |
                                        // | 24i-2130  Muhammad Hamza Adil |
                                        // |       " XONIX GAME "          |
                                        // |     LOGIN SYSTEM CPP FILE     |
                                        // =================================



#include "LoginSystem.h"
#include <fstream>
#include <ctime>
#include <iostream>
using namespace std;

// File jisme players ka data para hua hai
const string fileName = "players.txt";


// SPLIT FUNCTION
// File ko format ko read krta hai or jb usko "|" milta hai to wo usko seperate index me store kr deta hai

    void split(const string &line, string fields[], int maxFields)
    {
        int start = 0, field = 0;
        for (size_t i = 0; i <= line.size() && field < maxFields; i++)
        {
            if (i == line.size() || line[i] == '|')
            {
                fields[field] = line.substr(start, i - start);
                field++;
                start = i + 1;
            }
        }
    }



// VALIDATE PASSWORD FUNCTION
// Password ko check krta hai ke wo criteria ko meet krta hai ya nahi

    bool validatePassword(const string &password)
    {
        if (password.length() < 8)
            return false;

        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        string specialChars = "!@#$%^&*()-_+=";

        for (size_t i = 0; i < password.length(); i++)
        {
            char c = password[i];
            if (c >= 'A' && c <= 'Z')
                hasUpper = true;
            else if (c >= 'a' && c <= 'z')
                hasLower = true;
            else if (c >= '0' && c <= '9')
                hasDigit = true;
            else
            {
                for (size_t j = 0; j < specialChars.length(); j++)
                {
                    if (c == specialChars[j])
                    {
                        hasSpecial = true;
                        break;
                    }
                }
            }
        }

        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    
// USERNAME EXISTS FUNCTION
// Check krta hai ke username already exist krta hai ya nahi

    bool usernameExists(const string &username)
    {
        ifstream in(fileName.c_str());
        if (!in)
            return false;

        string line;
        while (getline(in, line))
        {
            if (line == "")
                continue;

            string fields[6];
            split(line, fields, 6);
            if (fields[1] == username)
                return true;
        }
        return false;
    }


// REGISTER PLAYER FUNCTION
// Naya player register krta hai agar username available ho or password valid ho or player.txt mai add kr deta hai

    bool registerPlayer(const string &username, const string &password,
                        const string &nickname, const string &email)
    {
        if (usernameExists(username))
        {
            cout << "Username already taken.\n";
            return false;
        }

        if (!validatePassword(password))
        {
            cout << "Password must contain uppercase, lowercase, number, special char.\n";
            return false;
        }

        int nextID = 1;
        {
            ifstream in(fileName.c_str());
            string line;
            while (getline(in, line))
            {
                if (line == "")
                    continue;
                nextID++;
            }
        }

        time_t t = time(nullptr);
        string ts = ctime(&t);
        ts.erase(ts.find('\n'));

        ofstream out(fileName.c_str(), ios::app);
        if (!out)
        {
            cout << "Error opening file for writing.\n";
            return false;
        }

        out << nextID << "|"
            << username << "|"
            << password << "|"
            << nickname << "|"
            << email << "|"
            << ts << "\n";

        out.flush();
        out.close();

        cout << "Player registered successfully!\n";
        return true;
    }


// LOGIN PLAYER FUNCTION
// Player ko login krta hai agr username or password match kr jaye to or agr wo file mai already ho to

    bool loginPlayer(const string &username, const string &password, Player &player)
    {
        ifstream in(fileName.c_str());
        if (!in)
            return false;

        string line;
        while (getline(in, line))
        {
            if (line == "")
                continue;

            string fields[6];
            split(line, fields, 6);

            if (fields[1] == username && fields[2] == password)
            {
                player.id = atoi(fields[0].c_str());
                player.username = fields[1];
                player.password = fields[2];
                player.nickname = fields[3];
                player.email = fields[4];
                player.timestamp = fields[5];
                return true;
            }
        }
        return false;
    }
