

                                        // =================================
                                        // |    24i-2008 Abdul Raffay      |
                                        // | 24i-2130  Muhammad Hamza Adil |
                                        // |       " XONIX GAME "          |
                                        // |     AVL TREE HEADER FILE      |
                                        // =================================


#ifndef AVL_TREE_H
#define AVL_TREE_H
#include "Theme.h"
#include <iostream>

// AVL NODE STRUCTURE
// Node structure for AVL Tree
    struct AVLNode {
            Theme data;
            AVLNode* left;
            AVLNode* right;
            int height;
            
            AVLNode(const Theme& theme)
                : data(theme), left(nullptr), right(nullptr), height(1) {}
        };


// AVL TREE CLASS
// Self-balancing binary search tree for Themes
    class AVLTree {
        private:
            AVLNode* root;
            
            int getHeight(AVLNode* node);
            int getBalanceFactor(AVLNode* node);
            AVLNode* rotateRight(AVLNode* y);
            AVLNode* rotateLeft(AVLNode* x);
            AVLNode* insertNode(AVLNode* node, const Theme& theme);
            AVLNode* deleteNode(AVLNode* node, int themeId);
            AVLNode* findMinNode(AVLNode* node);
            AVLNode* search(AVLNode* node, int themeId);
            AVLNode* searchByName(AVLNode* node, const std::string& themeName);
            int countThemes(AVLNode* node);
            void inorderTraversalToArray(AVLNode* node, Theme* arr, int& index, int maxSize);
            void deleteTree(AVLNode* node);
            
        public:
            AVLTree();
            ~AVLTree();
            
            void insert(const Theme& theme);
            void deleteTheme(int themeId);
            Theme* search(int themeId);
            Theme* searchByName(const std::string& themeName);
            int getTreeHeight();
            int getTotalThemes();
            bool isEmpty();
            void clear();
            void getAllThemesToArray(Theme* arr, int maxSize, int& outCount);
        };

        #endif 
