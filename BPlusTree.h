#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <iostream>
#include <cstring>
#include <vector>
#include <stdio.h>
#include <limits.h>
#include <cmath>
#include "Storage.h"

using namespace std;

extern const int N; // Indicate that N is defined elsewhere

class Node {
public:
    bool IS_LEAF; //2bytes
    int *key; // Pointer to an array of keys
    int size; //4bytes, the number of keys in the node
    Node **ptr; // Pointers to child nodes
    unsigned char **records; // Pointers to data records, for leaf nodes

    Node();
};

class BPlusTree {
    Node *root = NULL; //root node
    
    int nodes = 0;
    int levels = 0;
    int numKeys = 0;
    int deleteCounter = 0; // Keep track of deleted numVotes = 1000
    void insertInternal(int x, Node *parent, Node *child);
    void deleteInternal(int x, Node *curNode, Node *child);
    void splitLeafNode(Node* curNode, int x, unsigned char* record, Node* parent);
    void createNewRoot(Node* leftChild, Node* rightChild);
    Node *findParent(Node* currentNode, Node* targetChild);
    Node* createNewLeafNode(int key, unsigned char *data);
    Node* createNewBufferNode(int key, unsigned char *data);
    Node** traverseToLeafNode(int targetKey);
    void deallocate(Node *node);

public:
    BPlusTree();
    void search(int x);
    void insertKey(int x,unsigned char *record);
    void deleteKey(int x);
    void experiment2();
    void experiment5(int numVotesToDelete);
    void experiment3(int numVotes);
    void experiment4(int minVotes, int maxVotes);
};

#endif
