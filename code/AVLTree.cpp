
                                    // =================================
                                    // |    24i-2008 Abdul Raffay      |
                                    // | 24i-2130  Muhammad Hamza Adil |
                                    // |       " XONIX GAME "          |
                                    // |      AVL TREE CPP FILE        |
                                    // =================================


#include "AVLTree.h"
using namespace std;

// GET HEIGHT FUNCTIONS
// Tree ki height return krta hai
        int AVLTree::getHeight(AVLNode* node) {
            return (node == nullptr) ? 0 : node->height;
        }

// GET BALANCE FACTOR FUNCTION
// Node ka balance factor return krta hai (left subtree height - right subtree height)
        int AVLTree::getBalanceFactor(AVLNode* node) {
            if (node == nullptr) return 0;
            return getHeight(node->left) - getHeight(node->right);
        }

// ROTATE RIGHT FUNCTION
// Right rotation perform krta hai agar balance factor {-1,0,1} se bahar hojae
        AVLNode* AVLTree::rotateRight(AVLNode* root) {
            AVLNode* LeftChild = root->left;
            AVLNode* temp = LeftChild->right;
            
            LeftChild->right = root;
            root->left = temp;
            
            root->height = max(getHeight(root->left), getHeight(root->right)) + 1;
            LeftChild->height = max(getHeight(LeftChild->left), getHeight(LeftChild->right)) + 1;
            
            return LeftChild;
        }

// ROTATE LEFT FUNCTION
// Left rotation perform krta hai agar balance factor {-1,0,1} se bahar hojae

        AVLNode* AVLTree::rotateLeft(AVLNode* root) {
            AVLNode* RightChild = root->right;
            AVLNode* temp = RightChild->left;
            RightChild->left = root;
            root->right = temp;

            root->height = max(getHeight(root->left), getHeight(root->right)) + 1;
            RightChild->height = max(getHeight(RightChild->left), getHeight(RightChild->right)) + 1;
            
            return RightChild;
        }

// INSERT FUNCTION
// Naya theme insert krta hai AVL tree mai or balance maintain krta hai
        AVLNode* AVLTree::insertNode(AVLNode* root, const Theme& theme) {
            if (root == nullptr) {
                return new AVLNode(theme);
            }
            
            if (theme.themeId < root->data.themeId) {
                root->left = insertNode(root->left, theme);
            } else if (theme.themeId > root->data.themeId) {
                root->right = insertNode(root->right, theme);
            } else {
                root->data = theme;
                return root;
            }
            
            root->height = max(getHeight(root->left), getHeight(root->right)) + 1;
            
            int balanceFactor = getBalanceFactor(root);
            
            //Left-Left case
            if (balanceFactor > 1 && theme.themeId < root->left->data.themeId) {
                return rotateRight(root);
            }
            
            //Right-Right case
            if (balanceFactor < -1 && theme.themeId > root->right->data.themeId) {
                return rotateLeft(root);
            }
            
            //Left-Right case
            if (balanceFactor > 1 && theme.themeId > root->left->data.themeId) {
                root->left = rotateLeft(root->left);
                return rotateRight(root);
            }
            
            //Right-Left case
            if (balanceFactor < -1 && theme.themeId < root->right->data.themeId) {
                root->right = rotateRight(root->right);
                return rotateLeft(root);
            }
            
            return root;
        }

// SEARCH FUNCTION
// Theme ko themeId k basis par search krta hai

        AVLNode* AVLTree::search(AVLNode* node, int themeId) {
            if (node == nullptr) {
                return nullptr;
            }
            
            if (themeId == node->data.themeId) {
                return node;
            } else if (themeId < node->data.themeId) {
                return search(node->left, themeId);
            } else {
                return search(node->right, themeId);
            }
        }

// SEARCH BY NAME FUNCTION
// Theme ko themeName k basis par search krta hai

        AVLNode* AVLTree::searchByName(AVLNode* node, const string& themeName) {
            if (node == nullptr) {
                return nullptr;
            }
            
            if (themeName == node->data.themeName) {
                return node;
            }
            
            AVLNode* leftResult = searchByName(node->left, themeName);
            if (leftResult != nullptr) {
                return leftResult;
            }
            
            return searchByName(node->right, themeName);
        }

// COUNT THEMES FUNCTION
// Total themes count krta hai tree mai

        int AVLTree::countThemes(AVLNode* node) {
            if (node == nullptr) return 0;
            return 1 + countThemes(node->left) + countThemes(node->right);
        }

// INORDER TRAVERSAL TO ARRAY FUNCTION
// Themes ko sorted order mai array mai store krta hai (Tree to sorted array)
        void AVLTree::inorderTraversalToArray(AVLNode* node, Theme* arr, int& index, int maxSize) {
            if (node == nullptr) return;
            if (index >= maxSize) return;
            
            inorderTraversalToArray(node->left, arr, index, maxSize);
            if (index < maxSize) {
                arr[index++] = node->data;
            }
            inorderTraversalToArray(node->right, arr, index, maxSize);
        }

// FIND MIN NODE FUNCTION
// Given subtree mai minimum value wala node return krta hai

        AVLNode* AVLTree::findMinNode(AVLNode* node) {
            while (node->left != nullptr) {
                node = node->left;
            }
            return node;
        }

// DELETE NODE FUNCTION
// Theme ko themeId k basis par delete krta hai or balance maintain krta hai
        AVLNode* AVLTree::deleteNode(AVLNode* node, int themeId) {
            if (node == nullptr) {
                return nullptr;
            }
            
            if (themeId < node->data.themeId) {
                node->left = deleteNode(node->left, themeId);
            } else if (themeId > node->data.themeId) {
                node->right = deleteNode(node->right, themeId);
            } else {
                
                // Node ka koi child na ho
                if (node->left == nullptr && node->right == nullptr) {
                    delete node;
                    return nullptr;
                }
                
                // Node ka ek hi child ho
                if (node->left == nullptr) {
                    AVLNode* temp = node->right;
                    delete node;
                    return temp;
                }
                
                if (node->right == nullptr) {
                    AVLNode* temp = node->left;
                    delete node;
                    return temp;
                }
                
                // Node ke dono children ho
                AVLNode* minRight = findMinNode(node->right);
                node->data = minRight->data;
                node->right = deleteNode(node->right, minRight->data.themeId);
            }
            
            node->height = max(getHeight(node->left), getHeight(node->right)) + 1;
            int balanceFactor = getBalanceFactor(node);
            
            // Left-Left case
            if (balanceFactor > 1 && getBalanceFactor(node->left) >= 0) {
                return rotateRight(node);
            }
            
            // Left-Right case
            if (balanceFactor > 1 && getBalanceFactor(node->left) < 0) {
                node->left = rotateLeft(node->left);
                return rotateRight(node);
            }
            
            // Right-Right case
            if (balanceFactor < -1 && getBalanceFactor(node->right) <= 0) {
                return rotateLeft(node);
            }
            
            // Right-Left case
            if (balanceFactor < -1 && getBalanceFactor(node->right) > 0) {
                node->right = rotateRight(node->right);
                return rotateLeft(node);
            }
            
            return node;
        }

// DELETE TREE FUNCTION
// Puri tree ko delete krta hai or memory free krta hai

        void AVLTree::deleteTree(AVLNode* node) {
            if (node == nullptr) return;
            
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }

// AVL TREE CONSTRUCTOR AND DESTRUCTOR
        AVLTree::AVLTree() : root(nullptr) {}

        AVLTree::~AVLTree() {
            deleteTree(root);
        }

// INSERT FUNCTION
// Theme ko tree mai insert krta hai
        void AVLTree::insert(const Theme& theme) {
            root = insertNode(root, theme);
        }
// DELETE THEME FUNCTION
// Theme ko themeId k basis par delete krta hai
        void AVLTree::deleteTheme(int themeId) {
            root = deleteNode(root, themeId);
        }

// SEARCH FUNCTION
// Theme ko themeId k basis par search krta hai
        Theme* AVLTree::search(int themeId) {
            AVLNode* node = search(root, themeId);
            return (node != nullptr) ? &node->data : nullptr;
        }

// SEARCH BY NAME FUNCTION
// Theme ko themeName k basis par search krta hai
        Theme* AVLTree::searchByName(const string& themeName) {
            AVLNode* node = searchByName(root, themeName);
            return (node != nullptr) ? &node->data : nullptr;
        }


        int AVLTree::getTreeHeight() {
            return getHeight(root);
        }

        int AVLTree::getTotalThemes() {
            return countThemes(root);
        }

        bool AVLTree::isEmpty() {
            return root == nullptr;
        }

        void AVLTree::clear() {
            deleteTree(root);
            root = nullptr;
        }
// GET ALL THEMES TO ARRAY FUNCTION
// Themes ko sorted order mai array mai store krta hai
        void AVLTree::getAllThemesToArray(Theme* arr, int maxSize, int& outCount) {
            if (arr == nullptr || maxSize <= 0) {
                outCount = 0;
                return;
            }
            outCount = 0;
            inorderTraversalToArray(root, arr, outCount, maxSize);
        }
